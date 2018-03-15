// test_trigger_algorithm.cxx
// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// GSL:
#include <bayeux/mygsl/rng.h>
// Root :
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

// Boost :
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

// This project :
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/mapping.h>

#include <snemo/digitization/sd_to_calo_signal_algo.h>
#include <snemo/digitization/signal_to_calo_tp_algo.h>
#include <snemo/digitization/calo_tp_to_ctw_algo.h>

#include <snemo/digitization/sd_to_geiger_signal_algo.h>
#include <snemo/digitization/signal_to_geiger_tp_algo.h>
#include <snemo/digitization/geiger_tp_to_ctw_algo.h>

#include <snemo/digitization/trigger_algorithm.h>

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    // Parsing arguments
    bool is_display      = false;

    std::vector<std::string> input_filenames;
    std::string trigger_config_filename = "";
    std::string output_path = "";
    std::size_t max_events = 0;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h", "produce help message")
      ("display,d", "display mode")
      ("trace,t", "trace mode for debug purpose")
      ("input,i",
       po::value<std::vector<std::string> >(& input_filenames)->multitoken(),
       "set a list of input files")
      ("output-path,o",
       po::value<std::string>(& output_path),
       "set the output path where produced files are created")
      ("config,c",
       po::value<std::string>(& trigger_config_filename),
       "set the trigger configuration file")
      ("event_number,n",
       po::value<std::size_t>(& max_events)->default_value(10),
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

    // Use command line arguments :
    if (vm.count("display")) {
      is_display = true;
    }

    // Use command line arguments :
    if (vm.count("trace")) {
      logging = datatools::logger::PRIO_TRACE;
    }

    std::clog << "Test program for class 'snemo::digitization::test_trigger_algorithm' !" << std::endl;

    std::size_t file_counter = 0;
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++)
      {
	std::clog << "File #" << file_counter << ' ' << *file << std::endl;
	file_counter++;
      }

    if (input_filenames.size() == 0) {
      DT_LOG_WARNING(logging, "No input file(s) !");

      std::string input_default_file = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
      input_filenames.push_back(input_default_file);

      DT_LOG_WARNING(logging, "Default input file : " << input_filenames[0] << " !");
    }

    DT_LOG_INFORMATION(logging, "List of input file(s) : ");
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) std::clog << *file << ' ';

    // boolean for debugging (display etc)
    bool debug = false;
    if (is_display) debug = true;

    std::clog << "Test program for class ' !" << std::endl;
    int32_t seed = 314159;
    mygsl::rng random_generator;
    random_generator.initialize(seed);

    std::string manager_config_file;

    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env(manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config (manager_config_file,
					manager_config);
    geomtools::manager my_manager;
    manager_config.update ("build_mapping", true);
    if (manager_config.has_key ("mapping.excluded_categories"))
      {
	manager_config.erase ("mapping.excluded_categories");
      }
    my_manager.initialize (manager_config);

    // Simulated Data "SD" bank label :
    std::string SD_bank_label = "SD";

    // Event record :
    datatools::things ER;

    // Number of events :
    int max_record_total = static_cast<int>(max_events) * static_cast<int>(input_filenames.size());
    std::clog << "max_record total = " << max_record_total << std::endl;
    std::clog << "max_events       = " << max_events << std::endl;

    // Event reader :
    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store("logging.priority", "debug");
    // reader_config.store ("files.mode", "single");
    // reader_config.store_path("files.single.filename", input_filename);
    reader_config.store("files.mode", "list");
    reader_config.store("files.list.filenames", input_filenames);
    reader_config.store("max_record_total", max_record_total);
    reader_config.store("max_record_per_file", static_cast<int>(max_events));
    reader_config.tree_dump(std::clog, "Input module configuration parameters: ");
    reader.initialize_standalone(reader_config);
    reader.tree_dump(std::clog, "Simulated data reader module");


    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    // Electronic mapping :
    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.set_module_number(snemo::digitization::mapping::DEMONSTRATOR_MODULE_NUMBER);
    my_e_mapping.initialize();

    // Clock manager :
    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();

    // Initializing SD to calo signal algo :
    snemo::digitization::sd_to_calo_signal_algo sd_2_calo_signal(my_manager);
    sd_2_calo_signal.initialize();

    // Initializing SD to geiger signal algo :
    snemo::digitization::sd_to_geiger_signal_algo sd_2_geiger_signal(my_manager);
    sd_2_geiger_signal.initialize();

    // Initializing signal to calo tp algo :
    snemo::digitization::signal_to_calo_tp_algo signal_2_calo_tp;
    signal_2_calo_tp.initialize(my_e_mapping);

    // Initializing signal to geiger tp algo :
    snemo::digitization::signal_to_geiger_tp_algo signal_2_geiger_tp;
    signal_2_geiger_tp.initialize(my_e_mapping);

    // Initializing calo tp to calo ctw algorithms for each crate :
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_0;
    calo_tp_2_ctw_0.set_crate_number(snemo::digitization::mapping::MAIN_CALO_SIDE_0_CRATE);
    calo_tp_2_ctw_0.initialize();
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_1;
    calo_tp_2_ctw_1.set_crate_number(snemo::digitization::mapping::MAIN_CALO_SIDE_1_CRATE);
    calo_tp_2_ctw_1.initialize();
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_2;
    calo_tp_2_ctw_2.set_crate_number(snemo::digitization::mapping::XWALL_GVETO_CALO_CRATE);
    calo_tp_2_ctw_2.initialize();

    // Initializing geiger tp to geiger ctw :
    snemo::digitization::geiger_tp_to_ctw_algo geiger_tp_2_ctw;
    geiger_tp_2_ctw.initialize();

    // Multi properties to configure trigger algorithm :
    datatools::multi_properties trigger_config("name", "type", "Trigger parameters multi section configuration");

    if (!trigger_config_filename.empty()) {
      // Read trigger config from file
      datatools::fetch_path_with_env(trigger_config_filename);
      trigger_config.read(trigger_config_filename);
    } else {
      //Build trigger configuration multi properties section by section :

      /*********************/
      /* 'General' section */
      /*********************/
      trigger_config.add("general", "trigger_component");

      // Retrieve a hook to the 'general' section in the *trigger_config*:
      datatools::multi_properties::entry & gen_entry = trigger_config.grab("general");

      int coincidence_calorimeter_gate_size = 5; // Gate for the calorimeter gate size at 1600ns
      int L2_decision_coincidence_gate_size = 5; // Gate for calorimeter / tracker coincidence (5 x 1600 ns)
      int previous_event_buffer_depth = 10;      // Maximum number of PER record (with an internal counter of 1 ms)
      bool activate_any_coincidences = true;
      // bool activate_calorimeter_only = false; // not used for the moment -> to perform

      // Add properties in the 'general' section :
      gen_entry.grab_properties().store("coincidence_calorimeter_gate_size",
					coincidence_calorimeter_gate_size,
					"The coincidence calorimeter gate (1600ns) size value");

      gen_entry.grab_properties().store("L2_decision_coincidence_gate_size",
					L2_decision_coincidence_gate_size,
					"The L2 coincidence gate (1600ns) size value");

      gen_entry.grab_properties().store("previous_event_buffer_depth",
					previous_event_buffer_depth,
					"The previous event buffer size value");

      gen_entry.grab_properties().store("activate_any_coincidences",
					activate_any_coincidences,
					"Flag to activate any coincidence (CARACO, APE, DAVE...)");

      // gen_entry.grab_properties().store("activate_calorimeter_only", activate_calorimeter_only);

      /*************************/
      /* 'Calorimeter' section */
      /*************************/
      trigger_config.add("calorimeter", "trigger_component");
      datatools::multi_properties::entry & cal_entry = trigger_config.grab("calorimeter");

      int  calo_circular_buffer_depth = 4; // Size of the circular buffer (X * 25ns) default = 4*25=100ns to cumulate calorimeter hits
      int  calo_threshold = 1; // Number of calorimeter hit (with HT) to trigger the L1 decision
      double low_threshold_value = 30 * 1e-3 * CLHEP::volt;
      double high_threshold_value = 50 * 1e-3 * CLHEP::volt;
      bool inhibit_both_side_coinc = false;
      bool inhibit_single_side_coinc = false;

      cal_entry.grab_properties().store("circular_buffer_depth",
					calo_circular_buffer_depth,
					"The calorimeter circular buffer depth");

      cal_entry.grab_properties().store("total_multiplicity_threshold",
					calo_threshold,
					"The calorimeter total multiplicity threshold");

      // Value to check with explicit unit and compare it with the configuration file :
      cal_entry.grab_properties().store_with_explicit_unit("low_threshold_value",
							   low_threshold_value,
							   "The low threshold value in mV");

      // Value to check with explicit unit and compare it with the configuration file :
      cal_entry.grab_properties().store_with_explicit_unit("high_threshold_value",
							   high_threshold_value,
							   "The high threshold value in mV");

      cal_entry.grab_properties().store("inhibit_both_side",
					inhibit_both_side_coinc,
					"Inhibit both side trigger flag");

      cal_entry.grab_properties().store("inhibit_single_side",
					inhibit_single_side_coinc,
					"Inhibit single side trigger flag");

      /*********************/
      /* 'Tracker' section */
      /*********************/
      trigger_config.add("tracker", "trigger_component");
      datatools::multi_properties::entry & tra_entry = trigger_config.grab("tracker");

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

      tra_entry.grab_properties().store("mem1_file",
					mem1,
					"The memory 1 for the tracker trigger");
      tra_entry.grab_properties().store("mem2_file",
					mem2,
					"The memory 2 for the tracker trigger");
      tra_entry.grab_properties().store("mem3_file",
					mem3,
					"The memory 3 for the tracker trigger");
      tra_entry.grab_properties().store("mem4_file",
					mem4,
					"The memory 4 for the tracker trigger");
      tra_entry.grab_properties().store("mem5_file",
					mem5,
					"The memory 5 for the tracker trigger");

      /*************************/
      /* 'Coincidence' section */
      /*************************/
      trigger_config.add("coincidence", "trigger_component");
      // datatools::multi_properties::entry & coinc_entry = trigger_config.grab("coincidence");
    }

    // trigger_config.tree_dump(std::clog, "My trigger configuration");

    // Creation and initialization of trigger algorithm :
    snemo::digitization::trigger_algorithm my_trigger_algo;
    my_trigger_algo.set_electronic_mapping(my_e_mapping);
    my_trigger_algo.set_clock_manager(my_clock_manager);
    my_trigger_algo.initialize(trigger_config);

    trigger_config.tree_dump(std::clog, "My trigger config : ");

    // Root file output :
    std::string root_output_filename = output_path + "test_trigger_algorithm_time_output.root";
    datatools::fetch_path_with_env(root_output_filename);
    TFile* root_output_file = new TFile(root_output_filename.c_str(), "RECREATE");

    std::string string_buffer = "number_of_L2_decision_TH1F";
    TH1F * number_of_L2_decision_TH1F = new TH1F(string_buffer.c_str(),
						 Form("Number of L2 decision"),
						 10, 0, 10);

    string_buffer = "CARACO_decision_TH1F";
    TH1F * CARACO_decision_TH1F = new TH1F(string_buffer.c_str(),
    					   Form("CARACO decision"),
    					   2, 0, 2);

    string_buffer = "CARACO_CT_decision_1600ns_TH1F";
    TH1F * CARACO_CT_decision_1600ns_TH1F = new TH1F(string_buffer.c_str(),
    						     Form("CARACO CT decision 1600"),
    						     10, 0, 10);

    string_buffer = "delayed_decision_TH1F";
    TH1F * delayed_decision_TH1F = new TH1F(string_buffer.c_str(),
    					    Form("delayed decision"),
    					    2, 0, 2);

    string_buffer = "delayed_CT_decision_1600ns_TH1F";
    TH1F * delayed_CT_decision_1600ns_TH1F = new TH1F(string_buffer.c_str(),
    						      Form("delayed CT decision 1600"),
    						      650, 0, 650);

    string_buffer = "delayed_L2_trigger_mode_TH1F";
    TH1F * delayed_L2_trigger_mode_TH1F = new TH1F(string_buffer.c_str(),
						   Form("delayed delayed_L2_trigger_mode_TH1F"),
						   10, 0, 10);

    int psd_count = 0; // Event counter

    while (!reader.is_terminated())
      {
	reader.process(ER);
	DT_LOG_WARNING(logging, "Event #" << psd_count);
	// A plain `mctools::simulated_data' object is stored here :
	if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label))
	  {
	    // Access to the "SD" bank with a stored `mctools::simulated_data' :
	    const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);

	    my_clock_manager.compute_clockticks_ref(random_generator);
	    int32_t clocktick_25_reference  = my_clock_manager.get_clocktick_25_ref();
	    double  clocktick_25_shift      = my_clock_manager.get_shift_25();
	    int32_t clocktick_800_reference = my_clock_manager.get_clocktick_800_ref();
	    double  clocktick_800_shift     = my_clock_manager.get_shift_800();

	    // Creation of calo ctw data :
	    snemo::digitization::calo_ctw_data my_calo_ctw_data;

	    // Creation of geiger ctw data :
	    snemo::digitization::geiger_ctw_data my_geiger_ctw_data;

	    if (SD.has_step_hits("calo") || SD.has_step_hits("xcalo") || SD.has_step_hits("gveto") || SD.has_step_hits("gg"))
	      {
		// Creation of a signal data object to store calo & geiger signals :
		snemo::digitization::signal_data signal_data;

		// Processing Calo signal :
		sd_2_calo_signal.process(SD, signal_data);

		// Processing Geiger signal :
		sd_2_geiger_signal.process(SD, signal_data);

		if (logging == datatools::logger::PRIO_TRACE) {
		  signal_data.tree_dump(std::clog, "*** Signal Data ***", "INFO : ");
		  my_clock_manager.tree_dump(std::clog, "Clock utils : ", "INFO : ");
		}

		snemo::digitization::calo_tp_data my_calo_tp_data;
		// Calo signal to calo TP :
		if (signal_data.has_calo_signals())
		  {
		    // Set calo clockticks :
		    signal_2_calo_tp.set_clocktick_reference(clocktick_25_reference);
		    signal_2_calo_tp.set_clocktick_shift(clocktick_25_shift);

		    // Signal to calo TP process :
		    signal_2_calo_tp.process(signal_data, my_calo_tp_data);

		    // Calo TP to geiger CTW process :
		    calo_tp_2_ctw_0.process(my_calo_tp_data, my_calo_ctw_data);
		    calo_tp_2_ctw_1.process(my_calo_tp_data, my_calo_ctw_data);
		    calo_tp_2_ctw_2.process(my_calo_tp_data, my_calo_ctw_data);

		    if (logging == datatools::logger::PRIO_TRACE) {
		      my_calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");
		      my_calo_ctw_data.tree_dump(std::clog, "Calorimeter CTW(s) data : ", "INFO : ");
		    }

		  } // end of if has calo signal
		snemo::digitization::geiger_tp_data my_geiger_tp_data;
		if (signal_data.has_geiger_signals())
		  {
		    // Set geiger clockticks :
		    signal_2_geiger_tp.set_clocktick_reference(clocktick_800_reference);
		    signal_2_geiger_tp.set_clocktick_shift(clocktick_800_shift);
		    // Signal to geiger TP process
		    signal_2_geiger_tp.process(signal_data, my_geiger_tp_data);

		    // Geiger TP to geiger CTW process
		    geiger_tp_2_ctw.process(my_geiger_tp_data, my_geiger_ctw_data);

		    if (logging == datatools::logger::PRIO_TRACE) {
		      my_geiger_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");
		      my_geiger_ctw_data.tree_dump(std::clog, "Geiger CTW(s) data : ", "INFO : ");
		    }

		  } // end of if has geiger signal

	      } // end of if has "calo" || "xcalo" || "gveto" || "gg" step hits

	    // Trigger process
	    my_trigger_algo.process(my_calo_ctw_data,
				    my_geiger_ctw_data);

	    // Finale structures :

	    // Creation of outputs collection structures for calo and tracker
	    std::vector<snemo::digitization::trigger_structures::calo_summary_record> calo_collection_records = my_trigger_algo.get_calo_records_25ns_vector();
	    std::vector<snemo::digitization::trigger_structures::coincidence_calo_record> coincidence_collection_calo_records =  my_trigger_algo.get_coincidence_calo_records_1600ns_vector();
	    std::vector<snemo::digitization::trigger_structures::tracker_record>  tracker_collection_records = my_trigger_algo.get_tracker_records_vector();
	    std::vector<snemo::digitization::trigger_structures::coincidence_event_record> coincidence_collection_records = my_trigger_algo.get_coincidence_records_vector();
	    std::vector<snemo::digitization::trigger_structures::L2_decision> L2_decision_record = my_trigger_algo.get_L2_decision_records_vector();

	    uint16_t number_of_L2_decision = L2_decision_record.size();
	    bool caraco_decision = false;
	    uint32_t caraco_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
	    bool delayed_decision = false;
	    uint32_t delayed_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
	    bool already_delayed_trig = false;
	    snemo::digitization::trigger_structures::L2_trigger_mode delayed_trigger_mode = snemo::digitization::trigger_structures::L2_trigger_mode::INVALID;

	    if (number_of_L2_decision != 0)
	      {
	    	for (unsigned int isize = 0; isize < number_of_L2_decision; isize++)
	    	  {
	    	    if (L2_decision_record[isize].L2_decision_bool && L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::CARACO)
	    	      {
	    		caraco_decision         = L2_decision_record[isize].L2_decision_bool;
	    		caraco_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
	    	      }
	    	    else if (L2_decision_record[isize].L2_decision_bool &&  (L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE
	    								     || L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE) && already_delayed_trig == false)
	    	      {
	    		delayed_decision         = L2_decision_record[isize].L2_decision_bool;
	    		delayed_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
	    		delayed_trigger_mode     = L2_decision_record[isize].L2_trigger_mode;
	    		already_delayed_trig     = true;
	    	      }
	    	  }
	      }

	    // if (number_of_L2_decision == 4) std::cin.get();
	    number_of_L2_decision_TH1F->Fill(number_of_L2_decision);
	    CARACO_decision_TH1F->Fill(caraco_decision);
	    if (caraco_clocktick_1600ns != snemo::digitization::clock_utils::INVALID_CLOCKTICK) CARACO_CT_decision_1600ns_TH1F->Fill(caraco_clocktick_1600ns);
	    delayed_decision_TH1F->Fill(delayed_decision);
	    if (delayed_decision && delayed_clocktick_1600ns != snemo::digitization::clock_utils::INVALID_CLOCKTICK) delayed_CT_decision_1600ns_TH1F->Fill(delayed_clocktick_1600ns);
	    if (delayed_decision && delayed_trigger_mode != snemo::digitization::trigger_structures::L2_trigger_mode::INVALID) delayed_L2_trigger_mode_TH1F->Fill(delayed_trigger_mode);

	    DT_LOG_INFORMATION(logging, "Number of L2 decision : " << number_of_L2_decision);
	    DT_LOG_INFORMATION(logging, "CARACO decision :       " << caraco_decision);
	    DT_LOG_INFORMATION(logging, "CARACO CT1600ns :       " << caraco_clocktick_1600ns);
	    DT_LOG_INFORMATION(logging, "Delayed decision :      " << delayed_decision);
	    DT_LOG_INFORMATION(logging, "Delayed CT1600ns :      " << delayed_clocktick_1600ns);
	    DT_LOG_INFORMATION(logging, "Delayed trigger mode :  " << delayed_trigger_mode);

	    my_trigger_algo.reset_data();

	  } //end of if has bank label "SD"

	ER.clear();
	psd_count++;
	if (debug) std::clog << "DEBUG : psd count " << psd_count << std::endl;
	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
      } // end of reader is terminated

    root_output_file->Write();
    root_output_file->Close();

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
