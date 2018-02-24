// produce_self_trigger_hits.cxx

// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
#include <datatools/temporary_files.h>
#include <datatools/io_factory.h>
#include <datatools/properties.h>
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
#include <snemo/digitization/signal_data.h>



void generate_pool_of_calo_spurious_signals(mygsl::rng * rdm_gen_,
					    const datatools::properties & config_,
					    const std::vector<geomtools::geom_id> & gid_collection_,
					    snemo::digitization::signal_data & calo_tracker_spurious_signals_);

void generate_pool_of_geiger_spurious_signals(mygsl::rng * rdm_gen_,
					      const datatools::properties & config_,
					      const std::vector<geomtools::geom_id> & gid_collection_,
					      snemo::digitization::signal_data & calo_tracker_spurious_signals_);

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    bool is_display = false;
    std::string output_path = "";
    std::string config_file = "";

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
       po::value<std::string>(& config_file),
       "set the config file to produce self trigger events")
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

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    // Default config file (datatools::properties format):
    if (config_file.empty()) config_file = "${FALAISE_DIGITIZATION_DIR}/resources/self_trigger.conf";
    datatools::fetch_path_with_env(config_file);
    datatools::properties st_config;
    datatools::properties::read_config(config_file, st_config);

    std::clog << "Test program for a naive 'calo / tracker self trigger model' !" << std::endl;
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

    std::string output_filename = output_path + '/' + "self_trigger_hits.data.bz2";
    DT_LOG_INFORMATION(logging, "Serialization output file :" + output_filename);
    // Event serializer module :
    // dpp::output_module serializer;
    // datatools::properties writer_config;
    // writer_config.store ("logging.priority", "debug");
    // writer_config.store ("files.mode", "single");
    // writer_config.store ("files.single.filename", output_filename);
    datatools::data_writer serializer(output_filename,
                                      datatools::using_multiple_archives);

    //serializer.initialize_standalone(writer_config);
    //serializer.tree_dump(std::clog, "Self Trigger hits writer module");

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
      collection_of_main_wall_gid[i].set_any(4);
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
					   st_config,
					   collection_of_main_wall_gid,
					   calo_tracker_spurious_signals);

    // Generate pool of xwall spurious signals :
    generate_pool_of_calo_spurious_signals(&random_generator,
					   st_config,
					   collection_of_xwall_gid,
					   calo_tracker_spurious_signals);

    // Generate pool of gveto spurious signals :
    generate_pool_of_calo_spurious_signals(&random_generator,
					   st_config,
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

    std::clog << "DEBUG 0 : before generate pool of GG spurious " << std::endl;
    generate_pool_of_geiger_spurious_signals(&random_generator,
					     st_config,
					     collection_of_geiger_gid,
					     calo_tracker_spurious_signals);

    std::clog << "DEBUG 1 : after generate pool of GG spurious " << std::endl;

    std::clog << "DEBUG 2 : size of generate pool of GG spurious " << calo_tracker_spurious_signals.get_geiger_signals().size() << std::endl;

    std::sort(calo_tracker_spurious_signals.grab_geiger_signals().begin(),
	      calo_tracker_spurious_signals.grab_geiger_signals().end(),
	      snemo::digitization::geiger_signal::compare_handle_by_timestamp());

    std::clog << "DEBUG 3 : after sort pool of GG spurious " << std::endl;

    std::clog << "Total number of spurious calo signals  : " << calo_tracker_spurious_signals.get_number_of_calo_signals() << std::endl;
    std::clog << "Main wall number of spurious signals   : " << calo_tracker_spurious_signals.get_number_of_main_calo_signals() << std::endl;
    std::clog << "Xwall number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_xcalo_signals() << std::endl;
    std::clog << "Gveto number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_gveto_signals() << std::endl;
    std::clog << "Geiger cell number of spurious signals : " << calo_tracker_spurious_signals.get_number_of_geiger_signals() << std::endl;

    // Event builder of spurious signal to do :
    snemo::digitization::signal_data temp_signals = calo_tracker_spurious_signals;

    /*   Strategy to build events :
	 <-----> event window (fix)
	 |     |
	 -x--x---x---x--x-----x-----x--x---x-------> time     (x : calo | tracker hits)
	 |     |
	 tstart  tstop    step = 1600ns (lowest step in the trigger)
    */

    // Display 5 first calo and tracker signals
    // for (std::size_t icalo = 0; icalo < 5; icalo++)
    //   {
    // 	temp_signals.get_calo_signals()[icalo].get().tree_dump(std::clog, "A calo signal #" + std::to_string(icalo));
    //   }

    // for (std::size_t igeiger = 0; igeiger < 5; igeiger++)
    //   {
    // 	temp_signals.get_geiger_signals()[igeiger].get().tree_dump(std::clog, "A geiger signal #" + std::to_string(igeiger));
    //   }

    // std::ofstream ggtimestream;
    // std::string gg_time_filename = output_path + "anode_timestamp.dat";
    // ggtimestream.open(gg_time_filename);

    // for (std::size_t igeiger = 0; igeiger < calo_tracker_spurious_signals.get_number_of_geiger_signals(); igeiger++)
    //   {
    // 	ggtimestream << igeiger << ' ' << calo_tracker_spurious_signals.get_geiger_signals()[igeiger].get().get_anode_avalanche_time() / CLHEP::second << std::endl;
    //   }
    // ggtimestream.close();


    // for (std::size_t icalo = temp_signals.get_calo_signals().size() - 1; icalo >= temp_signals.get_calo_signals().size() - 10; icalo--)
    //   {
    // 	//temp_signals.get_calo_signals()[icalo].get().tree_dump(std::clog, "A calo signal #" + std::to_string(icalo));
    //   }

    // for (std::size_t igeiger = temp_signals.get_geiger_signals().size() - 1 ; igeiger >= temp_signals.get_geiger_signals().size() - 10; igeiger--)
    //   {
    // 	//temp_signals.get_geiger_signals()[igeiger].get().tree_dump(std::clog, "A geiger signal #" + std::to_string(igeiger));
    //   }



    std::clog << std::endl << "Building events from spurious signals..." << std::endl;

    /****************************************/
    /****  Event building from signals   ****/
    /****************************************/
    std::vector<snemo::digitization::signal_data> collection_of_events;
    double time_interval;
    double event_window; // = 100 * CLHEP::microsecond; // us : window tunable
    double event_stepper = 1.6 * CLHEP::microsecond;
    std::size_t event_counter = 0;
    datatools::invalidate(time_interval);
    datatools::invalidate(event_window);

    if (st_config.has_key("time_interval")) {
      time_interval = st_config.fetch_real("time_interval");
    }
    if (st_config.has_key("event_window_integration")) {
      event_window = st_config.fetch_real("event_window_integration");
    }

    std::clog << "Event window (in us) " << event_window / CLHEP::microsecond << std::endl;
    for (unsigned int event_tstart = 0 * CLHEP::microsecond; event_tstart < time_interval; event_tstart += event_stepper)
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
	    // std::clog << "Event #" << event_counter
	    // 	      << " Number of calo in event   : " << number_of_calo_in_event
	    // 	      << " Number of geiger in event : " << number_of_geiger_in_event << std::endl;

	    serializer.store(event_signals);

	    event_counter++;
	  }
      } // end of for

    std::clog << "Temp end Calo size = " << temp_signals.get_calo_signals().size()
	      << " Temp end Geiger size = " << temp_signals.get_geiger_signals().size() << std::endl;

    std::clog << "Number of events = " << collection_of_events.size() << " event counter " << event_counter << std::endl;


    std::ofstream ftmp;
    ftmp.open(output_path + "produce_self_trigger_hits_output.dat");

    ftmp << "Total number of spurious calo signals  : " << calo_tracker_spurious_signals.get_number_of_calo_signals() << std::endl;
    ftmp << "Main wall number of spurious signals   : " << calo_tracker_spurious_signals.get_number_of_main_calo_signals() << std::endl;
    ftmp << "Xwall number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_xcalo_signals() << std::endl;
    ftmp << "Gveto number of spurious signals       : " << calo_tracker_spurious_signals.get_number_of_gveto_signals() << std::endl;
    ftmp << "Geiger cell number of spurious signals : " << calo_tracker_spurious_signals.get_number_of_geiger_signals() << std::endl << std::endl;

    ftmp.close ();

    if (is_display){}
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
					    const datatools::properties & config_,
					    const std::vector<geomtools::geom_id> & gid_collection_,
					    snemo::digitization::signal_data & calo_tracker_spurious_signals_)
{
  int hit_count = 0;
  double time_interval; //= 0.1 * CLHEP::second;
  double calo_self_triggering_frequency; // = 1. / CLHEP::second; // Hertz
  double energy_min;
  double energy_max;
  datatools::invalidate(time_interval);
  datatools::invalidate(calo_self_triggering_frequency);
  datatools::invalidate(energy_min);
  datatools::invalidate(energy_max);

  if (config_.has_key("time_interval")) {
    time_interval = config_.fetch_real("time_interval");
  }

  if (config_.has_key("calo.self_trigger_frequency")) {
    calo_self_triggering_frequency = config_.fetch_real("calo.self_trigger_frequency");
  }

  if (config_.has_key("calo.energy_min")) {
    energy_min = config_.fetch_real("calo.energy_min");
  }

  if (config_.has_key("calo.energy_max")) {
    energy_max = config_.fetch_real("calo.energy_max");
  }

  double amplitude_min = energy_min / CLHEP::MeV * 300; // WIP : Hard coded for the moment
  double amplitude_max = energy_max / CLHEP::MeV * 300; // WIP : Hard coded for the moment

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

      for (std::size_t j = 0; j < number_of_calo_hit; j++)
        {
	  snemo::digitization::calo_signal & a_cs = calo_tracker_spurious_signals_.add_calo_signal();
	  a_cs.set_header(hit_count, gid_collection_[i]);
          const double timestamp = rdm_gen_->flat(0, time_interval);
	  const double amplitude = rdm_gen_->flat(amplitude_min, amplitude_max);
	  a_cs.set_data(timestamp, amplitude);
          hit_count++;
        }
    }

  return;
}

