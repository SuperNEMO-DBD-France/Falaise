// fldigi_delayed_alpha_near_source_validation.cxx
// Standard libraries :
#include <fstream>
#include <iostream>

// - Bayeux/datatools:
#include <datatools/clhep_units.h>
#include <datatools/io_factory.h>
#include <datatools/properties.h>
#include <datatools/utils.h>

// - Bayeux/geomtools:
#include <bayeux/geomtools/manager.h>

// - Bayeux/mctools:
#include <mctools/simulated_data.h>

// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// Root :
#include <TH1F.h>
#include <TH2F.h>
#include "TFile.h"
#include "TTree.h"

int main(int argc_, char** argv_) {
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  // Parsing arguments
  int iarg = 1;
  bool is_input_file = false;
  bool is_output_path = false;
  bool is_event_number = false;
  bool is_verbose = false;
  bool is_help = false;

  std::string input_filename;
  std::string output_path;
  int arg_event_number = -1;

  while (iarg < argc_) {
    std::string arg = argv_[iarg];

    if (arg == "-i" || arg == "--input") {
      is_input_file = true;
      input_filename = argv_[++iarg];
    }

    else if (arg == "-op" || arg == "--output-path") {
      is_output_path = true;
      output_path = argv_[++iarg];
    }

    else if (arg == "-n" || arg == "--number") {
      is_event_number = true;
      arg_event_number = atoi(argv_[++iarg]);
    }

    else if (arg == "-v" || arg == "--verbosity") {
      is_verbose = true;
    }

    else if (arg == "-h" || arg == "--help") {
      is_help = true;
    }

    iarg++;
  }

  if (is_help) {
    std::cerr << std::endl
              << "Usage :" << std::endl
              << std::endl
              << "$ BuildProducts/bin/fldigi_delayed_alpha_validation [OPTIONS] [ARGUMENTS]"
              << std::endl
              << std::endl
              << "Allowed options: " << std::endl
              << "-h  [ --help ]        produce help message" << std::endl
              << "-i  [ --input ]       set an input file" << std::endl
              << "-op [ --output_path ] set the output path for producted files" << std::endl
              << "-n  [ --number ]      set the number of events" << std::endl
              << "-v  [ --verbosity]    add verbosity level" << std::endl
              << "-p  [ --print ]       print files in PDF/PNG" << std::endl
              << "Example : " << std::endl
              << std::endl
              << "$ BuildProducts/bin/fldigi_delayed_alpha_validation -n 1000 -i "
                 "${FALAISE_DIGITIZATION_TESTING_DIR}/data/"
                 "Se82_0nubb-source_strips_bulk_SD_10_events.brio -op "
                 "${FALAISE_DIGITIZATION_TESTING_DIR}/output_default -v -p"
              << std::endl
              << std::endl;
    return 0;
  }

  // Process
  try {
    std::clog << "Little test program for analysis of alpha delayed number of geiger cells hit !"
              << std::endl;

    // Manager.conf
    std::string manager_config_file;
    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/3.0/manager.conf";
    datatools::fetch_path_with_env(manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config(manager_config_file, manager_config);
    geomtools::manager my_manager;
    manager_config.update("build_mapping", true);
    if (manager_config.has_key("mapping.excluded_categories")) {
      manager_config.erase("mapping.excluded_categories");
    }
    my_manager.initialize(manager_config);

    std::string pipeline_simulated_data_filename;

    // Simulated Data "SD" bank label :
    std::string SD_bank_label = "SD";

    datatools::fetch_path_with_env(input_filename);
    if (is_input_file) {
      pipeline_simulated_data_filename = input_filename;
    } else {
      pipeline_simulated_data_filename =
          "${FALAISE_DIGITIZATION_TESTING_DIR}/data/"
          "Se82_0nubb-source_strips_bulk_SD_10_events.brio";
      input_filename = pipeline_simulated_data_filename;
    }
    datatools::fetch_path_with_env(pipeline_simulated_data_filename);

    // Number of events :
    int event_number = -1;
    if (is_event_number)
      event_number = arg_event_number;
    else
      event_number = 10;
    // Output path :
    if (is_output_path)
      output_path = output_path;
    else
      output_path = "${FALAISE_DIGITIZATION_TESTING_DIR}/output_default/";
    datatools::fetch_path_with_env(output_path);

    // Event reader :
    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store("logging.priority", "debug");
    reader_config.store("max_record_total", event_number);
    reader_config.store("files.mode", "single");
    reader_config.store("files.single.filename", pipeline_simulated_data_filename);
    reader.initialize_standalone(reader_config);
    reader.tree_dump(std::clog, "Simulated data reader module");

    std::string run_number = input_filename;
    std::size_t found = 0;
    found = run_number.find("run");
    if (found != std::string::npos) {
      run_number.erase(0, found);
      found = run_number.find_first_of("/");
      run_number.erase(found, run_number.size());
    } else {
      run_number = "";
    }

    std::string delayed_DT = input_filename;
    found = delayed_DT.find("delayed");
    if (found != std::string::npos) {
      delayed_DT.erase(0, found);
      delayed_DT = "_" + delayed_DT;
    } else {
      found = delayed_DT.find_last_of("/");
      delayed_DT.erase(0, found);
      found = delayed_DT.find(".brio");
      delayed_DT.erase(found, delayed_DT.size());
    }
    std::string output_filename = output_path + run_number + delayed_DT + ".log";

    std::string five_or_more_gg_cells_delayed =
        output_path + "five_or_more_gg_cells_delayed" + delayed_DT;
    std::string zero_gg_cells_delayed = output_path + "zero_gg_cells_delayed" + delayed_DT;

    // Event writer :
    dpp::output_module writer_1;
    datatools::properties writer_config_1;
    writer_config_1.store("logging.priority", "debug");
    writer_config_1.store("files.mode", "single");
    writer_config_1.store("files.single.filename", five_or_more_gg_cells_delayed);
    writer_1.initialize_standalone(writer_config_1);

    dpp::output_module writer_2;
    datatools::properties writer_config_2;
    writer_config_2.store("logging.priority", "debug");
    writer_config_2.store("files.mode", "single");
    writer_config_2.store("files.single.filename", zero_gg_cells_delayed);
    writer_2.initialize_standalone(writer_config_2);

    static const int MAXIMUM_DELAYED_TIME =
        8000;  // in nanosecond, linked with the digi trigger and geiger drift

    // Event record :
    datatools::things ER;

    int psd_count = 0;  // Event counter

    int no_delayed_geiger_cells_count = 0;
    int one_delayed_geiger_cells_count = 0;
    int two_delayed_geiger_cells_count = 0;
    int three_delayed_geiger_cells_count = 0;
    int four_delayed_geiger_cells_count = 0;
    int five_or_more_delayed_geiger_cells_count = 0;

    while (!reader.is_terminated()) {
      reader.process(ER);
      // A plain `mctools::simulated_data' object is stored here :
      if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label)) {
        // Access to the "SD" bank with a stored `mctools::simulated_data' :
        const mctools::simulated_data& SD = ER.get<mctools::simulated_data>(SD_bank_label);

        int number_of_gg_cells = 0;
        int number_of_delayed_gg_cells = 0;
        int number_of_not_delayed_gg_cells = 0;

        if (SD.has_step_hits("gg")) {
          const size_t number_of_hits = SD.get_number_of_step_hits("gg");

          // New sd bank
          mctools::simulated_data flaged_sd = SD;

          // We have to flag the gg cells already hit before (maybe take into account the dead time
          // of a GG cell)
          for (size_t ihit = 0; ihit < number_of_hits; ihit++) {
            mctools::base_step_hit& geiger_hit = flaged_sd.grab_step_hit("gg", ihit);
            for (size_t jhit = ihit + 1; jhit < number_of_hits; jhit++) {
              mctools::base_step_hit& other_geiger_hit = flaged_sd.grab_step_hit("gg", jhit);
              if (geiger_hit.get_geom_id() == other_geiger_hit.get_geom_id()) {
                const double gg_hit_time = geiger_hit.get_time_start();
                const double other_gg_hit_time = other_geiger_hit.get_time_start();
                if (gg_hit_time > other_gg_hit_time) {
                  bool geiger_already_hit = true;
                  if (!geiger_hit.get_auxiliaries().has_flag("geiger_already_hit"))
                    geiger_hit.grab_auxiliaries().store("geiger_already_hit", geiger_already_hit);
                } else {
                  bool geiger_already_hit = true;
                  if (!other_geiger_hit.get_auxiliaries().has_flag("geiger_already_hit"))
                    other_geiger_hit.grab_auxiliaries().store("geiger_already_hit",
                                                              geiger_already_hit);
                }
              }
            }
          }

          // if (is_verbose) SD.tree_dump(std::clog, "Simulated Data : ", "INFO : ");
          mctools::simulated_data::hit_handle_collection_type BSHC = flaged_sd.get_step_hits("gg");
          if (is_verbose) std::clog << "BSCH step hits # = " << BSHC.size() << std::endl;
          number_of_gg_cells = BSHC.size();
          int count = 0;
          for (mctools::simulated_data::hit_handle_collection_type::const_iterator i = BSHC.begin();
               i != BSHC.end(); i++) {
            const mctools::base_step_hit& BSH = i->get();

            if (BSH.get_auxiliaries().has_flag("geiger_already_hit") ||
                BSH.get_auxiliaries().has_flag("other_geiger_already_hit")) {
            } else {
              // extract the corresponding geom ID:
              // const geomtools::geom_id & geiger_gid = BSH.get_geom_id();

              // if (is_verbose) BSH.tree_dump(std::clog, "A Geiger Base Step Hit : ", "INFO : ");

              double time_start = BSH.get_time_start();
              geomtools::vector_3d position_start_vector = BSH.get_position_start();
              geomtools::vector_3d position_stop_vector = BSH.get_position_stop();
              geomtools::vector_3d momentum_start_vector = BSH.get_momentum_start();

              if (time_start > MAXIMUM_DELAYED_TIME)
                number_of_delayed_gg_cells++;  // time in ns
              else
                number_of_not_delayed_gg_cells++;
              count++;
            }
          }
          if (is_verbose)
            std::clog << "GG Cells #" << number_of_gg_cells << " Delayed GG Cells #"
                      << number_of_delayed_gg_cells << " Not Delayed GG Cells #"
                      << number_of_not_delayed_gg_cells << std::endl;
        }

        if (number_of_delayed_gg_cells == 0) {
          no_delayed_geiger_cells_count++;
          writer_2.process(ER);
        }

        else if (number_of_delayed_gg_cells == 1) {
          one_delayed_geiger_cells_count++;
        }

        else if (number_of_delayed_gg_cells == 2) {
          two_delayed_geiger_cells_count++;
        }

        else if (number_of_delayed_gg_cells == 3) {
          three_delayed_geiger_cells_count++;
        }

        else if (number_of_delayed_gg_cells == 4) {
          four_delayed_geiger_cells_count++;
        }

        else {
          five_or_more_delayed_geiger_cells_count++;
          writer_1.process(ER);
        }
      }

      ER.clear();
      psd_count++;
      if (is_verbose) std::clog << "DEBUG : psd count " << psd_count << std::endl;
      DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
    }

    std::ofstream file(output_filename.c_str(), std::ios::out | std::ios::trunc);
    if (file) {
      file << "Input file  = " << input_filename << std::endl;
      file << "Output path = " << output_filename << std::endl;
      file << "Number of events    = " << psd_count << std::endl;
      file << "No delayed GG cells = " << no_delayed_geiger_cells_count << std::endl;
      file << "1 delayed GG cells  = " << one_delayed_geiger_cells_count << std::endl;
      file << "2 delayed GG cells  = " << two_delayed_geiger_cells_count << std::endl;
      file << "3 delayed GG cells  = " << three_delayed_geiger_cells_count << std::endl;
      file << "4 delayed GG cells  = " << four_delayed_geiger_cells_count << std::endl;
      file << "5+ delayed GG cells = " << five_or_more_delayed_geiger_cells_count << std::endl;

      file.close();
    } else {
      std::cerr << "Erreur à l'ouverture !" << std::endl;
    }

    std::clog << "The end." << std::endl;
  }

  catch (std::exception& error) {
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
