// produce_geiger_sd_modified.cxx
// Standard libraries :
#include <iostream>

// - Bayeux/geomtools:
#include <geomtools/manager.h>
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

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    // Parsing arguments
    bool is_display      = false;

    std::vector<std::string> input_filenames;
    std::string output_path = "";
    std::size_t max_events = 0;
    std::string modification_mode = "";
    double trigger_probability = 0;

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
      ("event_number,n",
       po::value<std::size_t>(& max_events)->default_value(10),
       "set the maximum number of events")
      ("modification_mode,m",
       po::value<std::string>(& modification_mode),
       "set the mode to modify SD with new Geiger hits, same_cell_trigger or neighbourg_trigger")
      ("trigger_probability,p",
       po::value<double>(& trigger_probability),
       "set the trigger probability depending of the mode")
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
      std::cout << "Example : produce_geiger_sd_modified -n 5 -o /tmp/ -m same_cell_trigger -p 0.01" << std::endl;
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

    std::clog << "Test program to add Geiger neighbour induce trigger hits to SD and pass through trigger system' !" << std::endl;

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    if (input_filenames.size() == 0) {
      DT_LOG_WARNING(logging, "No input file(s), default input file use : !");
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

    DT_THROW_IF(modification_mode.empty(), std::logic_error, "No modification mode specified, choose a mode between : same_cell_trigger, neighbourg_trigger or self_trigger ! ");

    // boolean for debugging (display etc)
    bool debug = false;
    if (is_display) debug = true;
    std::clog << "Debug activation : " << debug << std::endl;

    std::clog << "Program to modify 'Simulated Data' and add Geiger hits depending of the mode : self trigger, neighbourg trigger or again trigger for a given cell' !" << std::endl;
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

    // SD with neighbourg trigger GG cells :
    std::string modif_sd_filename_prefix = "modified_SD_";
    dpp::output_module modif_sd_writer;
    datatools::properties modif_sd_config;
    modif_sd_config.store("logging.priority", "debug");
    modif_sd_config.store("max_record_per_file", 100000);
    modif_sd_config.store("files.mode", "incremental");
    modif_sd_config.store("files.incremental.path", output_path);
    modif_sd_config.store ("files.incremental.prefix", "modified_SD_");
    modif_sd_config.store ("files.incremental.extension", ".brio");
    modif_sd_config.store ("files.incremental.start", 0);
    modif_sd_config.store ("files.incremental.increment", 1);
    modif_sd_config.store ("files.incremental.stop", 1000);
    modif_sd_writer.initialize_standalone(modif_sd_config);
    
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

    int psd_count = 0; // Event counter

    // Event record with modified SD inside :
    datatools::things ER_modified;

    snemo::geometry::gg_locator gg_locator;
    gg_locator.set_geo_manager(my_manager);
    gg_locator.set_module_number(0);
    gg_locator.initialize();

    const double geiger_cell_dead_time = 1 * CLHEP::millisecond;

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

		const std::size_t initial_number_of_geiger_hits = SD_modified.get_number_of_step_hits("gg");
		int32_t new_hit_id_counter = initial_number_of_geiger_hits;

		// 3 different modes : same_cell_trigger, neighbourg_trigger or self_trigger :
		if (modification_mode == "same_cell_trigger") {
		  DT_LOG_WARNING(logging, "Same cell trigger mode : a given Geiger cell has a chance to trigger again after 1 ms (dead time recovery) !");

		  // To introduce in a file mode or an option to change the proba :
		  const double same_cell_trigger_probability = trigger_probability;

		  for (size_t ihit = 0; ihit < initial_number_of_geiger_hits; ihit++)
		    {
		      mctools::base_step_hit & geiger_hit = SD_modified.grab_step_hit("gg", ihit);
		      // geiger_hit.tree_dump(std::clog, "A gg hit #" + std::to_string(geiger_hit.get_hit_id()));

		      // Same cell trigger probability
		      double retrigger_rdm = random_generator.uniform();
		      bool is_retrigger = false;
		      if (retrigger_rdm < same_cell_trigger_probability) is_retrigger = true;
		      if (is_retrigger) {
			// Add a new geiger hit in the same GID (cell) 1 ms after recovery dead time

			mctools::base_step_hit a_new_geiger_hit = SD_modified.grab_step_hit("gg", ihit);

			// Change time and position of the new geiger hit :
			a_new_geiger_hit.set_time_start(geiger_hit.get_time_start() + geiger_cell_dead_time);
			a_new_geiger_hit.set_hit_id(new_hit_id_counter);

			geomtools::vector_3d gg_cell_position;
			gg_locator.get_cell_position(a_new_geiger_hit.get_geom_id(), gg_cell_position);

			// New geiger step hit : randomize x,y,z position in the cell (r, theta, z)
			// r -> [0,Dmax/2], theta -> [0,2pi], z -> [-L/2, L/2]
			double new_radius = random_generator.flat(0, gg_locator.get_cell_diameter() / 2);
			double new_theta  = random_generator.flat(0, 2 * M_PI);

			double new_x = gg_cell_position.x() + new_radius * cos(new_theta);
			double new_y = gg_cell_position.y() + new_radius * sin(new_theta);
			double new_z = random_generator.flat(- gg_locator.get_anode_wire_length() / 2, gg_locator.get_anode_wire_length() / 2);

			geomtools::vector_3d start_new_hit_position(new_x, new_y, new_z);

			// Stop : x,y : center of the cell
			geomtools::vector_3d stop_new_hit_position(gg_cell_position.x(), gg_cell_position.y(), new_z);

			// std::clog << "Start pos = " << start_new_hit_position << std::endl;
			// std::clog << "Stop pos = " << stop_new_hit_position << std::endl;

			a_new_geiger_hit.set_position_start(start_new_hit_position);
			a_new_geiger_hit.set_position_stop(stop_new_hit_position);

			new_hit_id_counter++;
			SD_modified.add_step_hit("gg") = a_new_geiger_hit;
			//a_new_geiger_hit.tree_dump(std::clog, "A new gg hit #" + std::to_string(new_hit_id_counter));


			//std::clog << std::endl;
		      }
		    }

		} // end of same_cell_trigger mode

		else if (modification_mode == "neighbourg_trigger") {
		  DT_LOG_WARNING(logging, "Neighbourg trigger mode : a given Geiger cell has a chance to trigger his (first) neighbourgs !");

		  /****   WARNING : only first neighbourg of the primary tracks can be trigger !       ****/
		  /****   TO DO : model for the total chamber and hits propagation (use recursivity ?) ****/

		  const double neighbour_trigger_probability = trigger_probability;
		  const double induce_geiger_cell_max_time = 40 * CLHEP::microsecond;

		  for (size_t ihit = 0; ihit < initial_number_of_geiger_hits; ihit++)
		    {
		      mctools::base_step_hit & geiger_hit = SD_modified.grab_step_hit("gg", ihit);
		      // geiger_hit.tree_dump(std::clog, "A gg hit #" + std::to_string(geiger_hit.get_hit_id()));

		      std::vector<geomtools::geom_id> neighbours_ids;
		      gg_locator.get_neighbours_ids(geiger_hit.get_geom_id(), neighbours_ids);
		      for (std::size_t ineigh = 0; ineigh < neighbours_ids.size(); ineigh++) {

			// check if the neighbourh is not already trigger
			bool is_neigh_already_trigger = false;

			const std::size_t number_of_geiger_hits_modified = SD_modified.get_number_of_step_hits("gg");

			for (size_t jhit = 0; jhit < number_of_geiger_hits_modified; jhit++) {
			  mctools::base_step_hit & a_geiger_hit = SD_modified.grab_step_hit("gg", jhit);
			  if (neighbours_ids[ineigh] == a_geiger_hit.get_geom_id()) is_neigh_already_trigger = true;
			}
			if (!is_neigh_already_trigger){
			  double neigh_trigger_rdm = random_generator.uniform();
			  bool is_trigger = false;
			  if (neigh_trigger_rdm < neighbour_trigger_probability) is_trigger = true;
			  if (is_trigger) {
			    mctools::base_step_hit a_new_geiger_hit = SD_modified.grab_step_hit("gg", ihit);

			    // Change hit time position
			    // Trigger neighbour cell time up to 40 us (these K.Erramane p.160)
			    // 2 / 2.5 neighbour trigger / event (p.165)

			    a_new_geiger_hit.set_hit_id(new_hit_id_counter);

			    double new_time_start = random_generator.flat(geiger_hit.get_time_start(), induce_geiger_cell_max_time);

			    a_new_geiger_hit.set_time_start(new_time_start);
			    a_new_geiger_hit.set_geom_id(neighbours_ids[ineigh]);

			    geomtools::vector_3d gg_cell_position;
			    gg_locator.get_cell_position(a_new_geiger_hit.get_geom_id(), gg_cell_position);

			    // New geiger step hits : randomize x,y,z position in the cell (r, theta, z)
			    // r -> [0,Dmax/2(Rmax)], theta -> [0,2pi], z -> [-L/2, L/2]
			    double new_radius = random_generator.flat(0, gg_locator.get_cell_diameter() / 2);
			    double new_theta  = random_generator.flat(0, 2 * M_PI);

			    double new_x = gg_cell_position.x() + new_radius * cos(new_theta);
			    double new_y = gg_cell_position.y() + new_radius * sin(new_theta);
			    double new_z = random_generator.flat(- gg_locator.get_anode_wire_length() / 2, gg_locator.get_anode_wire_length() / 2);

			    geomtools::vector_3d start_new_hit_position(new_x, new_y, new_z);
			    //std::clog << "New X = " << new_x << " New Y = " << new_y << " New Z = " << new_z << std::endl;

			    // Stop : x,y : center of the cell
			    geomtools::vector_3d stop_new_hit_position(gg_cell_position.x(), gg_cell_position.y(), new_z);

			    // std::clog << "Start pos = " << start_new_hit_position << std::endl;
			    // std::clog << "Stop pos = " << stop_new_hit_position << std::endl;

			    a_new_geiger_hit.set_position_start(start_new_hit_position);
			    a_new_geiger_hit.set_position_stop(stop_new_hit_position);

			    new_hit_id_counter++;
			    SD_modified.add_step_hit("gg") = a_new_geiger_hit;

			    // geiger_hit.tree_dump(std::clog, "A gg hit #" + std::to_string(geiger_hit.get_hit_id()));
			    // a_new_geiger_hit.tree_dump(std::clog, "A new gg hit #" + std::to_string(a_new_geiger_hit.get_hit_id()));

			  } // end of is_trigger

			} // end of if neigh_already_trigger

		      } // end of ineigh

		    } // end of ihit

		} // end of else if neighbourg trigger

		// std::clog << "SD size = " << SD.get_number_of_step_hits("gg") << std::endl;
		// std::clog << "SD modified size = " << SD_modified.get_number_of_step_hits("gg") << std::endl;


	      } // end of has step hit "gg"

	    // Add modified SD in the datatools::things
	    ER_modified.add<mctools::simulated_data>("SD") = SD_modified;

	    // Checks :
	    // SD_modified.tree_dump(std::clog, "Modified SD");
	    // ER.tree_dump(std::clog, "ER");
	    // ER_modified.tree_dump(std::clog, "Modified ER");

	  } // end of has SD bank label

	// Record the new event and store it
	modif_sd_writer.process(ER_modified);

	// Clear things for next event
	ER.clear();
	ER_modified.clear();

	psd_count++;

	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);

      } // end of while reader is terminated

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
