// test_geiger_self_trigger.cxx
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
#include <falaise/snemo/geometry/gg_locator.h>

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
      ("help,h",    "produce help message")
      ("display,d", "display mode")
      ("trace,t",   "trace mode for debug purpose")
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
    else if (vm.count("display")) {
      is_display = true;
    }

    // Use command line arguments :
    else if (vm.count("trace")) {
      logging = datatools::logger::PRIO_TRACE;
    }

    std::clog << "Test program to add Geiger self trigger hits to SD and pass through trigger system' !" << std::endl;

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    if (input_filenames.size() == 0) {
      DT_LOG_WARNING(logging, "No input file(s) !");

      std::string input_default_file = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
      input_filenames.push_back(input_default_file);

      DT_LOG_WARNING(logging, "Default input file : " << input_filenames[0] << " !");
    }

    std::size_t file_counter = 0;
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) {
      std::clog << "File #" << file_counter << ' ' << *file << std::endl;
      file_counter++;
    }

    DT_LOG_INFORMATION(logging, "List of input file(s) : ");
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) std::clog << *file << ' ';
    std::clog << std::endl;

    // boolean for debugging (display etc)
    bool debug = false;
    if (is_display) debug = true;
    std::clog << "Debug activation : " << debug << std::endl;

    std::clog << "Test program for class ' !" << std::endl;
    int32_t seed = 314159;
    mygsl::rng random_generator;
    random_generator.initialize(seed);

    // Configure the geometry manager :
    std::string manager_config_file;
    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env(manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config (manager_config_file,
					manager_config);
    geomtools::manager my_manager;
    manager_config.update ("build_mapping", true);
    if (manager_config.has_key ("mapping.excluded_categories")) {
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
    reader_config.store("files.mode", "list");
    reader_config.store("files.list.filenames", input_filenames);
    reader_config.store("max_record_total", max_record_total);
    reader_config.store("max_record_per_file", static_cast<int>(max_events));
    reader_config.tree_dump(std::clog, "Input module configuration parameters: ");
    reader.initialize_standalone(reader_config);
    reader.tree_dump(std::clog, "Simulated data reader module");

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

    // Multi properties to configure trigger algorithm :
    datatools::multi_properties trigger_config("name", "type", "Trigger parameters multi section configuration");
    if (trigger_config_filename.empty()) trigger_config_filename = "$FALAISE_DIGITIZATION_DIR/resources/config/snemo/common/1.0/trigger_parameters.conf";

    // Read trigger config from file
    datatools::fetch_path_with_env(trigger_config_filename);
    trigger_config.read(trigger_config_filename);

    // trigger_config.tree_dump(std::clog, "My trigger configuration");

    // Creation and initialization of trigger algorithm :
    snemo::digitization::trigger_algorithm my_trigger_algo;
    my_trigger_algo.set_electronic_mapping(my_e_mapping);
    my_trigger_algo.set_clock_manager(my_clock_manager);
    my_trigger_algo.initialize(trigger_config);

    int psd_count = 0; // Event counter

    // Trigger output writers :
    // Fake trigger writer
    std::string gg_self_trigger_filename = output_path + "gg_self_trigger_SD.brio";
    dpp::output_module gg_st_writer;
    datatools::properties gg_st_config;
    gg_st_config.store ("logging.priority", "debug");
    gg_st_config.store ("files.mode", "single");
    gg_st_config.store ("files.single.filename", gg_self_trigger_filename);
    gg_st_writer.initialize_standalone(gg_st_config);

    // Event record with geiger self trigger cells :
    datatools::things ER_st;

    const double retrigger_probability = 0.1;
    const double geiger_cell_dead_time = 1 * CLHEP::millisecond;
    snemo::geometry::gg_locator gg_locator;
    gg_locator.set_geo_manager(my_manager);
    gg_locator.set_module_number(snemo::digitization::mapping::DEMONSTRATOR_MODULE_NUMBER);
    gg_locator.initialize();

    while (!reader.is_terminated())
      {
    	reader.process(ER);
    	DT_LOG_WARNING(logging, "Event #" << psd_count);
	// A plain `mctools::simulated_data' object is stored here :
	if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label))
	  {
	    // Access to the "SD" bank with a stored `mctools::simulated_data' :
	    const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);
	    mctools::simulated_data SD_modified = SD;
	    if (SD_modified.has_step_hits("gg"))
	      {
		const std::size_t number_of_geiger_hits = SD.get_number_of_step_hits("gg");
		int32_t new_hit_id_counter = number_of_geiger_hits;

		for (size_t ihit = 0; ihit < number_of_geiger_hits; ihit++)
		  {
		    mctools::base_step_hit & geiger_hit = SD_modified.grab_step_hit("gg", ihit);
		    //const double gg_hit_time       = geiger_hit.get_time_start();
		    // geiger_hit.tree_dump(std::clog, "A gg hit #" + std::to_string(geiger_hit.get_hit_id()));

		    // Retrigger probability
		    double retrigger_rdm = random_generator.uniform();
		    bool is_retrigger = false;
		    if (retrigger_rdm < retrigger_probability) is_retrigger = true;
		    if (is_retrigger) {
		      // add a geiger hit 1ms later
		      // geiger_hit.tree_dump(std::clog, "A gg hit #" + std::to_string(ihit));
		      mctools::base_step_hit a_new_geiger_hit = SD_modified.grab_step_hit("gg", ihit);

		      // to change : time and position of the hit
		      a_new_geiger_hit.set_time_start(geiger_hit.get_time_start() + geiger_cell_dead_time);
		      a_new_geiger_hit.set_hit_id(new_hit_id_counter);
		      // a_new_geiger_hit.tree_dump(std::clog, "A new gg hit #" + std::to_string(new_hit_id_counter));

		      geomtools::vector_3d gg_cell_position;
		      gg_locator.get_cell_position(a_new_geiger_hit.get_geom_id(), gg_cell_position);
		      std::clog << gg_cell_position << std::endl;
		      		// << ' ' << gg_locator.get_cell_length()
		      		// << ' ' << gg_locator.get_anode_wire_length()
		      		// << ' ' << gg_locator.get_cell_diameter()
		      		// << ' ' << gg_locator.get_anode_wire_diameter()
		      		// << std::endl;

		      // New geiger step hits : randomize x,y,z position in the cell (r, theta, z)
		      // r -> [0,Dmax/2(Rmax)], theta -> [0,2pi], z -> [-L/2, L/2]
		      double new_radius = random_generator.flat(0, gg_locator.get_cell_diameter() / 2);
		      double new_theta = random_generator.flat(0, 2 * M_PI);

		      double new_x = gg_cell_position.x() + new_radius * cos(new_theta);
		      double new_y = gg_cell_position.y() + new_radius * sin(new_theta);
		      double new_z = random_generator.flat(- gg_locator.get_anode_wire_length() / 2, gg_locator.get_anode_wire_length() / 2);

		      geomtools::vector_3d start_new_hit_position(new_x, new_y, new_z);
		      //std::clog << "New X = " << new_x << " New Y = " << new_y << " New Z = " << new_z << std::endl;

		      // Stop : x,y : center of the cell
		      geomtools::vector_3d stop_new_hit_position(gg_cell_position.x(), gg_cell_position.y(), new_z);


		      std::clog << "Start pos = " << start_new_hit_position << std::endl;
		      std::clog << "Stop pos = " << stop_new_hit_position << std::endl;

		      a_new_geiger_hit.set_position_start(start_new_hit_position);
		      a_new_geiger_hit.set_position_stop(stop_new_hit_position);

		      new_hit_id_counter++;
		      SD_modified.add_step_hit("gg") = a_new_geiger_hit;
		      a_new_geiger_hit.tree_dump(std::clog, "A new gg hit #" + std::to_string(new_hit_id_counter));


		      std::clog << std::endl;
		    }
		  }
		std::clog << "SD size = " << SD.get_number_of_step_hits("gg") << std::endl;
		std::clog << "SD modified size = " << SD_modified.get_number_of_step_hits("gg") << std::endl;
	      }

	    ER_st.add<mctools::simulated_data>("SD") = SD_modified;
	    SD_modified.tree_dump(std::clog, "Modified SD");
	    ER.tree_dump(std::clog, "ER");
	    ER_st.tree_dump(std::clog, "Modified ER");

	    // Work with SD modified (with retrigger hits) :

	    // TO DO and process at CC LYON ....

	  } // end of has SD label

	// Self trigger writer to check if we want to store SD with geiger self trigger hits
	gg_st_writer.process(ER_st);
	ER_st.clear();

	ER.clear();
    	psd_count++;
    	if (debug) std::clog << "DEBUG : psd count " << psd_count << std::endl;
    	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);

      } // end of reader is terminated

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
