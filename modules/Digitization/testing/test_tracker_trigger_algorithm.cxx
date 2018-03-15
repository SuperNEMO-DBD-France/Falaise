//test_tracker_trigger_algorithm.cxx
// Standard libraries :
#include <iostream>
#include <fstream>

// GSL:
#include <bayeux/mygsl/rng.h>
#include <bayeux/mygsl/histogram.h>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
#include <datatools/properties.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>
// Falaise:
#include <falaise/falaise.h>

// Third part :
// Boost :
#include <boost/program_options.hpp>

// This project :
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/sd_to_geiger_signal_algo.h>
#include <snemo/digitization/signal_to_geiger_tp_algo.h>
#include <snemo/digitization/geiger_tp_to_ctw_algo.h>
#include <snemo/digitization/tracker_trigger_algorithm.h>

int main(int  argc_ , char ** argv_)
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  std::string input_filename = "";
  int max_events = 0;

  // Parse options:
  namespace po = boost::program_options;
  po::options_description opts("Allowed options");
  opts.add_options()
    ("help,h", "produce help message")
    ("input,i",
     po::value<std::string>(& input_filename),
     "set an input file")
    ("event_number,n",
     po::value<int>(& max_events)->default_value(10),
     "set the maximum number of events")
    ; // end of options description

  // Describe command line arguments :
  po::variables_map vm;
  po::store(po::command_line_parser(argc_, argv_)
	    .options(opts)
	    .run(), vm);
  po::notify(vm);

  // Use command line arguments :
  if (vm.count("help")) {
    std::cout << "Usage : " << std::endl;
    std::cout << opts << std::endl;
    return(error_code);
  }

  // Process
  try {
    std::clog << "Test program for class 'snemo::digitization::tracker_trigger_algorithm' !" << std::endl;
    // Random generator and seed
    int32_t seed = 314159;
    mygsl::rng random_generator;
    random_generator.initialize(seed);

    // Geometry manager
    std::string manager_config_file;
    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env(manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config(manager_config_file,
					manager_config);
    geomtools::manager my_manager;
    manager_config.update("build_mapping", true);
    if (manager_config.has_key("mapping.excluded_categories"))
      {
	manager_config.erase("mapping.excluded_categories");
      }
    my_manager.initialize(manager_config);

    std::string pipeline_simulated_data_filename;
    // Simulated Data "SD" bank label :
    std::string SD_bank_label = "SD";
    // Trigger Decision Data "TDD" bank label :
    std::string TDD_bank_label = "TDD";

    // Default input file :
    if (!input_filename.empty()) pipeline_simulated_data_filename = input_filename;
    else
      {
	pipeline_simulated_data_filename = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
      }
    datatools::fetch_path_with_env(pipeline_simulated_data_filename);

    // Loading memory from external files
    std::string mem1 = "${FALAISE_DIGITIZATION_TESTING_DIR}/config/trigger/tracker/mem1.conf";
    std::string mem2 = "${FALAISE_DIGITIZATION_TESTING_DIR}/config/trigger/tracker/mem2.conf";
    std::string mem3 = "${FALAISE_DIGITIZATION_TESTING_DIR}/config/trigger/tracker/mem3.conf";
    std::string mem4 = "${FALAISE_DIGITIZATION_TESTING_DIR}/config/trigger/tracker/mem4.conf";
    std::string mem5 = "${FALAISE_DIGITIZATION_TESTING_DIR}/config/trigger/tracker/mem5.conf";

    datatools::fetch_path_with_env(mem1);
    datatools::fetch_path_with_env(mem2);
    datatools::fetch_path_with_env(mem3);
    datatools::fetch_path_with_env(mem4);
    datatools::fetch_path_with_env(mem5);

    // Event reader :
    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store ("logging.priority", "debug");
    reader_config.store ("max_record_total", max_events);
    reader_config.store ("files.mode", "single");
    reader_config.store ("files.single.filename", pipeline_simulated_data_filename);
    reader.initialize_standalone (reader_config);
    reader.tree_dump (std::clog, "Simulated data reader module");

    // Event record :
    datatools::things ER;

    // Electronic mapping :
    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.set_module_number(snemo::digitization::mapping::DEMONSTRATOR_MODULE_NUMBER);
    my_e_mapping.initialize();

    // Clock manager :
    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();

    // Internal counters
    int psd_count = 0;         // Event counter

    while (!reader.is_terminated())
      {
	reader.process(ER);
	//datatools::properties & TDD = ER.add<datatools::properties>(TDD_bank_label);

	// A plain `mctools::simulated_data' object is stored here :
	if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label))
	  {
	    // Access to the "SD" bank with a stored `mctools::simulated_data' :
	    const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);

	    //SD.tree_dump(std::clog, "my_SD", "SD :");
	    if(SD.has_step_hits("gg"))
	      {
		// Creation of a signal data object to store geiger signals :
		snemo::digitization::signal_data signal_data;

		// Initializing and processing SD to Geiger signal :
		snemo::digitization::sd_to_geiger_signal_algo sd_2_geiger_signal(my_manager);
		sd_2_geiger_signal.initialize();
		sd_2_geiger_signal.process(SD, signal_data);

		std::vector<snemo::digitization::trigger_structures::tracker_record> tracker_collection_records;
		std::vector<snemo::digitization::trigger_structures::geiger_matrix> geiger_matrix_collection_records;
		if( signal_data.has_geiger_signals())
		  {
		    // Random clocktick references and shifts
		    my_clock_manager.compute_clockticks_ref(random_generator);
		    int32_t clocktick_25_reference  = my_clock_manager.get_clocktick_25_ref();
		    double  clocktick_25_shift      = my_clock_manager.get_shift_25();
		    int32_t clocktick_800_reference = my_clock_manager.get_clocktick_800_ref();
		    double  clocktick_800_shift     = my_clock_manager.get_shift_800();

		    std::clog << "CT 25 Ref : " << clocktick_25_reference
			      << " CT 25 shift : " << clocktick_25_shift << std::endl;

		    std::clog << " CT 800 Ref : " << clocktick_800_reference
			      << " CT 800 shift : " << clocktick_800_shift << std::endl;

		    // Creation of a geiger TP data object to store geiger TP :
		    snemo::digitization::geiger_tp_data my_geiger_tp_data;

		    // Initializing and processing Geiger signal to Geiger TP :
		    snemo::digitization::signal_to_geiger_tp_algo signal_2_geiger_tp;
		    signal_2_geiger_tp.initialize(my_e_mapping);
		    signal_2_geiger_tp.set_clocktick_reference(clocktick_800_reference);
		    signal_2_geiger_tp.set_clocktick_shift(clocktick_800_shift);
		    signal_2_geiger_tp.process(signal_data, my_geiger_tp_data);
		    // my_geiger_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");

		    // Creation of a geiger CTW data object to store geiger CTW :
		    snemo::digitization::geiger_ctw_data my_geiger_ctw_data;

		    // Initializing and processing Geiger TP to Geiger CTW :
		    snemo::digitization::geiger_tp_to_ctw_algo geiger_tp_2_ctw;
		    geiger_tp_2_ctw.initialize();
		    geiger_tp_2_ctw.process(my_geiger_tp_data, my_geiger_ctw_data);
		    my_geiger_ctw_data.tree_dump(std::clog, "Geiger CTW(s) data : ", "INFO : ");

		    // Initializing and processing tracker trigger algorithm to make a decision :
		    snemo::digitization::tracker_trigger_algorithm my_tracker_algo;
		    my_tracker_algo.set_electronic_mapping(my_e_mapping);
		    my_tracker_algo.fill_mem1_all(mem1);
		    my_tracker_algo.fill_mem2_all(mem2);
		    my_tracker_algo.fill_mem3_all(mem3);
		    my_tracker_algo.fill_mem4_all(mem4);
		    my_tracker_algo.fill_mem5_all(mem5);

		    my_tracker_algo.initialize();

		    // Do the job CT by CT (in 'global' implementation : driven by the trigger_algorithm)
		    uint32_t ct_800_min = my_geiger_ctw_data.get_clocktick_min();
		    uint32_t ct_800_max = my_geiger_ctw_data.get_clocktick_max();

		    // For this test program all CT 800 are kept and processed.
		    for (uint32_t ict800 = ct_800_min; ict800 <= ct_800_max; ict800++)
		      {

			snemo::digitization::geiger_ctw_data::geiger_ctw_collection_type gg_ctw_list_per_ct800;
			my_geiger_ctw_data.get_list_of_geiger_ctw_per_clocktick(ict800, gg_ctw_list_per_ct800);

			snemo::digitization::trigger_structures::tracker_record a_tracker_record;
			a_tracker_record.clocktick_1600ns = ict800; // only for this test purpose. Has to be 1600 ns.
			my_tracker_algo.process(gg_ctw_list_per_ct800,
						a_tracker_record);
			if (!a_tracker_record.is_empty()) tracker_collection_records.push_back(a_tracker_record);

			snemo::digitization::trigger_structures::geiger_matrix a_geiger_matrix = my_tracker_algo.get_geiger_matrix_for_a_clocktick();
			a_geiger_matrix.clocktick_1600ns = ict800; // only for this test purpose. Has to be 1600 ns.
			if (!a_geiger_matrix.is_empty()) geiger_matrix_collection_records.push_back(a_geiger_matrix);
		      }

		  } // end of if has geiger signals data

		// Display tracker and geiger matrix records :
		for (unsigned int i = 0; i < tracker_collection_records.size(); i++)
		  {
		    tracker_collection_records[i].display();
		    geiger_matrix_collection_records[i].display();
		  }

	      } // end of if has "gg" step hits

	  } // end of if has bank label
	ER.clear();

	psd_count++;
	//std::clog << "DEBUG : psd count " << psd_count << std::endl;
	//std::clog << "\r" << "DEBUG : psd count " << psd_count << std::flush;
	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);

      } // end of while

    std::clog << "The end." << std::endl;
  }
  catch (std::exception & error) {
    DT_LOG_FATAL(logging, error.what());
    error_code = EXIT_FAILURE;
  }

  catch (...) {
    DT_LOG_FATAL(logging, "Unexpected error!");
    error_code = EXIT_FAILURE;
  }

  falaise::terminate();
  return error_code;
}
