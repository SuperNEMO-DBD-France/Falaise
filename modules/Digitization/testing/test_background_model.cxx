//test_background_model.cxx

// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
#include <datatools/temporary_files.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
#include <geomtools/base_hit.h>
#include <geomtools/gnuplot_draw.h>
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
#include <geomtools/gnuplot_i.h>
#include <geomtools/gnuplot_drawer.h>
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>
#include <falaise/snemo/geometry/calo_locator.h>
#include <falaise/snemo/geometry/gveto_locator.h>
#include <falaise/snemo/geometry/gg_locator.h>

// Third part :
// GSL:
#include <bayeux/mygsl/rng.h>

// Boost :
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

// This project
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/mapping.h>

#include <snemo/digitization/sd_to_calo_signal_algo.h>
#include <snemo/digitization/signal_to_calo_tp_algo.h>
#include <snemo/digitization/calo_tp_to_ctw_algo.h>

#include <snemo/digitization/sd_to_geiger_signal_algo.h>
#include <snemo/digitization/signal_to_geiger_tp_algo.h>
#include <snemo/digitization/geiger_tp_to_ctw_algo.h>

#include <snemo/digitization/trigger_algorithm.h>


void generate_pool_of_calo_spurious_signals(mygsl::rng * rdm_gen_,
					    const std::vector<geomtools::geom_id> & gid_collection_,
					    snemo::digitization::signal_data & calo_tracker_spurious_signals_);

