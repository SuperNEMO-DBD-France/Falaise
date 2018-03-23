// Standard library:
#include <cstdlib>
#include <string>
#include <iostream>
#include <stdexcept>
#include <limits>

// Third party:
// - Boost:
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

// - Bayeux/datatools:
#include <datatools/logger.h>
#include <datatools/exception.h>
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
#include <datatools/things.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Root :
#include "TError.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

// This project:
#include <fecom/commissioning_event.hpp>
#include <fecom/channel_mapping.hpp>
#include <fecom/data_statistics.hpp>

int main(int argc_, char ** argv_)
{
  int error_code = EXIT_SUCCESS;

  gErrorAbortLevel = 2000;

  try {

    bool is_debug = false;
    std::vector<std::string> input_filenames;// = "";
    // std::string input_filename = "";
    std::string output_path = "";
    std::size_t max_events     = 0;
    std::string input_tracker_mapping_file = "";
    std::string input_calo_mapping_file = "";
    std::size_t min_calo_hits  = 0;
    std::size_t min_tracker_hits  = 0;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h", "produce help message")
      ("debug,d", "debug mode")
      ("input,i",
       po::value<std::vector<std::string> >(& input_filenames)->multitoken(),
       "set a list of input files")
      ("output,o",
       po::value<std::string>(& output_path),
       "set the output path")
      ("max-events,M",
       po::value<std::size_t>(& max_events)->default_value(10),
       "set the maximum number of events")
      ("calo-map,c",
       po::value<std::string>(& input_calo_mapping_file),
       "set the calorimeter map")
      ("tracker-map,t",
       po::value<std::string>(& input_tracker_mapping_file),
       "set the tracker map")
      ("min-calo-hits,C",
       po::value<std::size_t>(& min_calo_hits)->default_value(0),
       "set the minimum number of calo hits in an event to record in a brio file")
      ("min-tracker-hits,T",
       po::value<std::size_t>(& min_tracker_hits)->default_value(0),
       "set the minimum number of tracker hits in an event to record in a brio file")
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
    if (vm.count("debug")) {
      is_debug = true;
    }

    datatools::logger::priority logging;
    if (is_debug) logging = datatools::logger::PRIO_DEBUG;
    else logging = datatools::logger::PRIO_INFORMATION;
    DT_LOG_INFORMATION(logging, "Entering hc_data_quality.cxx...");

    DT_LOG_INFORMATION(logging, "List of input file(s) : ");
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) std::clog << *file << ' ';
    DT_THROW_IF(input_filenames.size() == 0, std::logic_error, "No input file(s) ! ");

    DT_LOG_INFORMATION(logging, "Output path for files = " + output_path);
    if (output_path.empty()) {
      output_path = ".";
      DT_LOG_INFORMATION(logging, "No output path, default output path is = " + output_path);
    }
    datatools::fetch_path_with_env(output_path);

    std::string output_stat_filename = output_path + "output_data.stat";

    DT_LOG_INFORMATION(logging, "Data quality output file : " + output_stat_filename);
    DT_LOG_INFORMATION(logging, "Output path              : " + output_path);

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
    reader.tree_dump(std::clog, "Input module 'reader' dump");

    DT_THROW_IF(input_tracker_mapping_file.empty() || input_calo_mapping_file.empty(),
		std::logic_error,
		"Missing tracker or calo mapping file !");
    datatools::fetch_path_with_env(input_tracker_mapping_file);
    datatools::fetch_path_with_env(input_calo_mapping_file);

    DT_LOG_INFORMATION(logging, "Mapping tracker file :" + input_tracker_mapping_file);
    DT_LOG_INFORMATION(logging, "Mapping calo file    :" + input_calo_mapping_file);

    // Channel mapping :
    fecom::channel_mapping my_channel_mapping;
    my_channel_mapping.build_tracker_mapping_from_file(input_tracker_mapping_file);
    my_channel_mapping.build_calo_mapping_from_file(input_calo_mapping_file);
    my_channel_mapping.initialize();


    // Calo tracker output module event (1 Calo + >1 tracker)
    std::string calo_tracker_events_filename = output_path + "/output_calo_tracker_events.brio";
    dpp::output_module serializer;
    datatools::properties writer_config;
    writer_config.store ("logging.priority", "debug");
    writer_config.store ("files.mode", "single");
    writer_config.store ("files.single.filename", calo_tracker_events_filename);
    serializer.initialize_standalone(writer_config);

    // Calo tracker statistics :
    std::string output_data_root_filename = output_path + "/output_rootfile.root";
    DT_LOG_INFORMATION(logging, "Output filename for data in root format :" + output_data_root_filename);
    TFile * data_root_file = new TFile(output_data_root_filename.c_str(), "RECREATE");
    TTree * root_tree = new TTree("HC_data_tree","Half commissioning data");

    // General info :
    uint32_t event_number = 0;
    double   event_time_start_ns = 0;
    uint32_t number_of_calo = 0;
    uint32_t number_of_geiger_cells = 0;
    root_tree->Branch("event_number", &event_number);
    root_tree->Branch("event_tstart", &event_time_start_ns);
    root_tree->Branch("number_of_calo", &number_of_calo);
    root_tree->Branch("number_of_geiger_cells", &number_of_geiger_cells);

    // Calorimeter info :
    std::vector<uint32_t> calo_column;
    std::vector<uint32_t> calo_row;
    std::vector<double>   raw_timestamp;
    std::vector<double>   timestamp_ns;
    std::vector<bool>     high_treshold;
    std::vector<bool>     low_treshold_only;
    std::vector<uint32_t> low_threshold_trig_count;
    std::vector<int32_t>  low_threshold_time_count;
    std::vector<uint32_t> waveform_data_size;
    std::vector<uint32_t> fcr;

    // std::vector<TGraph>   raw_waveform_data; // to add ???? <- big size 1024 * 16 int * X calo hit
    // std::vector<int> x_value_sample;
    // for (auto isample = 0; isample < fecom::calo_constants::MAX_NUMBER_OF_SAMPLES; isample++) x_value_sample.push_back(isample);

    std::vector<int32_t>  raw_baseline;
    std::vector<double>   baseline_volt;
    std::vector<int32_t>  raw_peak;
    std::vector<double>   peak_volt;
    std::vector<double>   raw_charge;
    std::vector<double>   charge_picocoulomb;
    std::vector<bool>     raw_charge_overflow;
    std::vector<uint32_t> rising_cell;
    std::vector<uint32_t> rising_offset;
    std::vector<double>   rising_time_ns;
    std::vector<uint32_t> falling_cell;
    std::vector<uint32_t> falling_offset;
    std::vector<uint32_t> falling_time_ns;
    root_tree->Branch("vector_calo_column", &calo_column);
    root_tree->Branch("vector_calo_row", &calo_row);
    root_tree->Branch("vector_raw_timestamp", &raw_timestamp);
    root_tree->Branch("vector_timestamp_ns", &timestamp_ns);
    root_tree->Branch("vector_high_treshold", &high_treshold);
    root_tree->Branch("vector_low_treshold_only", &low_treshold_only);
    root_tree->Branch("vector_low_threshold_trig_count", &low_threshold_trig_count);
    root_tree->Branch("vector_low_threshold_time_count", &low_threshold_time_count);
    root_tree->Branch("vector_waveform_data_size", &waveform_data_size);
    root_tree->Branch("vector_fcr", &fcr);

    // Branch of std::vector<TGraph> not working, error : The class requested (vector<TGraph>) for the branch "vector_raw_waveform_data" refer to an stl collection and do not have a compiled CollectionProxy.  Please generate the dictionary for this class (vector<TGraph>)
    // We don't add waveforms to the root tree for the moment.

    // root_tree->Branch("vector_raw_waveform_data", "std::vector<TGraph>", &raw_waveform_data);
    root_tree->Branch("vector_raw_baseline", &raw_baseline);
    root_tree->Branch("vector_baseline_volt", &baseline_volt);
    root_tree->Branch("vector_raw_peak", &raw_peak);
    root_tree->Branch("vector_peak_volt", &peak_volt);
    root_tree->Branch("vector_raw_charge", &raw_charge);
    root_tree->Branch("vector_charge_picocoulomb", &charge_picocoulomb);
    root_tree->Branch("vector_raw_charge_overflow", &raw_charge_overflow);
    root_tree->Branch("vector_rising_cell", &rising_cell);
    root_tree->Branch("vector_rising_offset", &rising_offset);
    root_tree->Branch("vector_rising_time_ns", &rising_time_ns);
    root_tree->Branch("vector_falling_cell", &falling_cell);
    root_tree->Branch("vector_falling_offset", &falling_offset);
    root_tree->Branch("vector_falling_time_ns", &falling_time_ns);

    // Tracker info :
    std::vector<geomtools::geom_id> cell_gid;
    std::vector<uint32_t> cell_layer;
    std::vector<uint32_t> cell_row;
    std::vector<double> anodic_t0_ns;
    std::vector<double> anodic_t1_ns;
    std::vector<double> anodic_t2_ns;
    std::vector<double> anodic_t3_ns;
    std::vector<double> anodic_t4_ns;
    std::vector<double> bot_cathodic_time_ns;
    std::vector<double> top_cathodic_time_ns;
    // root_tree->Branch("vector_cell_gid", &cell_gid);
    root_tree->Branch("vector_cell_layer", &cell_layer);
    root_tree->Branch("vector_cell_row", &cell_row);
    root_tree->Branch("vector_anodic_t0_ns", &anodic_t0_ns);
    root_tree->Branch("vector_anodic_t1_ns", &anodic_t1_ns);
    root_tree->Branch("vector_anodic_t2_ns", &anodic_t2_ns);
    root_tree->Branch("vector_anodic_t3_ns", &anodic_t3_ns);
    root_tree->Branch("vector_anodic_t4_ns", &anodic_t4_ns);
    root_tree->Branch("vector_bot_cathodic_time_ns", &bot_cathodic_time_ns);
    root_tree->Branch("vector_top_cathodic_time_ns", &top_cathodic_time_ns);

    std::string output_quality_histograms = output_path + "/hc_quality_histograms.root";
    DT_LOG_INFORMATION(logging, "Output histograms for data quality :" + output_quality_histograms);
    TFile * histogram_file = new TFile(output_quality_histograms.c_str(), "RECREATE");

    // Event record :
    datatools::things ER;

    // Simulated Data "SD" bank label :
    std::string HCRD_bank_label = "HCRD";

    // Statistics class
    fecom::data_statistics cts;
    cts.initialize();

    int modulo = 1000;

    DT_LOG_INFORMATION(logging, "Reading commisioning events...");
    DT_LOG_INFORMATION(logging, "...");


    DT_THROW_IF(!cts.is_initialized(),
		std::logic_error,
		"Data statistics class is not initialized !");

    while (!reader.is_terminated()) {

      reader.process(ER);

      // A plain `fecom::commissioning' object is stored here :
      if (ER.has(HCRD_bank_label) && ER.is_a<fecom::commissioning_event>(HCRD_bank_label))
	{
	  fecom::commissioning_event & CE = ER.grab<fecom::commissioning_event>(HCRD_bank_label);

	  if (is_debug) CE.tree_dump(std::clog, "A com event");

	  CE.set_channel_mapping(my_channel_mapping);

	  /*********************/
	  /* FILLING ROOT TREE */
	  /*********************/

	  event_number = CE.get_event_id().get_event_number();
	  event_time_start_ns = CE.get_time_start_ns();

	  number_of_calo = CE.get_number_of_calo();
	  number_of_geiger_cells = CE.get_number_of_tracker();

	  if ((event_number % modulo) == 1 ){
	    DT_LOG_DEBUG(logging, "Event number = " << event_number);
	  }

	  // loop on all calo hits in each commissioning event


	  if (CE.has_calo_hits()) {
	    for (auto icalo = CE.get_calo_hit_collection().begin();
		 icalo != CE.get_calo_hit_collection().end();
		 icalo++) {
	      geomtools::geom_id calo_eid = icalo->electronic_id;

	      // Check if the calo hit is in the bimap :
	      if (my_channel_mapping.is_calo_channel_in_map(calo_eid))
		{
		  if (is_debug) icalo->tree_dump(std::clog, "A calo hit");

		  geomtools::geom_id calo_gid;
		  my_channel_mapping.get_geometric_id_from_electronic_id(calo_eid,
									 calo_gid);

		  // Calo hit ID is an electronic ID, mapping is needed to have the corresponding GID :
		  uint16_t column = calo_gid.get(fecom::calo_constants::COLUMN_INDEX);
		  uint16_t row = calo_gid.get(fecom::calo_constants::ROW_INDEX);

		  // calo_raw_charge_trig_row_TH1F[row]->Fill(icalo->raw_charge);
		  // hit_calo_count_total_TH2F->Fill(column, row);

		  // Fill raw data calo in root tree :
		  calo_column.push_back(column);
		  calo_row.push_back(row);
		  raw_timestamp.push_back(icalo->raw_tdc);
		  timestamp_ns.push_back(icalo->tdc_ns);
		  high_treshold.push_back(icalo->high_threshold);
		  low_treshold_only.push_back(icalo->low_threshold);
		  low_threshold_trig_count.push_back(icalo->low_threshold_trig_count);
		  low_threshold_time_count.push_back(icalo->low_threshold_time_count);
		  waveform_data_size.push_back(icalo->waveform_data_size);
		  fcr.push_back(icalo->fcr);

		  // std::vector<int> intermediate_raw_waveform_data;
		  // for (auto isample = 0; isample < fecom::calo_constants::MAX_NUMBER_OF_SAMPLES; isample ++) {
		  //   intermediate_raw_waveform_data.push_back(icalo->raw_waveform_data[isample]);
		  // }

		  // TGraph my_graph(x_value_sample.size(),
		  // 		  & x_value_sample[0],
		  // 		  & intermediate_raw_waveform_data[0]);
		  // raw_waveform_data.push_back(my_graph);

		  raw_baseline.push_back(icalo->raw_baseline);
		  baseline_volt.push_back(icalo->baseline_volt);
		  raw_peak.push_back(icalo->raw_peak);
		  peak_volt.push_back(icalo->peak_volt);
		  raw_charge.push_back(icalo->raw_charge);
		  charge_picocoulomb.push_back(icalo->charge_picocoulomb);
		  raw_charge_overflow.push_back(icalo->raw_charge_overflow);
		  rising_cell.push_back(icalo->rising_cell);
		  rising_offset.push_back(icalo->rising_offset);
		  rising_time_ns.push_back(icalo->rising_time_ns);
		  falling_cell.push_back(icalo->falling_cell);
		  falling_offset.push_back(icalo->falling_offset);
		  falling_time_ns.push_back(icalo->falling_time_ns);
		}
	    }
	  }

	  // loop on all tracker hits in each commissioning event
	  if (CE.has_tracker_hits()) {
	    std::size_t tracker_hit_counter = 0;

	    for (auto itrack = CE.get_tracker_hit_collection().begin();
		 itrack != CE.get_tracker_hit_collection().end();
		 itrack++) {
	      geomtools::geom_id the_actual_cell_gid = itrack->fecom_geom_id;

	      if (itrack->fecom_geom_id.is_valid()) {
		unsigned int layer = itrack->fecom_geom_id.get(fecom::tracker_constants::LAYER_INDEX);
		unsigned int row = itrack->fecom_geom_id.get(fecom::tracker_constants::ROW_INDEX);
		// hit_tracker_count_total_TH2F->Fill(row, layer);

		// Fill raw data calo in root tree :
		cell_gid.push_back(itrack->fecom_geom_id);
		cell_layer.push_back(layer);
		cell_row.push_back(row);
		if (itrack->has_anodic_t0()) anodic_t0_ns.push_back(itrack->anodic_t0_ns);
		if (itrack->has_anodic_t1()) anodic_t1_ns.push_back(itrack->anodic_t1_ns);
		if (itrack->has_anodic_t2()) anodic_t2_ns.push_back(itrack->anodic_t2_ns);
		if (itrack->has_anodic_t3()) anodic_t3_ns.push_back(itrack->anodic_t3_ns);
		if (itrack->has_anodic_t4()) anodic_t4_ns.push_back(itrack->anodic_t4_ns);
		if (itrack->has_bot_cathodic_time()) bot_cathodic_time_ns.push_back(itrack->bot_cathodic_time_ns);
		if (itrack->has_top_cathodic_time()) top_cathodic_time_ns.push_back(itrack->top_cathodic_time_ns);
		tracker_hit_counter++;
	      }
	      else {
		//std::clog << "GID is not valid, it seems that the tracker hit was not in the mapping" << std::endl;
		// Error not throw because it will be useful for debug purpose
		// In commissioning, we had some data from the FEB not mapped with a physical geiger cell
	      }
	    }
	  } // end of if has tracker hit

	  if (is_debug) std::clog << "Event number before filling = " << event_number << std::endl;
	  // Fill the tree for each commissioning event :
	  root_tree->Fill();



	  /**********************/
	  /* FILLING HISTOGRAMS */
	  /**********************/

	  // General :
	  cts.number_of_events++;

	  if (CE.has_calo_hits()) {
	    cts.number_of_events_with_calo++;

	    for (auto icalo = CE.get_calo_hit_collection().begin();
		 icalo != CE.get_calo_hit_collection().end();
		 icalo++) {

	      geomtools::geom_id calo_eid = icalo->electronic_id;

	      // Check if the calo hit is in the bimap :
	      if (my_channel_mapping.is_calo_channel_in_map(calo_eid))
		{
		  geomtools::geom_id calo_gid;
		  my_channel_mapping.get_geometric_id_from_electronic_id(calo_eid,
									 calo_gid);
		  // Calo hit ID is an electronic ID, mapping is needed to have the corresponding GID :
		  uint16_t column = calo_gid.get(fecom::calo_constants::COLUMN_INDEX);
		  uint16_t row = calo_gid.get(fecom::calo_constants::ROW_INDEX);

		  cts.calo_distrib_TH2F->Fill(column, row);
		  cts.charge_picocoulomb_calo_TH1F[row]->Fill(std::abs(icalo->charge_picocoulomb));
		  if (icalo->high_threshold) {
		    cts.charge_picocoulomb_ht_calo_TH1F[row]->Fill(std::abs(icalo->charge_picocoulomb));
		    cts.calo_distrib_ht_TH2F->Fill(column, row);
		  } else {
		    cts.charge_picocoulomb_no_ht_calo_TH1F[row]->Fill(std::abs(icalo->charge_picocoulomb));
		    cts.calo_distrib_no_ht_TH2F->Fill(column, row);
		  }

		  if (CE.get_calo_hit_collection().size() > 1) {
		    if (icalo->high_threshold) {
		      double DT_calo_tref = icalo->tdc_ns + icalo->falling_time_ns - event_time_start_ns;
		      if (DT_calo_tref != 0) cts.calo_delta_t_calo_tref_TH1F->Fill(DT_calo_tref);
		    }
		  }
		}
	      else {
		std::string unmapped_channel = "CATEGORY=CALO TRIGGER_ID=" + std::to_string(icalo->trigger_id) + " SLOT=" + std::to_string(icalo->electronic_id.get(fecom::calo_constants::SLOT_INDEX)) + " CHANNEL=" + std::to_string(icalo->electronic_id.get(fecom::calo_constants::CHANNEL_INDEX));
		cts.unmapped_channels_in_raw_data.push_back(unmapped_channel);
	      }

	    } // end of for icalo

	    cts.mean_number_of_calo.first = (cts.mean_number_of_calo.first * cts.mean_number_of_calo.second + CE.get_number_of_calo()) / (cts.mean_number_of_calo.second + 1);
	    cts.mean_number_of_calo.second++;
	    cts.mean_number_of_calo_ht.first = (cts.mean_number_of_calo_ht.first * cts.mean_number_of_calo_ht.second + CE.get_number_of_calo_ht()) / (cts.mean_number_of_calo_ht.second + 1);
	    cts.mean_number_of_calo_ht.second++;
	  } // end of has calo hits

	  if (CE.has_tracker_hits())
	    {
	      cts.number_of_events_with_tracker++;
	      for (auto itrack = CE.get_tracker_hit_collection().begin();
		   itrack != CE.get_tracker_hit_collection().end();
		   itrack++) {
		geomtools::geom_id the_actual_cell_gid = itrack->fecom_geom_id;

		if (itrack->fecom_geom_id.is_valid()) {
		  unsigned int layer = itrack->fecom_geom_id.get(fecom::tracker_constants::LAYER_INDEX);
		  unsigned int row = itrack->fecom_geom_id.get(fecom::tracker_constants::ROW_INDEX);
		  cts.tracker_distrib_TH2F->Fill(row, layer);

		  if (CE.is_only_tracker()) cts.tracker_only_distrib_TH2F->Fill(row, layer);
		  cts.cell_registers[layer].hit_counter++;
		  if (itrack->has_anodic_t0()) cts.cell_registers[layer].R0_count++;
		  if (itrack->has_anodic_t1()) cts.cell_registers[layer].R1_count++;
		  if (itrack->has_anodic_t2()) cts.cell_registers[layer].R2_count++;
		  if (itrack->has_anodic_t3()) cts.cell_registers[layer].R3_count++;
		  if (itrack->has_anodic_t4()) cts.cell_registers[layer].R4_count++;
		  if (itrack->has_bot_cathodic_time()) cts.cell_registers[layer].Rbot_cath_count++;
		  if (itrack->has_top_cathodic_time()) cts.cell_registers[layer].Rtop_cath_count++;
		  if (itrack->has_anodic_t0() && itrack->has_bot_cathodic_time()) cts.cell_registers[layer].bot_cathode_efficiency++;

		} else {
		  // std::clog << "GID is not valid, it seems that the tracker hit was not in the mapping" << std::endl;
		  // Error not throw because it will be useful for debug purpose
		  // In commissioning, we had some data from the FEB not mapped with a physical geiger cell
		  std::string unmapped_channel = "CATEGORY=TRACKER TRIGGER_ID=" + std::to_string(itrack->trigger_id);
		  cts.unmapped_channels_in_raw_data.push_back(unmapped_channel);
		}
	      }

	      cts.mean_number_of_tracker_cells.first = (cts.mean_number_of_tracker_cells.first * cts.mean_number_of_tracker_cells.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells.second + 1);
	      cts.mean_number_of_tracker_cells.second++;
	    }

	  // Calo only :
	  if (CE.is_only_calo()) cts.number_of_calo_only_events++;

	  // Tracker only :
	  if (CE.is_only_tracker()) {
	    cts.number_of_tracker_only_events++;
	    cts.mean_number_of_tracker_cells_if_0_calo_ht.first = (cts.mean_number_of_tracker_cells_if_0_calo_ht.first * cts.mean_number_of_tracker_cells_if_0_calo_ht.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells_if_0_calo_ht.second + 1);
	    cts.mean_number_of_tracker_cells_if_0_calo_ht.second++;
	  }

	  // Fill histograms if calo tracker :
	  if (CE.is_calo_tracker()) {
	    cts.number_of_calo_tracker_events++;

	    for (auto icalo = CE.get_calo_hit_collection().begin();
	    	 icalo != CE.get_calo_hit_collection().end();
	    	 icalo++) {
	      geomtools::geom_id calo_eid = icalo->electronic_id;

	      if (my_channel_mapping.is_calo_channel_in_map(calo_eid)) {

		geomtools::geom_id calo_gid;
		my_channel_mapping.get_geometric_id_from_electronic_id(calo_eid,
								       calo_gid);

		uint16_t column = calo_gid.get(fecom::calo_constants::COLUMN_INDEX);
		uint16_t row = calo_gid.get(fecom::calo_constants::ROW_INDEX);

		cts.calo_tracker_calo_distrib_TH2F->Fill(column, row);

		if (icalo->high_threshold || icalo->low_threshold) {
	    	  cts.calo_tracker_calo_ht_distrib_TH2F->Fill(column, row);
		  cts.calo_ht_number_tracker_number_distrib_TH2F->Fill(CE.get_number_of_tracker(), CE.get_number_of_calo_ht());

	    	  double delta_t_calo_calo = icalo->tdc_ns - event_time_start_ns;
	    	  if (delta_t_calo_calo != 0) cts.calo_tracker_delta_t_calo_tref_TH1F->Fill(delta_t_calo_calo);
	    	}
	      }
	    } // end of for icalo
	    cts.mean_number_of_calo_ht_if_calo_tracker.first = (cts.mean_number_of_calo_ht_if_calo_tracker.first * cts.mean_number_of_calo_ht_if_calo_tracker.second + CE.get_number_of_calo_ht()) / (cts.mean_number_of_calo_ht_if_calo_tracker.second + 1);
	    cts.mean_number_of_calo_ht_if_calo_tracker.second++;

	    // First loop to search anode minimum and cathode minimum :
	    double anode_min_time_ns = 0;
	    double cathode_min_time_ns = 0;
	    for (auto itrack = CE.get_tracker_hit_collection().begin();
	  	 itrack != CE.get_tracker_hit_collection().end();
	  	 itrack++) {
	      if (itrack->has_anodic_t0()) {
	  	if (anode_min_time_ns == 0 || itrack->anodic_t0_ns < anode_min_time_ns) anode_min_time_ns = itrack->anodic_t0_ns;
	      }
	      if (itrack->has_bot_cathodic_time()) {
	  	if (cathode_min_time_ns == 0 || itrack->bot_cathodic_time_ns < cathode_min_time_ns) cathode_min_time_ns = itrack->bot_cathodic_time_ns;
	      }
	    } // end of itrack
	    cts.mean_number_of_tracker_cells_if_calo_tracker.first = (cts.mean_number_of_tracker_cells_if_calo_tracker.first * cts.mean_number_of_tracker_cells_if_calo_tracker.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells_if_calo_tracker.second + 1);
	    cts.mean_number_of_tracker_cells_if_calo_tracker.second++;

	    if (CE.get_number_of_calo_ht() == 1) {
	      cts.mean_number_of_tracker_cells_if_1_calo_ht.first = (cts.mean_number_of_tracker_cells_if_1_calo_ht.first * cts.mean_number_of_tracker_cells_if_1_calo_ht.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells_if_1_calo_ht.second + 1);
	      cts.mean_number_of_tracker_cells_if_1_calo_ht.second++;
	    } else if (CE.get_number_of_calo_ht() == 2) {
	      cts.mean_number_of_tracker_cells_if_2_calo_ht.first = (cts.mean_number_of_tracker_cells_if_2_calo_ht.first * cts.mean_number_of_tracker_cells_if_2_calo_ht.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells_if_2_calo_ht.second + 1);
	      cts.mean_number_of_tracker_cells_if_2_calo_ht.second++;
	    } else {
	      cts.mean_number_of_tracker_cells_if_3p_calo_ht.first = (cts.mean_number_of_tracker_cells_if_3p_calo_ht.first * cts.mean_number_of_tracker_cells_if_3p_calo_ht.second + CE.get_number_of_tracker()) / (cts.mean_number_of_tracker_cells_if_3p_calo_ht.second + 1);
	      cts.mean_number_of_tracker_cells_if_3p_calo_ht.second++;
	    }

	    for (auto itrack = CE.get_tracker_hit_collection().begin();
	  	 itrack != CE.get_tracker_hit_collection().end();
	  	 itrack++) {

	      geomtools::geom_id the_actual_cell_gid = itrack->fecom_geom_id;

	      if (itrack->fecom_geom_id.is_valid()) {
		unsigned int layer = itrack->fecom_geom_id.get(fecom::tracker_constants::LAYER_INDEX);
		unsigned int row = itrack->fecom_geom_id.get(fecom::tracker_constants::ROW_INDEX);

		cts.cell_registers_if_calo_tracker[layer].hit_counter++;
		if (itrack->has_anodic_t0()) cts.cell_registers_if_calo_tracker[layer].R0_count++;
		if (itrack->has_anodic_t1()) cts.cell_registers_if_calo_tracker[layer].R1_count++;
		if (itrack->has_anodic_t2()) cts.cell_registers_if_calo_tracker[layer].R2_count++;
		if (itrack->has_anodic_t3()) cts.cell_registers_if_calo_tracker[layer].R3_count++;
		if (itrack->has_anodic_t4()) cts.cell_registers_if_calo_tracker[layer].R4_count++;
		if (itrack->has_bot_cathodic_time()) cts.cell_registers_if_calo_tracker[layer].Rbot_cath_count++;
		if (itrack->has_top_cathodic_time()) cts.cell_registers_if_calo_tracker[layer].Rtop_cath_count++;
		if (itrack->has_anodic_t0() && itrack->has_bot_cathodic_time()) cts.cell_registers_if_calo_tracker[layer].bot_cathode_efficiency++;

		cts.calo_tracker_tracker_distrib_TH2F->Fill(row, layer);

		if (itrack->has_anodic_t0()) {

		  double DT_anode_tref = itrack->anodic_t0_ns - event_time_start_ns;
		  if (DT_anode_tref != 0) cts.calo_tracker_delta_t_anode_tref_TH1F->Fill(DT_anode_tref);

		  double DT_anode_anode = itrack->anodic_t0_ns - anode_min_time_ns;
		  if (DT_anode_anode != 0) cts.calo_tracker_delta_t_anode_anode_TH1F->Fill(DT_anode_anode);
		}

		if (itrack->has_bot_cathodic_time()) {

		  double DT_cathode_tref = itrack->bot_cathodic_time_ns - event_time_start_ns;
		  cts.calo_tracker_delta_t_cathode_tref_TH1F->Fill(DT_cathode_tref);

		  if (itrack->has_anodic_t0()) {
		    double DT_cathode_anode_same_hit = itrack->bot_cathodic_time_ns - itrack->anodic_t0_ns;
		    cts.calo_tracker_delta_t_anode_cathode_same_hit_TH1F->Fill(DT_cathode_anode_same_hit);
		  }
		}
	      } // end of gid is valid

	    } // end of itrack

	    bool save_calo_tracker_event = true;
	    if (CE.get_number_of_calo_ht() < min_calo_hits) {
	      save_calo_tracker_event = false;
	    }
	    if (CE.get_number_of_tracker() < min_tracker_hits) {
	      save_calo_tracker_event = false;
	    }
	    if (save_calo_tracker_event) {
	      serializer.process(ER);
	    }

	  } // end of is calo tracker

	  // Clear vectors
	  event_number = 0;
	  event_time_start_ns = 0;
	  number_of_calo = 0;
	  number_of_geiger_cells = 0;
	  calo_column.clear();
	  calo_row.clear();
	  raw_timestamp.clear();
	  timestamp_ns.clear();
	  high_treshold.clear();
	  low_treshold_only.clear();
	  low_threshold_trig_count.clear();
	  low_threshold_time_count.clear();
	  waveform_data_size.clear();
	  fcr.clear();
	  // raw_waveform_data.clear();
	  raw_baseline.clear();
	  baseline_volt.clear();
	  raw_peak.clear();
	  peak_volt.clear();
	  raw_charge.clear();
	  charge_picocoulomb.clear();
	  raw_charge_overflow.clear();
	  rising_cell.clear();
	  rising_offset.clear();
	  rising_time_ns.clear();
	  falling_cell.clear();
	  falling_offset.clear();
	  falling_time_ns.clear();

	  cell_gid.clear();
	  cell_layer.clear();
	  cell_row.clear();
	  anodic_t0_ns.clear();
	  anodic_t1_ns.clear();
	  anodic_t2_ns.clear();
	  anodic_t3_ns.clear();
	  anodic_t4_ns.clear();
	  bot_cathodic_time_ns.clear();
	  top_cathodic_time_ns.clear();

	} // end of has HCRD bank label

      ER.clear();
      // event_number++;
    } // end of while is reader

    // Write tree and histograms in file:
    data_root_file->cd();
    root_tree->Write("", TObject::kOverwrite);
    data_root_file->Close();

    // Save histograms in an other file
    histogram_file->cd();
    cts.save_in_root_file(histogram_file);
    histogram_file->Close();

    std::ofstream ofstat(output_stat_filename.c_str());
    cts.print(ofstat);
    ofstat.close();

    DT_LOG_INFORMATION(logging, "The end.");
  } catch (std::exception & error) {
    std::cerr << "error: " << error.what() << std::endl;
    error_code = EXIT_FAILURE;
  }
  return error_code;
}
