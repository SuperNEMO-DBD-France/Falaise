// trigger_algorithm_efficiency_validation.cxx
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

  // Parsing arguments
  bool is_display      = false;

  std::vector<std::string> input_filenames;
  std::string trigger_config_filename = "";
  std::string output_path = "";
  std::size_t max_events = 0;

  try {
    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h", "produce help message")
      ("display,d", "display mode")
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
      return(1);
    }

    // Use command line arguments :
    else if (vm.count("display")) {
      is_display = true;
    }

    std::clog << "Test program for class 'snemo::digitization::trigger_algorithm_efficiency_validation' !" << std::endl;

    std::size_t file_counter = 0;
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++)
      {
	std::clog << "File #" << file_counter << ' ' << *file << std::endl;
	file_counter++;
      }

    if (input_filenames.size() == 0) DT_LOG_WARNING(logging, "No input file(s) !");

    DT_LOG_INFORMATION(logging, "List of input file(s) : ");
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) std::clog << *file << ' ';

    std::clog << std::endl;
    // boolean for debugging (display etc)
    bool debug = false;
    if (is_display) debug = true;

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

    std::string pipeline_simulated_data_filename;

    // Simulated Data "SD" bank label :
    std::string SD_bank_label = "SD";
    // Trigger Decision Data "TDD" bank label :
    std::string TDD_bank_label = "TDD";

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
    if (debug) reader.tree_dump(std::clog, "Simulated data reader module");

    datatools::fetch_path_with_env(output_path);
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);

    // Name of SD output files (FT : Fake Trigger & RT: Real Trigger) :
    std::string SD_prompt_real_trigger_no   = output_path + "prompt_DT_no" + ".brio";
    std::string SD_prompt_real_trigger_yes  = output_path + "prompt_DT_yes" + ".brio";
    std::string SD_delayed_real_trigger_no  = output_path + "delayed_DT_no" + ".brio";
    std::string SD_delayed_real_trigger_yes = output_path + "delayed_DT_yes" + ".brio";

    // Event writer :
    dpp::output_module writer_1;
    datatools::properties writer_config_1;
    writer_config_1.store ("logging.priority", "debug");
    writer_config_1.store ("files.mode", "single");
    writer_config_1.store ("files.single.filename", SD_prompt_real_trigger_no);
    writer_1.initialize_standalone(writer_config_1);

    // Event writer :
    dpp::output_module writer_2;
    datatools::properties writer_config_2;
    writer_config_2.store ("logging.priority", "debug");
    writer_config_2.store ("files.mode", "single");
    writer_config_2.store ("files.single.filename", SD_prompt_real_trigger_yes);
    writer_2.initialize_standalone(writer_config_2);

    // Event writer :
    dpp::output_module writer_3;
    datatools::properties writer_config_3;
    writer_config_3.store ("logging.priority", "debug");
    writer_config_3.store ("files.mode", "single");
    writer_config_3.store ("files.single.filename", SD_delayed_real_trigger_no);
    writer_3.initialize_standalone(writer_config_3);

    // Event writer :
    dpp::output_module writer_4;
    datatools::properties writer_config_4;
    writer_config_4.store ("logging.priority", "debug");
    writer_config_4.store ("files.mode", "single");
    writer_config_4.store ("files.single.filename", SD_delayed_real_trigger_yes);
    writer_4.initialize_standalone(writer_config_4);

    // Event record :
    datatools::things ER;

    // Output ROOT file :
    std::string root_filename = output_path + "trigger_validation.root";
    datatools::fetch_path_with_env(root_filename);
    TFile* root_file = new TFile(root_filename.c_str(), "RECREATE");

    TTree* trigger_decision_tree = new TTree("TriggerDecision", "Trigger decision histograms");

    // Variables definitions :
    Int_t event_id    = 0;
    Bool_t raw_trigger_prompt_decision = false;
    Bool_t raw_trigger_delayed_decision = false;
    Int_t total_number_of_calo = 0;
    Int_t total_number_of_main_calo = 0;
    Int_t total_number_of_gveto = 0;
    Int_t total_number_of_gg_cells = 0;
    Int_t total_number_of_prompt_gg_cells = 0;
    Int_t total_number_of_delayed_gg_cells = 0;

    // Branch definitions :
    trigger_decision_tree->Branch("event_id", &event_id, "evend_id/I");
    trigger_decision_tree->Branch("raw_trigger_prompt_decision", &raw_trigger_prompt_decision, "raw_trigger_prompt_decision/O");
    trigger_decision_tree->Branch("raw_trigger_delayed_decision", &raw_trigger_delayed_decision, "raw_trigger_delayed_decision/O");
    trigger_decision_tree->Branch("total_number_of_calo", &total_number_of_calo, "total_number_of_calo/I");
    trigger_decision_tree->Branch("total_number_of_main_calo", &total_number_of_main_calo, "total_number_of_main_calo/I");
    trigger_decision_tree->Branch("total_number_of_gveto", &total_number_of_gveto, "total_number_of_gveto/I");
    trigger_decision_tree->Branch("total_number_of_gg_cells", &total_number_of_gg_cells, "total_number_of_gg_cells/I");
    trigger_decision_tree->Branch("total_number_of_prompt_gg_cells", &total_number_of_prompt_gg_cells, "total_number_of_prompt_gg_cells/I");
    trigger_decision_tree->Branch("total_number_of_delayed_gg_cells", &total_number_of_delayed_gg_cells, "total_number_of_delayed_gg_cells/I");

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
    } // end of manual configuration

    snemo::digitization::trigger_algorithm my_trigger_algo;
    my_trigger_algo.set_electronic_mapping(my_e_mapping);
    my_trigger_algo.set_clock_manager(my_clock_manager);
    my_trigger_algo.initialize(trigger_config);
    // trigger_config.tree_dump(std::clog, "My trigger config : ");

    // Creation of trigger display manager :
    snemo::digitization::trigger_display_manager my_trigger_display;
    datatools::properties trigger_display_config;
    bool calo_25ns      = true;
    bool calo_1600ns    = true;
    bool tracker_1600ns = true;
    bool coinc_1600ns   = true;
    trigger_display_config.store("calo_25ns", calo_25ns);
    trigger_display_config.store("calo_1600ns", calo_1600ns);
    trigger_display_config.store("tracker_1600ns", tracker_1600ns);
    trigger_display_config.store("coinc_1600ns", coinc_1600ns);
    my_trigger_display.initialize(trigger_display_config);

    // Internal counters
    int psd_count = 0;         // Event counter

    while (!reader.is_terminated())
      {
	event_id = psd_count;
	raw_trigger_prompt_decision = false;
	raw_trigger_delayed_decision = false;
	total_number_of_calo = 0;
	total_number_of_main_calo = 0;
	total_number_of_gveto = 0;
	total_number_of_gg_cells = 0;
	total_number_of_prompt_gg_cells = 0;
	total_number_of_delayed_gg_cells = 0;

	reader.process(ER);
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

	    if (SD.has_step_hits("calo") || SD.has_step_hits("xcalo") || SD.has_step_hits("gveto") || SD.has_step_hits("gg"))
	      {
		// Creation of a signal data object to store calo & geiger signals :
		snemo::digitization::signal_data signal_data;

		// Processing Calo signal :
		sd_2_calo_signal.process(SD, signal_data);

		// Processing Geiger signal :
		sd_2_geiger_signal.process(SD, signal_data);

		if (debug) signal_data.tree_dump(std::clog, "*** Signal Data ***", "INFO : ");

		// Creation of calo ctw data :
		snemo::digitization::calo_ctw_data my_calo_ctw_data;

		// Creation of geiger ctw data :
		snemo::digitization::geiger_ctw_data my_geiger_ctw_data;

		if (debug) my_clock_manager.tree_dump(std::clog, "Clock utils : ", "INFO : ");

		snemo::digitization::calo_tp_data my_calo_tp_data;
		// Calo signal to calo TP :
		if (signal_data.has_calo_signals())
		  {
		    // Set calo clockticks :
		    signal_2_calo_tp.set_clocktick_reference(clocktick_25_reference);
		    signal_2_calo_tp.set_clocktick_shift(clocktick_25_shift);
		    // Signal to calo TP process :
		    signal_2_calo_tp.process(signal_data, my_calo_tp_data);

		    total_number_of_main_calo = signal_data.get_number_of_main_calo_signals() + signal_data.get_number_of_xcalo_signals();
		    total_number_of_gveto     = signal_data.get_number_of_gveto_signals();
		    total_number_of_calo      = total_number_of_main_calo + total_number_of_gveto;

		    if (debug) my_calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");

		    // Calo TP to geiger CTW process :
		    calo_tp_2_ctw_0.process(my_calo_tp_data, my_calo_ctw_data);
		    calo_tp_2_ctw_1.process(my_calo_tp_data, my_calo_ctw_data);
		    calo_tp_2_ctw_2.process(my_calo_tp_data, my_calo_ctw_data);
		    if (debug) my_calo_ctw_data.tree_dump(std::clog, "Calorimeter CTW(s) data : ", "INFO : ");
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
		    if (debug) my_geiger_ctw_data.tree_dump(std::clog, "Geiger CTW(s) data : ", "INFO : ");

		    double time_limit = 5000.; // time in nanosecond

		    total_number_of_gg_cells = signal_data.get_number_of_geiger_signals();
		    total_number_of_prompt_gg_cells = signal_data.get_number_of_prompt_geiger_signals(time_limit);
		    total_number_of_delayed_gg_cells = signal_data.get_number_of_delayed_geiger_signals(time_limit);
		  } // end of if has geiger signal

		// Creation of outputs collection structures for calo and tracker
		std::vector<snemo::digitization::trigger_structures::calo_summary_record> calo_collection_records;
		std::vector<snemo::digitization::trigger_structures::tracker_record>   tracker_collection_records;
		std::vector<snemo::digitization::trigger_structures::coincidence_event_record> coincidence_collection_records;

		// Reseting trigger display
		my_trigger_display.reset_matrix_pattern();

		// Trigger process
		my_trigger_algo.process(my_calo_ctw_data,
					my_geiger_ctw_data);

		// Finale structures :
		calo_collection_records = my_trigger_algo.get_calo_records_25ns_vector();
		tracker_collection_records = my_trigger_algo.get_tracker_records_vector();
		coincidence_collection_records = my_trigger_algo.get_coincidence_records_vector();

		if (debug) my_trigger_display.display_calo_trigger_25ns(my_trigger_algo);
	        if (debug) my_trigger_display.display_calo_trigger_1600ns(my_trigger_algo);
		if (debug) my_trigger_display.display_tracker_trigger_1600ns(my_trigger_algo);
		if (debug) my_trigger_display.display_coincidence_trigger_1600ns(my_trigger_algo);

		if (debug) std::clog << "********* Size of Finale structures for one event *********" << std::endl;
		if (debug) std::clog << "Calo collection size    : " << calo_collection_records.size() << std::endl;
		if (debug) std::clog << "Tracker collection size : " << tracker_collection_records.size() << std::endl;
		if (debug) std::clog << "Coincidence collection size : " << coincidence_collection_records.size() << std::endl;

	      } // end of if has "calo" || "xcalo" || "gveto" || "gg" step hits

	    std::vector<snemo::digitization::trigger_structures::coincidence_calo_record> coincidence_collection_calo_records = my_trigger_algo.get_coincidence_calo_records_1600ns_vector();

	    // raw_trigger_prompt_decision = my_trigger_algo.get_finale_decision();
	    // raw_trigger_delayed_decision = my_trigger_algo.get_delayed_finale_decision();

	    // if (debug) std::clog << "trigger_finale_decision         [" << raw_trigger_prompt_decision << "]" << std::endl;
	    // if (debug) std::clog << "delayed trigger_finale_decision [" << raw_trigger_delayed_decision << "]" << std::endl;

	    if (debug) std::clog << "********************************************************************" << std::endl;
	    if (debug) std::clog << "Total calo             = " << total_number_of_calo             << std::endl;
	    if (debug) std::clog << "Total main calo        = " << total_number_of_main_calo        << std::endl;
	    if (debug) std::clog << "Total gveto calo       = " << total_number_of_gveto            << std::endl;
	    if (debug) std::clog << "Total GG cells         = " << total_number_of_gg_cells         << std::endl;
	    if (debug) std::clog << "Total prompt GG cells  = " << total_number_of_prompt_gg_cells  << std::endl;
	    if (debug) std::clog << "Total delayed GG cells = " << total_number_of_delayed_gg_cells << std::endl;
	    if (debug) std::clog << "********************************************************************" << std::endl;

	    // Write in several SD files depending of the trigger decision
	    // if(!raw_trigger_prompt_decision) writer_1.process(ER);
	    // else writer_2.process(ER);

	    // if (raw_trigger_prompt_decision && !raw_trigger_delayed_decision) writer_3.process(ER);
	    // else if (raw_trigger_prompt_decision && raw_trigger_delayed_decision) writer_4.process(ER);

	    my_trigger_algo.reset_data();

	  } //end of if has bank label "SD"

	trigger_decision_tree->Fill();

	ER.clear();
	psd_count++;
	if (debug) std::clog << "DEBUG : psd count " << psd_count << std::endl;
	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
      } // end of reader is terminated

    root_file->Write();
    root_file->Close();

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