void generate_pool_of_geiger_spurious_signals(mygsl::rng * rdm_gen_,
					      const std::vector<geomtools::geom_id> & gid_collection_,
					      snemo::digitization::signal_data & calo_tracker_spurious_signals_);

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    bool is_display = false;
    std::string trigger_config_filename = "";
    std::string output_path = "";
    std::size_t max_events = 0;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h",    "produce help message")
      ("display,d", "display mode")
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
      std::cout << "Usage :" << std::endl;
      std::cout << opts << std::endl;
      return(error_code);
    }

    if (vm.count("display")) {
      is_display = true;
    }

    std::clog << "Test program for a naive 'background trigger model' !" << std::endl;
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

    unsigned int module_number = 0;

    // Select calo main wall GID :
    geomtools::geom_id main_wall_gid_pattern(1302,
                                             module_number,
                                             geomtools::geom_id::ANY_ADDRESS, // Side
                                             geomtools::geom_id::ANY_ADDRESS, // Column
                                             geomtools::geom_id::ANY_ADDRESS, // Row
                                             0); // part only 0, to convert into any (to not have part 0 and 1 in the vector double count)
    std::vector<geomtools::geom_id> collection_of_main_wall_gid;
    my_manager.get_mapping().compute_matching_geom_id(main_wall_gid_pattern,
                                                      collection_of_main_wall_gid);

    // Convert part 0 into any (*)
    for (std::size_t i = 0; i < collection_of_main_wall_gid.size(); i++) {
      collection_of_main_wall_gid[i].set_any(5);
    }

    // Select calo xwall GID :
    geomtools::geom_id xwall_gid_pattern(1232,
                                         module_number,
                                         geomtools::geom_id::ANY_ADDRESS,  // Side
                                         geomtools::geom_id::ANY_ADDRESS,  // Wall
                                         geomtools::geom_id::ANY_ADDRESS,  // Column
                                         geomtools::geom_id::ANY_ADDRESS); // Row

    std::vector<geomtools::geom_id> collection_of_xwall_gid;
    my_manager.get_mapping().compute_matching_geom_id(xwall_gid_pattern,
                                                      collection_of_xwall_gid);

    // Select calo gveto GID :
    geomtools::geom_id gveto_gid_pattern(1252,
                                         module_number,
                                         geomtools::geom_id::ANY_ADDRESS,  // Side
                                         geomtools::geom_id::ANY_ADDRESS,  // Wall
                                         geomtools::geom_id::ANY_ADDRESS); // Column

    std::vector<geomtools::geom_id> collection_of_gveto_gid;
    my_manager.get_mapping().compute_matching_geom_id(gveto_gid_pattern,
                                                      collection_of_gveto_gid);

    snemo::digitization::signal_data calo_tracker_spurious_signals;

    std::clog << "Generating pool of spurious signals..." << std::endl;

    // Generate pool of main wall spurious signals :
    generate_pool_of_calo_spurious_signals(&random_generator,
					   collection_of_main_wall_gid,
					   calo_tracker_spurious_signals);

    // Generate pool of xwall spurious signals :
    generate_pool_of_calo_spurious_signals(&random_generator,
					   collection_of_xwall_gid,
					   calo_tracker_spurious_signals);

    // Generate pool of gveto spurious signals :
    generate_pool_of_calo_spurious_signals(&random_generator,
					   collection_of_gveto_gid,
					   calo_tracker_spurious_signals);

    std::sort(calo_tracker_spurious_signals.grab_calo_signals().begin(),
	      calo_tracker_spurious_signals.grab_calo_signals().end(),
	      snemo::digitization::calo_signal::compare_handle_by_timestamp());

    // Select geiger GID :
    geomtools::geom_id geiger_gid_pattern(1204,
					  module_number,
					  geomtools::geom_id::ANY_ADDRESS,  // Side
					  geomtools::geom_id::ANY_ADDRESS,  // Layer
					  geomtools::geom_id::ANY_ADDRESS); // Row

    std::vector<geomtools::geom_id> collection_of_geiger_gid;
    // geomtools::geom_id a_test_gid(1204,0,1,4,48);
    // collection_of_geiger_gid.push_back(a_test_gid);
    my_manager.get_mapping().compute_matching_geom_id(geiger_gid_pattern,
						      collection_of_geiger_gid);

    generate_pool_of_geiger_spurious_signals(&random_generator,
					     collection_of_geiger_gid,
					     calo_tracker_spurious_signals);

    std::sort(calo_tracker_spurious_signals.grab_geiger_signals().begin(),
	      calo_tracker_spurious_signals.grab_geiger_signals().end(),
	      snemo::digitization::geiger_signal::compare_handle_by_timestamp());

    std::clog << "Total number of spurious calo signals  : " << calo_tracker_spurious_signals.get_number_of_calo_signals() << std::endl;
    std::clog << "Main wall number of spurious signals   : " << calo_tracker_spurious_signals.get_number_of_main_calo_signals() << std::endl;
    std::clog << "Xwall number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_xcalo_signals() << std::endl;
    std::clog << "Gveto number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_gveto_signals() << std::endl;
    std::clog << "Geiger cell number of spurious signals : " << calo_tracker_spurious_signals.get_number_of_geiger_signals() << std::endl;

    // Event builder of spurious signal to do :
    snemo::digitization::signal_data temp_signals = calo_tracker_spurious_signals;
    std::vector<snemo::digitization::signal_data> collection_of_events;
    const double event_window = 100 * CLHEP::microsecond; // us : window tunable
    double event_stepper = 1.6 * CLHEP::microsecond;

    /*   Strategy to build events :
	 <-----> event window (fix)
	 |     |
	 -x--x---x---x--x-----x-----x--x---x-------> time     (x : calo | tracker hits)
	 |     |
	 tstart  tstop    step = 1600ns (lowest step in the trigger)
    */

    // Display 5 first calo and tracker signals
    for (std::size_t icalo = 0; icalo < 5; icalo++)
      {
    	temp_signals.get_calo_signals()[icalo].get().tree_dump(std::clog, "A calo signal #" + std::to_string(icalo));
      }

    for (std::size_t igeiger = 0; igeiger < 5; igeiger++)
      {
    	temp_signals.get_geiger_signals()[igeiger].get().tree_dump(std::clog, "A geiger signal #" + std::to_string(igeiger));
      }

    std::clog << std::endl << "Building events from spurious signals..." << std::endl;

    std::clog << "Event window (in us) " << event_window / CLHEP::microsecond << std::endl;
    std::size_t event_counter = 0;


    // // For testing purpose :
    // snemo::digitization::signal_data test_signals;
    // double max_amplitude = 600;
    // for (std::size_t j = 0; j < 10; j++)
    //   {
    // 	geomtools::geom_id a_gid(1302,0,0,j,4);
    // 	snemo::digitization::calo_signal & a_cs = test_signals.add_calo_signal();
    // 	a_cs.set_header(j, a_gid);
    // 	const double timestamp = random_generator.flat(0, 100) * j * CLHEP::microsecond;
    // 	a_cs.set_data(timestamp, max_amplitude);
    //   }

    // std::sort(test_signals.grab_calo_signals().begin(),
    // 	      test_signals.grab_calo_signals().end(),
    // 	      snemo::digitization::calo_signal::compare_handle_by_timestamp());
    // for (std::size_t i = 0; i < 10; i++)
    //   {
    // 	test_signals.get_calo_signals()[i].get().tree_dump(std::clog, "A test calo signal #"+ std::to_string(i));
    //   }

    /****************************************/
    /****  Event building from signals   ****/
    /****************************************/

    for (unsigned int event_tstart = 0 * CLHEP::microsecond; event_tstart < 1 * CLHEP::second; event_tstart += event_stepper)
      {
	unsigned int event_tstop = event_tstart + event_window;
	// std::clog << "Event_tstart = " << event_tstart << " Event_tstop " << event_tstop << std::endl;

	snemo::digitization::signal_data event_signals;

	// Calo signals part :

	// Lambda functions not working (see later)
	// auto above_calo_pred = [=](const datatools::handle<snemo::digitization::calo_signal> & ha) {
	//   bool answer = (ha.get().get_signal_time() >= event_tstop);
	//   return answer;
	// };
	// auto below_calo_pred = [=](const datatools::handle<snemo::digitization::calo_signal> & ha) {
	//   bool answer = (ha.get().get_signal_time() <= event_tstart);
	//   return answer;
	// };
	// Find calorimeter signals between [tstart; tstop]
	// auto below_calo = std::find_if(temp_signals.get_calo_signals().begin(),
	// 			          temp_signals.get_calo_signals().end(),
	// 			          below_calo_pred);
	// auto above_calo = std::find_if(temp_signals.get_calo_signals().begin(),
	// 		                  temp_signals.get_calo_signals().end(),
	// 			          above_calo_pred);

	// Own method (working)
	// Find first calo signal (in tstart) and last before tstop :
	int first_calo_in_window = -1;
	bool first_calo_found = false;
	int last_calo_in_window = -1;
	bool last_calo_found = false;
	for (unsigned int i = 0; i < temp_signals.get_calo_signals().size(); i++)
	  {
	    if (first_calo_found == false && temp_signals.get_calo_signals()[i].get().get_signal_time() >= event_tstart) {
	      first_calo_in_window = i;
	      first_calo_found = true;
	    }
	    if (last_calo_found == false &&  temp_signals.get_calo_signals()[i].get().get_signal_time() >= event_tstop) {
	      last_calo_in_window = i - 1;
	      last_calo_found = true;
	    }
	    if (first_calo_found && last_calo_found) break;
	  }
	if (first_calo_in_window != -1 && last_calo_in_window != -1) {
	  for (int icalo = first_calo_in_window; icalo <= last_calo_in_window; icalo++)
	    {
	      snemo::digitization::calo_signal & a_calo_signal = event_signals.add_calo_signal();
	      a_calo_signal = temp_signals.get_calo_signals()[icalo].get();
	    }
	}
	// Tracker signals part :
	// Find first geiger signal (in tstart) and last before tstop :
	int first_geiger_in_window = -1;
	bool first_geiger_found = false;
	int last_geiger_in_window = -1;
	bool last_geiger_found = false;
	for (unsigned int i = 0; i < temp_signals.get_geiger_signals().size(); i++)
	  {
	    if (first_geiger_found == false && temp_signals.get_geiger_signals()[i].get().get_anode_avalanche_time() >= event_tstart) {
	      first_geiger_in_window = i;
	      first_geiger_found = true;
	    }
	    if (last_geiger_found == false &&  temp_signals.get_geiger_signals()[i].get().get_anode_avalanche_time() >= event_tstop) {
	      last_geiger_in_window = i - 1;
	      last_geiger_found = true;
	    }
	    if (first_geiger_found && last_geiger_found) break;
	  }

	if (first_geiger_in_window != -1 && last_geiger_in_window != -1) {
	  for (int igeiger = first_geiger_in_window; igeiger <= last_geiger_in_window; igeiger++)
	    {
	      snemo::digitization::geiger_signal & a_geiger_signal = event_signals.add_geiger_signal();
	      a_geiger_signal = temp_signals.get_geiger_signals()[igeiger].get();
	    }
	}

	std::size_t number_of_calo_in_event = event_signals.get_calo_signals().size();
	std::size_t number_of_geiger_in_event = event_signals.get_geiger_signals().size();

	if (number_of_calo_in_event != 0 || number_of_geiger_in_event != 0)
	  {
	    collection_of_events.push_back(event_signals);
	    std::clog << "Event #" << event_counter
	    	      << " Number of calo in event   : " << number_of_calo_in_event
	    	      << " Number of geiger in event : " << number_of_geiger_in_event << std::endl;
	    event_counter++;
	  }
      } // end of for

    std::clog << "Temp end Calo size = " << temp_signals.get_calo_signals().size()
	      << " Temp end Geiger size = " << temp_signals.get_geiger_signals().size() << std::endl;

    std::clog << "Number of events = " << collection_of_events.size() << " event counter " << event_counter << std::endl;

    /*** End of self-trigger event construction**/

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
    std::clog << "Number of events for the trigger = " << max_events << std::endl;

    // Set the default output path :
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

    std::clog << "Trigger decision computations for " << max_events << " events" << std::endl;
    std::clog << "Size collection of events : "  << collection_of_events.size() << std::endl;
    if (max_events >= collection_of_events.size()) max_events = collection_of_events.size();
    for (unsigned int ievent = 0; ievent < max_events; ievent++)
      {
	std::clog << "Trigger event : " << ievent << std::endl;
	//collection_of_events[ievent].tree_dump(std::clog, "*** Signal Data ***", "INFO : ");
	my_clock_manager.compute_clockticks_ref(random_generator);
	int32_t clocktick_25_reference  = my_clock_manager.get_clocktick_25_ref();
	double  clocktick_25_shift      = my_clock_manager.get_shift_25();
	int32_t clocktick_800_reference = my_clock_manager.get_clocktick_800_ref();
	double  clocktick_800_shift     = my_clock_manager.get_shift_800();

	snemo::digitization::signal_data signal_data = collection_of_events[ievent];

	// signal_data.tree_dump(std::clog, "*** Signal Data ***", "INFO : ");
	// Display of signal data for event i :
	// for (std::size_t icalo = 0; icalo < signal_data.get_calo_signals().size(); icalo++)
	//   {
	//     signal_data.get_calo_signals()[icalo].get().tree_dump(std::clog, "A calo signal #" + std::to_string(icalo) + " in event #" + std::to_string(ievent));
	//   }

	// for (std::size_t igeiger = 0; igeiger < signal_data.get_geiger_signals().size(); igeiger++)
	//   {
	//     signal_data.get_geiger_signals()[igeiger].get().tree_dump(std::clog, "A geiger signal #" + std::to_string(igeiger) + " in event #" + std::to_string(ievent));
	//   }
	// my_clock_manager.tree_dump(std::clog, "Clock utils : ", "INFO : ");

	snemo::digitization::calo_tp_data my_calo_tp_data;
	// Creation of calo ctw data :
	snemo::digitization::calo_ctw_data my_calo_ctw_data;
	// Calo signal to calo TP :
	if (signal_data.has_calo_signals())
	  {
	    // Set calo clockticks :
	    signal_2_calo_tp.set_clocktick_reference(clocktick_25_reference);
	    signal_2_calo_tp.set_clocktick_shift(clocktick_25_shift);

	    // Signal to calo TP process :
	    signal_2_calo_tp.process(signal_data, my_calo_tp_data);

	    // for (std::size_t icalotp = 0; icalotp < my_calo_tp_data.get_calo_tps().size(); icalotp++)
	    //   {
	    // 	my_calo_tp_data.get_calo_tps()[icalotp].get().tree_dump(std::clog, "A calo TP #" + std::to_string(icalotp) + " in event #" + std::to_string(ievent));
	    //   }

	    // Calo TP to geiger CTW process :
	    calo_tp_2_ctw_0.process(my_calo_tp_data, my_calo_ctw_data);
	    calo_tp_2_ctw_1.process(my_calo_tp_data, my_calo_ctw_data);
	    calo_tp_2_ctw_2.process(my_calo_tp_data, my_calo_ctw_data);

	    // my_calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");
	    // my_calo_ctw_data.tree_dump(std::clog, "Calorimeter CTW(s) data : ", "INFO : ");

	  } // end of if has calo signal

	snemo::digitization::geiger_tp_data my_geiger_tp_data;
	// Creation of geiger ctw data :
	snemo::digitization::geiger_ctw_data my_geiger_ctw_data;
	if (signal_data.has_geiger_signals())
	  {
	    // Set geiger clockticks :
	    signal_2_geiger_tp.set_clocktick_reference(clocktick_800_reference);
	    signal_2_geiger_tp.set_clocktick_shift(clocktick_800_shift);
	    // Signal to geiger TP process
	    signal_2_geiger_tp.process(signal_data, my_geiger_tp_data);

	    // Geiger TP to geiger CTW process
	    geiger_tp_2_ctw.process(my_geiger_tp_data, my_geiger_ctw_data);

	    // my_geiger_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");
	    // my_geiger_ctw_data.tree_dump(std::clog, "Geiger CTW(s) data : ", "INFO : ");

	  } // end of if has geiger signal

	// Trigger process :
	my_trigger_algo.process(my_calo_ctw_data,
				my_geiger_ctw_data);

	// Finale structures :
	// Creation of outputs collection structures for calo and tracker
	std::vector<snemo::digitization::trigger_structures::calo_summary_record> calo_collection_records = my_trigger_algo.get_calo_records_25ns_vector();
	std::vector<snemo::digitization::trigger_structures::coincidence_calo_record> coincidence_collection_calo_records =  my_trigger_algo.get_coincidence_calo_records_1600ns_vector();
	std::vector<snemo::digitization::trigger_structures::tracker_record>  tracker_collection_records = my_trigger_algo.get_tracker_records_vector();
	std::vector<snemo::digitization::trigger_structures::coincidence_event_record> coincidence_collection_records = my_trigger_algo.get_coincidence_records_vector();
	std::vector<snemo::digitization::trigger_structures::L2_decision> L2_decision_record = my_trigger_algo.get_L2_decision_records_vector();

	// for (std::size_t i = 0; i  < calo_collection_records.size(); i++) {
	//    calo_collection_records[i].display();
	// }

	// for (std::size_t i = 0; i  < coincidence_collection_calo_records.size(); i++) {
	//   coincidence_collection_calo_records[i].display();
	// }

	// for (std::size_t i = 0; i  < tracker_collection_records.size(); i++) {
	//   tracker_collection_records[i].display();
	// }

	for (std::size_t i = 0; i  < coincidence_collection_records.size(); i++) {
	  coincidence_collection_records[i].display();
	}
	std::size_t number_of_L2_decision = L2_decision_record.size();
	bool caraco_decision = false;
	// uint32_t caraco_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
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
	if (caraco_decision) total_number_of_CARACO_decision++;
	if (delayed_decision && delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE) total_number_of_APE_decision++;
	if (delayed_decision && delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE) total_number_of_DAVE_decision++;

	// std::clog << "Number of L2 decision : " << number_of_L2_decision << std::endl;
	// std::clog << "CARACO decision :       " << caraco_decision << std::endl;
	// std::clog << "CARACO CT1600ns :       " << caraco_clocktick_1600ns << std::endl;
	// std::clog << "Delayed decision :      " << delayed_decision << std::endl;
	// std::clog << "Delayed CT1600ns :      " << delayed_clocktick_1600ns << std::endl;
	// std::clog << "Delayed trigger mode :  " << delayed_trigger_mode << std::endl;
	my_trigger_algo.reset_data();
	// std::clog << std::endl;
      }


    std::ofstream ftmp;
    ftmp.open(output_path + "background_output.dat");

    ftmp << "Total number of spurious calo signals  : " << calo_tracker_spurious_signals.get_number_of_calo_signals() << std::endl;
    ftmp << "Main wall number of spurious signals   : " << calo_tracker_spurious_signals.get_number_of_main_calo_signals() << std::endl;
    ftmp << "Xwall number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_xcalo_signals() << std::endl;
    ftmp << "Gveto number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_gveto_signals() << std::endl;
    ftmp << "Geiger cell number of spurious signals : " << calo_tracker_spurious_signals.get_number_of_geiger_signals() << std::endl << std::endl;;


    ftmp << "Size of the event window in us : " << event_window / CLHEP::microsecond << std::endl;
    ftmp << "Number of events : " << collection_of_events.size() << std::endl;
    ftmp << "Max events for the trigger = " << max_events << std::endl;
    ftmp << "Total number of L2 decision : " << total_number_of_L2_decision << std::endl;
    ftmp << "Total number of CARACO decision : " << total_number_of_CARACO_decision << std::endl;
    ftmp << "Total number of APE decision : " << total_number_of_APE_decision << std::endl;
    ftmp << "Total number of DAVE decision : " << total_number_of_DAVE_decision << std::endl;

    ftmp.close ();

    if (is_display){
      // #if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
      //       Gnuplot g1;
      //       g1.cmd("set title 'Test gnuplot draw' ");
      //       g1.cmd("set grid");
      //       g1.cmd("set xrange [0:1]");
      //       {
      //         std::ostringstream plot_cmd;
      //         plot_cmd << "plot '" << ftmp.get_filename() << "' using 1:2 with lines";
      //         g1.cmd(plot_cmd.str());
      //         g1.showonscreen(); // window output
      //         geomtools::gnuplot_drawer::wait_for_key();
      //         usleep(200);
      //       }
      // #endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
      //     }

      //     // std::clog << "Enter a value to close the program" << std::endl;
      //     // std::string key ="";
      //     // std::cin >> key;
    }
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

