// test_simulated_data_reading.cxx
// Standard library:
#include <iostream>
#include <exception>
#include <cstdlib>
#include <string>

// Third party:
// - Bayeux/datatools:
#include <datatools/logger.h>
#include <datatools/utils.h>
#include <datatools/io_factory.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
#include <mctools/simulated_data_reader.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// Boost :
#include <boost/program_options.hpp>

int main( int  argc_ , char **argv_  )
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

  try {
    std::clog << "Test program for read calibrated datas !" << std::endl;
    std::string pipeline_simulated_data_filename;
    std::string SD_bank_label = "SD";

    if(!input_filename.empty()){
      pipeline_simulated_data_filename = input_filename;
    }else{
      pipeline_simulated_data_filename = "${FALAISE_DIGITIZATION_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
    }

    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store ("logging.priority", "debug");
    reader_config.store ("max_record_total", max_events);
    reader_config.store ("files.mode", "single");
    reader_config.store ("files.single.filename", pipeline_simulated_data_filename);
    reader.initialize_standalone (reader_config);
    reader.tree_dump (std::clog, "Simulated data reader module");

    datatools::things ER;

    int psd_count = 0;
    while (!reader.is_terminated()) {
      reader.process(ER);
      // A plain `mctools::simulated_data' object is stored here :
      if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label)) {
        // Access to the "SD" bank with a stored `mctools::simulated_data' :
	const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);
	SD.tree_dump(std::cerr);
	std::clog << "DEBUG : has SD_BANK" << std::endl;
	std::clog << "DEBUG : has data " << SD.has_data() << std::endl;
	std::clog << "DEBUG : has data " << SD.get_vertex() << std::endl;
	std::clog << "DEBUG : has step_hits " << SD.has_step_hits("gg") << std::endl;
	if( SD.has_step_hits("gg")){
	  size_t number = SD.get_number_of_step_hits("gg");
	  for (unsigned int i = 0; i < number; i++)
	    {
	      const mctools::base_step_hit & gg_hit = SD.get_step_hit("gg", i);
	      std::clog << "DEBUG : hit id = " << gg_hit.get_hit_id() << ": GID = " << gg_hit.get_geom_id() << std::endl;
	    }
	}
      }
      // CF README.RST pour display graphique avec loader de manager.conf
      // -> /home/guillaume/data/Bayeux/Bayeux-trunk/source/bxmctools/examples/ex00
      ER.clear();

      psd_count++;
      std::clog << "DEBUG : psd count " << psd_count << std::endl;
      DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
    }
    std::clog << "Number of processed simulated data records: " << psd_count << std::endl;

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
