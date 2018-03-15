// trigger_program_on_signals.cxx
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
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;

  try {
    bool is_display = false;
    std::string input_filename;
    std::string trigger_config_filename = "";
    std::string output_path = "";
    // std::size_t max_events = 0;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h",    "produce help message")
      ("display,d", "display mode")
      ("input,i",
       po::value<std::string>(& input_filename),
       "set an input file")
      ("output-path,o",
       po::value<std::string>(& output_path),
       "set the output path where produced files are created")
      ("config,c",
       po::value<std::string>(& trigger_config_filename),
       "set the trigger configuration file")
      // ("event_number,n",
      //  po::value<std::size_t>(& max_events)->default_value(10),
      //  "set the maximum number of events")
      ; // end of options description

    // Describe command line arguments :
    po::variables_map vm;
    po::store(po::command_line_parser(argc_, argv_)
              .options(opts)
              .run(), vm);
    po::notify(vm);

    // Use command line arguments :
    if (vm.count("help")) {
      std::cout << "Usage :" << std::endl;
      std::cout << opts << std::endl;
      return(error_code);
    }

    else if (vm.count("display")) {
      is_display = true;
      DT_LOG_INFORMATION(logging, "Display mode activated : " + is_display);
    }

    DT_LOG_INFORMATION(logging, "Trigger program with an input of signal data only !");

    if (input_filename.empty()) {
      DT_LOG_WARNING(logging, "No input file(s) !");
      input_filename = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/calo_tracker_self_trigger_default.data.bz2";
      datatools::fetch_path_with_env(input_filename);
    }

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    int32_t seed = 314158;
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
    my_manager.initialize(manager_config);

    DT_LOG_INFORMATION(logging, "Deserialization input file :" + input_filename);
    datatools::data_reader deserializer(input_filename, datatools::using_multiple_archives);

    std::string output_stat_filename = output_path + '/' + "output_trigger.stat";
    DT_LOG_INFORMATION(logging, "Trigger statistics output file :" + output_stat_filename);

    std::string output_details_filename = output_path + '/' + "output_trigger.log";
    DT_LOG_INFORMATION(logging, "Trigger details output file :" + output_details_filename);

    // open filestreams :
    std::ofstream statstream;
    statstream.open(output_stat_filename);
    statstream << "Welcome on self trigger statistic file" << std::endl << std::endl;

    std::ofstream detstream;
    detstream.open(output_details_filename);
    detstream << "Welcome on self trigger details log file" << std::endl << std::endl;

    /********************************************************/
    /****  Trigger part on the self triggering signals   ****/
    /********************************************************/

    // Multi properties to configure trigger algorithm :
    datatools::multi_properties trigger_config("name", "type", "Trigger parameters multi section configuration");

    if (trigger_config_filename.empty()) {
      trigger_config_filename = "${FALAISE_DIGITIZATION_DIR}/resources/config/snemo/common/1.0/trigger_parameters.conf";
    }
    datatools::fetch_path_with_env(trigger_config_filename);
    trigger_config.read(trigger_config_filename);

    // Number of events :
    //    std::clog << "Number of events for the trigger = " << max_events << std::endl;

    // Electronic mapping :
    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.set_module_number(snemo::digitization::mapping::DEMONSTRATOR_MODULE_NUMBER);
    my_e_mapping.initialize();

    // Clock manager :
    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();

    // Initializing signal to calo_tp algo :
    snemo::digitization::signal_to_calo_tp_algo signal_2_calo_tp;
    signal_2_calo_tp.initialize(my_e_mapping);

    // Initializing signal to geiger_tp algo :
    snemo::digitization::signal_to_geiger_tp_algo signal_2_geiger_tp;
    signal_2_geiger_tp.initialize(my_e_mapping);

    // Initializing calo_tp to calo_ctw algorithms for each crate :
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_0;
    calo_tp_2_ctw_0.set_crate_number(snemo::digitization::mapping::MAIN_CALO_SIDE_0_CRATE);
    calo_tp_2_ctw_0.initialize();
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_1;
    calo_tp_2_ctw_1.set_crate_number(snemo::digitization::mapping::MAIN_CALO_SIDE_1_CRATE);
    calo_tp_2_ctw_1.initialize();
    snemo::digitization::calo_tp_to_ctw_algo calo_tp_2_ctw_2;
    calo_tp_2_ctw_2.set_crate_number(snemo::digitization::mapping::XWALL_GVETO_CALO_CRATE);
    calo_tp_2_ctw_2.initialize();

    // Initializing geiger_tp to geiger_ctw :
    snemo::digitization::geiger_tp_to_ctw_algo geiger_tp_2_ctw;
    geiger_tp_2_ctw.initialize();

    // Creation and initialization of trigger algorithm :
    snemo::digitization::trigger_algorithm my_trigger_algo;
    my_trigger_algo.set_electronic_mapping(my_e_mapping);
    my_trigger_algo.set_clock_manager(my_clock_manager);
    my_trigger_algo.initialize(trigger_config);

    // General statistics :
    std::size_t total_number_of_L2_decision = 0;
    std::size_t total_number_of_CARACO_decision = 0;
    std::size_t total_number_of_APE_decision = 0;
    std::size_t total_number_of_DAVE_decision = 0;

    std::size_t total_number_of_decision_non_sucessive = 0;
    std::size_t decision_last_event_number = 0;
    //std::clog << "Trigger decision computations for " << max_events << " events" << std::endl;

    std::size_t number_of_events_deserialized = 0;

    while (deserializer.has_record_tag()) {
      //DT_LOG_DEBUG(logging, "Entering has record tag...");
      if (number_of_events_deserialized % 10000 == 0) {
	DT_LOG_INFORMATION(logging, "Trigger event : " + std::to_string(number_of_events_deserialized));
	detstream << "Trigger event : " << number_of_events_deserialized << std::endl;
      }

      my_clock_manager.compute_clockticks_ref(random_generator);
      int32_t clocktick_25_reference  = my_clock_manager.get_clocktick_25_ref();
      double  clocktick_25_shift      = my_clock_manager.get_shift_25();
      int32_t clocktick_800_reference = my_clock_manager.get_clocktick_800_ref();
      double  clocktick_800_shift     = my_clock_manager.get_shift_800();

      snemo::digitization::signal_data signal_data;

      if (deserializer.record_tag_is(snemo::digitization::signal_data::SERIAL_TAG)) {
	deserializer.load(signal_data);

	// signal_data.tree_dump(std::clog, "*** Signal Data ***", "INFO : ");

	snemo::digitization::calo_tp_data my_calo_tp_data;
	snemo::digitization::calo_ctw_data my_calo_ctw_data;
	if (signal_data.has_calo_signals())
	  {
	    signal_2_calo_tp.set_clocktick_reference(clocktick_25_reference);
	    signal_2_calo_tp.set_clocktick_shift(clocktick_25_shift);

	    signal_2_calo_tp.process(signal_data, my_calo_tp_data);

	    calo_tp_2_ctw_0.process(my_calo_tp_data, my_calo_ctw_data);
	    calo_tp_2_ctw_1.process(my_calo_tp_data, my_calo_ctw_data);
	    calo_tp_2_ctw_2.process(my_calo_tp_data, my_calo_ctw_data);

	  } // end of if has calo signal

	snemo::digitization::geiger_tp_data my_geiger_tp_data;
	snemo::digitization::geiger_ctw_data my_geiger_ctw_data;
	if (signal_data.has_geiger_signals())
	  {
	    signal_2_geiger_tp.set_clocktick_reference(clocktick_800_reference);
	    signal_2_geiger_tp.set_clocktick_shift(clocktick_800_shift);

	    signal_2_geiger_tp.process(signal_data, my_geiger_tp_data);

	    geiger_tp_2_ctw.process(my_geiger_tp_data, my_geiger_ctw_data);
	  } // end of if has geiger signal

	// Trigger process :
	my_trigger_algo.process(my_calo_ctw_data,
				my_geiger_ctw_data);

	// Finale structures :
	// Creation of outputs collection structures for calo and tracker
	auto calo_collection_records = my_trigger_algo.get_calo_records_25ns_vector();
	auto coincidence_collection_calo_records =  my_trigger_algo.get_coincidence_calo_records_1600ns_vector();
	auto tracker_collection_records = my_trigger_algo.get_tracker_records_vector();
	auto coincidence_collection_records = my_trigger_algo.get_coincidence_records_vector();
	auto L1_calo_decision_records = my_trigger_algo.get_L1_calo_decision_records_vector();
	auto L2_decision_record = my_trigger_algo.get_L2_decision_records_vector();

	// Display all information of trigger event if coincidence in details filestream
	// if (coincidence_collection_records.size() != 0) {

	//   detstream  << "******* Event number #" << number_of_events_deserialized <<" ********" << std::endl;
	//   statstream << "******* Event number #" << number_of_events_deserialized <<" ********" << std::endl;

	//   for (std::size_t i = 0; i  < calo_collection_records.size(); i++) {
	//     calo_collection_records[i].display(detstream);
	//   }

	//   for (std::size_t i = 0; i  < coincidence_collection_calo_records.size(); i++) {
	//     coincidence_collection_calo_records[i].display(detstream);
	//   }

	//   for (std::size_t i = 0; i  < tracker_collection_records.size(); i++) {
	//     tracker_collection_records[i].display(detstream);
	//   }

	//   for (std::size_t i = 0; i  < coincidence_collection_records.size(); i++) {
	//     coincidence_collection_records[i].display(detstream);
	//   }

	//   for (std::size_t i = 0; i  < L1_calo_decision_records.size(); i++) {
	//     L1_calo_decision_records[i].display(detstream);
	//   }

	//   for (std::size_t i = 0; i  < L2_decision_record.size(); i++) {
	//     L2_decision_record[i].display(detstream);
	//   }
	// }

	std::size_t number_of_L2_decision = L2_decision_record.size();
	bool caraco_decision = false;
	//uint32_t caraco_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
	bool delayed_decision = false;
	// uint32_t delayed_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
	bool already_delayed_trig = false;
	snemo::digitization::trigger_structures::L2_trigger_mode delayed_trigger_mode = snemo::digitization::trigger_structures::L2_trigger_mode::INVALID;

	if (number_of_L2_decision != 0)
	  {
	    for (std::size_t isize = 0; isize < number_of_L2_decision; isize++)
	      {
		if (L2_decision_record[isize].L2_decision_bool && L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::CARACO)
		  {
		    caraco_decision         = L2_decision_record[isize].L2_decision_bool;
		    // caraco_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
		  }
		else if (L2_decision_record[isize].L2_decision_bool &&  (L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE
									 || L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE) && already_delayed_trig == false)
		  {
		    delayed_decision         = L2_decision_record[isize].L2_decision_bool;
		    // delayed_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
		    delayed_trigger_mode     = L2_decision_record[isize].L2_trigger_mode;
		    already_delayed_trig     = true;
		  }
	      }
	  }
	total_number_of_L2_decision += L2_decision_record.size();
	if (caraco_decision) {
	  detstream << "Event number : " << number_of_events_deserialized << std::endl;
	  detstream << "***** CARACO DECISION *****" << std::endl << std::endl;

	  if (number_of_events_deserialized != decision_last_event_number + 1) total_number_of_decision_non_sucessive += 1;

	  decision_last_event_number = number_of_events_deserialized;
	  total_number_of_CARACO_decision++;
	}

	if (delayed_decision && delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE)
	  {
	    detstream << "Event number : " << number_of_events_deserialized << std::endl;
	    detstream << "***** APE DECISION *****" << std::endl << std::endl;
	    if (number_of_events_deserialized != decision_last_event_number + 1) total_number_of_decision_non_sucessive += 1;

	    decision_last_event_number = number_of_events_deserialized;
	    total_number_of_APE_decision++;
	  }
	if (delayed_decision && delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE)
	  {
	    detstream << "Event number : " << number_of_events_deserialized << std::endl;
	    detstream << "***** DAVE DECISION *****" << std::endl << std::endl;
	    if (number_of_events_deserialized != decision_last_event_number + 1) total_number_of_decision_non_sucessive += 1;

	    decision_last_event_number = number_of_events_deserialized;
	    total_number_of_DAVE_decision++;
	  }

	// if (caraco_decision || delayed_decision) {
	//   detstream << "Number of L2 decision :  " << number_of_L2_decision << std::endl;
	//   detstream << "CARACO decision :        " << caraco_decision << std::endl;
	//   detstream << "CARACO CT1600ns :        " << caraco_clocktick_1600ns << std::endl;
	//   detstream << "Delayed decision :       " << delayed_decision << std::endl;
	//   detstream << "Delayed CT1600ns :       " << delayed_clocktick_1600ns << std::endl;
	//   detstream << "Delayed trigger mode :   " << delayed_trigger_mode << std::endl << std::endl;

	//   statstream << "Number of L2 decision : " << number_of_L2_decision << std::endl;
	//   statstream << "CARACO decision :       " << caraco_decision << std::endl;
	//   statstream << "CARACO CT1600ns :       " << caraco_clocktick_1600ns << std::endl;
	//   statstream << "Delayed decision :      " << delayed_decision << std::endl;
	//   statstream << "Delayed CT1600ns :      " << delayed_clocktick_1600ns << std::endl;
	//   statstream << "Delayed trigger mode :  " << delayed_trigger_mode << std::endl << std::endl;
	// }
	my_trigger_algo.reset_data();

	number_of_events_deserialized++;
      }
    } // end of while


    DT_LOG_INFORMATION(logging, "Total number of events : " + std::to_string(number_of_events_deserialized));
    statstream << "Total number of events : " << std::to_string(number_of_events_deserialized) << std::endl;
    statstream << "Number of decision non successive : " << std::to_string(total_number_of_decision_non_sucessive) << std::endl;
    statstream << "Number of events with CARACO decision : " << std::to_string(total_number_of_CARACO_decision) << std::endl;
    statstream << "Number of events with delayed (APE | DAVE) decision : " << std::to_string(total_number_of_APE_decision + total_number_of_DAVE_decision) << std::endl;


    detstream << "Total number of events : " << std::to_string(number_of_events_deserialized) << std::endl;
    detstream << "Number of decision non successive : " << std::to_string(total_number_of_decision_non_sucessive) << std::endl;
    detstream << "Number of events with CARACO decision : " << std::to_string(total_number_of_CARACO_decision) << std::endl;
    detstream << "Number of events with delayed (APE | DAVE) decision : " << std::to_string(total_number_of_APE_decision + total_number_of_DAVE_decision) << std::endl;

    // close filestreams :
    statstream.close();
    detstream.close();

    deserializer.reset();
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