void generate_pool_of_calo_spurious_signals(mygsl::rng * rdm_gen_,
					    const std::vector<geomtools::geom_id> & gid_collection_,
					    snemo::digitization::signal_data & calo_tracker_spurious_signals_)
{
  int hit_count = 0;
  const double time_interval = 1 * CLHEP::second; // second
  const double calo_self_triggering_frequency = 10. / CLHEP::second; // Hertz

  // Create spurious hits during a time interval for each calo GID :
  for (std::size_t i = 0; i < gid_collection_.size(); i++)
    {
      double mean_number = time_interval * calo_self_triggering_frequency;
      double sigma_gauss = std::sqrt(mean_number);
      std::size_t number_of_calo_hit = 0;
      std::string distrib = "";
      // Number of calo hit during time_interval for a calo block (identified by his GID) :
      // If > 20 gaussian distribution
      if (mean_number > 20) {
        number_of_calo_hit = rdm_gen_->gaussian(mean_number, sigma_gauss);
        distrib = "gaussian";
      }
      // Else Poisson distribution
      else {
        number_of_calo_hit = rdm_gen_->poisson(mean_number);
        distrib = "poisson";
      }

      double min_amplitude = 15;  // mV (50 keV equivalent) -> unit pb ?
      double max_amplitude = 600; // mV (2 MeV equivalent) -> unit pb ?
      for (std::size_t j = 0; j < number_of_calo_hit; j++)
        {
	  snemo::digitization::calo_signal & a_cs = calo_tracker_spurious_signals_.add_calo_signal();
	  a_cs.set_header(hit_count, gid_collection_[i]);
          const double timestamp = rdm_gen_->flat(0, time_interval);
	  const double amplitude = rdm_gen_->flat(min_amplitude, max_amplitude);
	  a_cs.set_data(timestamp, amplitude);
          hit_count++;
        }
    }

  return;
}