void generate_pool_of_geiger_spurious_signals(mygsl::rng * rdm_gen_,
					      const datatools::properties & config_,
					      const std::vector<geomtools::geom_id> & gid_collection_,
					      snemo::digitization::signal_data & calo_tracker_spurious_signals_)
{
  int hit_count = 0;
  double time_interval; //= 0.1 * CLHEP::second;
  double geiger_self_triggering_frequency; // = 1. / CLHEP::second; // Hertz
  double cell_dead_time;
  double retrigger_probability; // WIP : not implemented yet
  datatools::invalidate(time_interval);
  datatools::invalidate(geiger_self_triggering_frequency);
  datatools::invalidate(cell_dead_time);
  datatools::invalidate(retrigger_probability);

  if (config_.has_key("time_interval")) {
    time_interval = config_.fetch_real("time_interval");
  }

  if (config_.has_key("geiger.self_trigger_frequency")) {
    geiger_self_triggering_frequency = config_.fetch_real("geiger.self_trigger_frequency");
  }

  if (config_.has_key("geiger.dead_time")) {
    cell_dead_time = config_.fetch_real("geiger.dead_time");
  }

  if (config_.has_key("geiger.retrigger_probability")) {
    retrigger_probability = config_.fetch_real("geiger.retrigger_probability");
  }

  std::clog << "Time interval = " << time_interval << " GG ST freq = " << geiger_self_triggering_frequency << " GG cell_dead_time = " << cell_dead_time << " GG retrigger_probability = " << retrigger_probability << std::endl;

  std::clog << "GID collection size = " << gid_collection_.size() << std::endl;

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
      // std::clog << "Time interval = " << time_interval << " Time int lim = " << time_interval_limit << " tmax = " << time_max << " Freq = " << geiger_self_triggering_frequency << " Mean = " << mean_number << " GG hits = " << number_of_geiger_hit << std::endl;
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
	  // std::clog << "Timestamp generate = " << timestamp_pool[j] / CLHEP::millisecond << std::endl;
	}

      double last_anodic_time = 0;
      for (std::size_t j = 0; j < timestamp_pool.size(); j++)
	{
	  double anodic_time = 0;
	  if (particular_case) anodic_time = timestamp_pool[j];
	  else anodic_time = timestamp_pool[j] + j * cell_dead_time;

	  // Take care if new timestamp is in previous retrigger timestamp
	  bool is_in_dead_time = false;
	  if (anodic_time < last_anodic_time + cell_dead_time) {
	    is_in_dead_time = true;
	    // std::clog << "In dead time, anodic time = " << anodic_time << " Last anodic time = " << last_anodic_time << " is in dt " << is_in_dead_time << std::endl;
	  }

	  if (!is_in_dead_time) {
	    {
	      // Add signal for hit
	      // std::clog << "Anodic time = " << anodic_time << std::endl;
	      snemo::digitization::geiger_signal & a_gs = calo_tracker_spurious_signals_.add_geiger_signal();
	      a_gs.set_header(hit_count, actual_gid);
	      a_gs.set_data(anodic_time);
	      hit_count++;
	    }

	    // Retrigger probability
	    double retrigger_rdm = rdm_gen_->uniform();
	    bool is_retrigger = false;
	    if (retrigger_rdm < retrigger_probability) is_retrigger = true;
	    if (is_retrigger) {
	      double retrigger_anodic_time = anodic_time + cell_dead_time;
	      // std::clog << "retrigger_rdm = " << retrigger_rdm << " retrigger_probability = " << retrigger_probability << " Anodic time = " << anodic_time << " Retrigger anodic time " << retrigger_anodic_time << std::endl;

	      // Add signal for retrigger hit
	      // std::clog << "Anodic time = " << anodic_time << std::endl;
	      snemo::digitization::geiger_signal & a_gs = calo_tracker_spurious_signals_.add_geiger_signal();
	      a_gs.set_header(hit_count, actual_gid);
	      a_gs.set_data(retrigger_anodic_time);
	      hit_count++;
	      if (is_retrigger) last_anodic_time = retrigger_anodic_time;
	    }

	    if (!is_retrigger) last_anodic_time = anodic_time;
	  }
	}
    }

  return;
}
