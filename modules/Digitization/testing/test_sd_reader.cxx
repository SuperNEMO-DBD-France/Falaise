//test_sd_reader.cxx

// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// Boost :
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

int main( int  argc_ , char **  argv_ )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    bool is_display = false;
    std::string input_filename = "";
    int number_of_events = 10;
    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h",    "produce help message")
      ("display,d", "display mode")
      ("input,i",
       po::value<std::string>(& input_filename),
       "set an input file")
      ("nbr-events,n",
       po::value<int>(& number_of_events),
       "set the number of events")
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
    // Use command line arguments :
    if (vm.count("display")) {
      is_display = true;
    }

    std::clog << "Test program for a simple SD reader' !" << std::endl;
    std::string manager_config_file;

    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env (manager_config_file);
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

    std::string pipeline_simulated_data_filename;
    std::string SD_bank_label = "SD";

    if (input_filename.empty()) input_filename = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";

    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store ("logging.priority", "debug");
    reader_config.store ("max_record_total", number_of_events);
    reader_config.store ("files.mode", "single");
    reader_config.store ("files.single.filename", input_filename);
    reader.initialize_standalone (reader_config);
    reader.tree_dump (std::clog, "Simulated data reader module");

    datatools::things ER;

    int psd_count = 0;
    while (!reader.is_terminated())
      {
	reader.process(ER);
	// A plain `mctools::simulated_data' object is stored here :
	if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label))
	  {
	    // Access to the "SD" bank with a stored `mctools::simulated_data' :
	    const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);

	    SD.tree_dump(std::clog, "A simulated data");
	    if (is_display) std::clog << "INFO : Display mode" << std::endl;
	    if (SD.has_step_hits("gg")) {
	      const std::size_t number_of_gg_hits = SD.get_number_of_step_hits("gg");
	      for (std::size_t ihit = 0; ihit < number_of_gg_hits; ihit++)
		{
		  const mctools::base_step_hit & geiger_hit = SD.get_step_hit("gg", ihit);
		  geiger_hit.tree_dump(std::clog, "A Geiger hit #" + std::to_string(ihit));
		}
	    }

	    if (SD.has_step_hits("calo")) {
	      const std::size_t number_of_main_calo_hits = SD.get_number_of_step_hits("calo");
	      for (std::size_t ihit = 0; ihit < number_of_main_calo_hits; ihit++)
		{
		  const mctools::base_step_hit & main_calo_hit = SD.get_step_hit("calo", ihit);
		  main_calo_hit.tree_dump(std::clog, "A main calo hit #" + std::to_string(ihit));
		  // const geomtools::geom_id & calo_gid = main_calo_hit.get_geom_id();
		  // const double signal_time    = main_calo_hit.get_time_stop();
		  // const double energy_deposit = main_calo_hit.get_energy_deposit();
		  // const double amplitude      = _convert_energy_to_amplitude(energy_deposit);
		}
	    }

	    if (SD.has_step_hits("xcalo")) {

	    }

	    if (SD.has_step_hits("gveto")) {
	    }

	  }
	ER.clear();

	psd_count++;
	std::clog << "DEBUG : psd count " << psd_count << std::endl;
	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
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