void generate_pool_of_geiger_spurious_signals(mygsl::rng * rdm_gen_,
					      const std::vector<geomtools::geom_id> & gid_collection_,
					      snemo::digitization::signal_data & calo_tracker_spurious_signals_)
{
  int hit_count = 0;
  const double time_interval = 1 * CLHEP::second; // second
  const double geiger_self_triggering_frequency = 5. / CLHEP::second; // 'real' value = 0.2 Hertz
  const double cell_dead_time = 1 * CLHEP::millisecond; // 'real' value = 1 ms

  // Create spurious hits during a time interval for each geiger cell thanks to GID :
  for (std::size_t i = 0; i < gid_collection_.size(); i++)
    {
      double mean_number = time_interval * geiger_self_triggering_frequency;
      double sigma_gauss = std::sqrt(mean_number);
      std::size_t number_of_geiger_hit = 0;
      std::string distrib = "";
      // Number of geiger hit during time_interval for a geiger cell (identified by his GID) :
      // If nhits > 20 : gaussian distribution
      if (mean_number > 20) {
        number_of_geiger_hit = rdm_gen_->gaussian(mean_number, sigma_gauss);
        distrib = "gaussian";
      }
      // else Poisson distribution
      else {
        number_of_geiger_hit = rdm_gen_->poisson(mean_number);
        distrib = "poisson";
      }
      const geomtools::geom_id actual_gid = gid_collection_[i];

      // We have to generate 'number_of_geiger_hit' of the same cell in the time [0:time_interval]
      // We have to take care about the dead time of the cell.
      // After a spurious hit, each cell cannot trigger during a dead time :
      // TO DO : Retrigger probability to implement ? For each cell, probability = 0.05 to retrigger after a spurious hit at 'new' timestamp = old_timestamp + tdead_cell

      std::vector<double> timestamp_pool;

      double time_interval_limit = number_of_geiger_hit * cell_dead_time;
      double time_max = time_interval - time_interval_limit;
      bool particular_case = false;
      if (time_max <= 0) {
	// Particular case, high frequency / low dead_time or time interval too small
	// Geiger cell always trigger (freq >> dead_time), kind of 'saturation'
	for (std::size_t j = 0; j < number_of_geiger_hit; j++) {
	  const double timestamp = j * cell_dead_time;
	  if (timestamp >= 0 && timestamp < time_interval) {
	    timestamp_pool.push_back(timestamp);
	  }
	}
	particular_case = true;
      }
      else {
	for (std::size_t j = 0; j < number_of_geiger_hit; j++) {
	  const double timestamp = rdm_gen_ -> flat(0, time_max);
	  timestamp_pool.push_back(timestamp);
	}
	particular_case = false;
      }
      std::sort(timestamp_pool.begin(), timestamp_pool.end());

      for (std::size_t j = 0; j < timestamp_pool.size(); j++)
	{
	  snemo::digitization::geiger_signal & a_gs = calo_tracker_spurious_signals_.add_geiger_signal();
	  a_gs.set_header(hit_count, actual_gid);
	  double anodic_time = 0;
	  if (particular_case) anodic_time = timestamp_pool[j];
 	  else anodic_time = timestamp_pool[j] + j * cell_dead_time;
	  a_gs.set_data(anodic_time);
	  hit_count++;
	}
    }

  return;
}
