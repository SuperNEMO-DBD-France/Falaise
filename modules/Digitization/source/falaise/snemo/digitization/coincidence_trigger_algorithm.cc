// snemo/digitization/coincidence_trigger_algorithm.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Ourselves:
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/coincidence_trigger_algorithm.h>

namespace snemo {

namespace digitization {

const uint32_t coincidence_trigger_algorithm::SIZE_OF_RESERVED_COINCIDENCE_CALO_RECORDS;
const uint32_t coincidence_trigger_algorithm::SIZE_OF_L2_COINCIDENCE_DECISION_GATE;

coincidence_trigger_algorithm::coincidence_base_record::coincidence_base_record() {
  coincidence_base_record::reset();
  return;
}

void coincidence_trigger_algorithm::coincidence_base_record::reset() {
  calo_zoning_word[0].reset();
  calo_zoning_word[1].reset();
  total_multiplicity_side_0.reset();
  total_multiplicity_side_1.reset();
  LTO_side_0 = false;
  LTO_side_1 = false;
  total_multiplicity_gveto.reset();
  LTO_gveto = false;
  xt_info_bitset.reset();
  single_side_coinc = true;
  total_multiplicity_threshold = false;
  decision = false;
  return;
}

void coincidence_trigger_algorithm::coincidence_base_record::display() const {
  std::clog << "XTS|L|HG|L|L|H1|H0| ZONING S1| ZONING S0 " << std::endl;
  std::clog << xt_info_bitset << ' ';
  std::clog << LTO_gveto << ' ';
  std::clog << total_multiplicity_gveto << ' ';
  std::clog << LTO_side_1 << ' ';
  std::clog << LTO_side_0 << ' ';
  std::clog << total_multiplicity_side_1 << ' ';
  std::clog << total_multiplicity_side_0 << ' ';
  for (unsigned int iside = trigger_info::NSIDES - 1; iside > 0; iside--) {
    for (unsigned int izone = trigger_info::NZONES - 1; izone > 0; izone--) {
      std::clog << calo_zoning_word[iside][izone];
    }
    std::clog << ' ';
  }
  std::clog << std::endl;
  std::clog << "Single Side coinc : " << single_side_coinc
            << "  |  Threshold total mult : " << total_multiplicity_threshold << std::endl;
  return;
}

coincidence_trigger_algorithm::coincidence_calo_record::coincidence_calo_record() {
  coincidence_calo_record::reset();
  return;
}

void coincidence_trigger_algorithm::coincidence_calo_record::reset() {
  coincidence_base_record::reset();
  clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
  return;
}

void coincidence_trigger_algorithm::coincidence_calo_record::display() const {
  std::clog
      << "************************************************************************************"
      << std::endl;
  std::clog << "*************************** Coincidence calo record ********************"
            << std::endl;
  std::clog << "*************************** Clocktick 1600 = " << clocktick_1600ns
            << " ***************************" << std::endl;
  coincidence_base_record::display();
  std::clog << "Coincidence calo record decision : [" << decision << "]" << std::endl;
  std::clog << std::endl;
  return;
}

coincidence_trigger_algorithm::coincidence_event_record::coincidence_event_record() {
  coincidence_event_record::reset();
  return;
}

void coincidence_trigger_algorithm::coincidence_event_record::reset() {
  coincidence_base_record::reset();
  clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
  trigger_mode = INVALID;
  zoning_word[0].reset();
  zoning_word[1].reset();
  for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
    for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
      tracker_finale_data_per_zone[iside][izone].reset();
    }
  }
  return;
}

void coincidence_trigger_algorithm::coincidence_event_record::display() const {
  std::clog
      << "************************************************************************************"
      << std::endl;
  std::clog << "*************************** Coincidence event record ********************"
            << std::endl;
  std::clog << "*************************** Clocktick 1600 = " << clocktick_1600ns
            << " ***************************" << std::endl;

  coincidence_base_record::display();
  std::clog << "Bitset : [NSZL NSZR L M R O I] " << std::endl;
  for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
    std::clog << "Side = " << iside << " | ";
    for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
      std::clog << "[" << tracker_finale_data_per_zone[iside][izone] << "] ";
    }  // end of izone
    std::clog << std::endl;
  }
  std::clog << "Coincidence zoning word : " << std::endl;
  std::clog << "ZW [0] = " << zoning_word[0] << std::endl;
  std::clog << "ZW [1] = " << zoning_word[1] << std::endl;
  std::clog << "Coincidence trigger mode : [" << trigger_mode << "]" << std::endl;
  std::clog << "Coincidence event record decision : [" << decision << "]" << std::endl;
  return;
}

coincidence_trigger_algorithm::L2_coincidence_decision::L2_coincidence_decision() {
  L2_coincidence_decision::reset();
  return;
}

void coincidence_trigger_algorithm::L2_coincidence_decision::reset() {
  L2_coincidence_decision_bool = false;
  L2_clocktick_decision = clock_utils::INVALID_CLOCKTICK;
  trigger_mode = INVALID;
  return;
}

void coincidence_trigger_algorithm::L2_coincidence_decision::display() const {
  std::clog << "L2 coincidence decision structure : " << std::endl;
  std::clog << "L2 decision   [" << L2_coincidence_decision_bool << "]" << std::endl;
  std::clog << "CT decision  : " << L2_clocktick_decision << std::endl;
  std::clog << "Trigger mode : " << trigger_mode << std::endl;
  return;
}

coincidence_trigger_algorithm::previous_event_record::previous_event_record() {
  previous_event_record::reset();
  return;
}

void coincidence_trigger_algorithm::previous_event_record::reset() {
  coincidence_base_record::reset();
  previous_clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
  counter_1600ns = 0;
  for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
    for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
      tracker_finale_data_per_zone[iside][izone].reset();
    }
  }
  return;
}

void coincidence_trigger_algorithm::previous_event_record::display() const {
  std::clog
      << "************************************************************************************"
      << std::endl;
  std::clog << "*************************** Previous event record ********************"
            << std::endl;
  std::clog << "*************************** Previous clocktick 1600 = " << previous_clocktick_1600ns
            << " ********************" << std::endl;
  std::clog << "*************************** Counter 1600 = " << counter_1600ns
            << " ***************************" << std::endl;
  coincidence_base_record::display();
  std::clog << "Bitset : [NSZL NSZR L M R O I] " << std::endl;
  for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
    std::clog << "Side = " << iside << " | ";
    for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
      std::clog << "[" << tracker_finale_data_per_zone[iside][izone] << "] ";
    }  // end of izone
    std::clog << std::endl;
  }
  std::clog << "Coincidence zoning word : " << std::endl;
  std::clog << "ZW [0] = " << zoning_word[0] << std::endl;
  std::clog << "ZW [1] = " << zoning_word[1] << std::endl;
  std::clog << std::endl;
  return;
}

coincidence_trigger_algorithm::coincidence_trigger_algorithm() {
  _initialized_ = false;
  _electronic_mapping_ = 0;
  _coincidence_calorimeter_gate_size_ = 0;
  _previous_event_circular_buffer_depth_ = 0;
  _coincidence_calo_records_.reserve(SIZE_OF_RESERVED_COINCIDENCE_CALO_RECORDS);
  _caraco_decision_ = false;
  _delayed_coincidence_decision_ = false;
  _previous_event_records_.reset();
  return;
}

coincidence_trigger_algorithm::~coincidence_trigger_algorithm() {
  if (is_initialized()) {
    reset();
  }
  return;
}

void coincidence_trigger_algorithm::set_electronic_mapping(
    const electronic_mapping& my_electronic_mapping_) {
  DT_THROW_IF(
      is_initialized(), std::logic_error,
      "Coincidence trigger algorithm is already initialized, electronic mapping can't be set ! ");
  _electronic_mapping_ = &my_electronic_mapping_;
  return;
}

bool coincidence_trigger_algorithm::has_calorimeter_gate_size() const {
  return _coincidence_calorimeter_gate_size_ != 0;
}

void coincidence_trigger_algorithm::set_calorimeter_gate_size(unsigned int calorimeter_gate_size_) {
  DT_THROW_IF(is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is already initialized, coincidence calorimeter gate "
              "size can't be set ! ");
  _coincidence_calorimeter_gate_size_ = calorimeter_gate_size_;
  return;
}

bool coincidence_trigger_algorithm::has_previous_event_buffer_depth() const {
  return _previous_event_circular_buffer_depth_ != 0;
}

void coincidence_trigger_algorithm::set_previous_event_buffer_depth(
    unsigned int previous_event_circular_buffer_depth_) {
  DT_THROW_IF(is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is already initialized, previous event buffer depth "
              "can't be set ! ");
  _previous_event_circular_buffer_depth_ = previous_event_circular_buffer_depth_;
  return;
}

unsigned int coincidence_trigger_algorithm::get_previous_event_buffer_depth() const {
  return _previous_event_circular_buffer_depth_;
}

void coincidence_trigger_algorithm::initialize_simple() {
  datatools::properties dummy_config;
  initialize(dummy_config);
  return;
}

void coincidence_trigger_algorithm::initialize(const datatools::properties& config_) {
  DT_THROW_IF(is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is already initialized ! ");
  if (!has_calorimeter_gate_size()) {
    if (config_.has_key("calorimeter_gate_size")) {
      int calorimeter_gate_size = config_.fetch_integer("calorimeter_gate_size");
      DT_THROW_IF(calorimeter_gate_size <= 0, std::domain_error,
                  "Invalid value calorimeter_gate_size !");
      set_calorimeter_gate_size((unsigned int)calorimeter_gate_size);
    }
  }

  if (!has_previous_event_buffer_depth()) {
    if (config_.has_key("previous_event_buffer_depth")) {
      int previous_event_buffer_depth = config_.fetch_integer("previous_event_buffer_depth");
      DT_THROW_IF(previous_event_buffer_depth <= 0, std::domain_error,
                  "Invalid negative previous event buffer depth!");
      set_previous_event_buffer_depth((unsigned int)previous_event_buffer_depth);
    }
  }

  _initialized_ = true;
  return;
}

bool coincidence_trigger_algorithm::is_initialized() const { return _initialized_; }

void coincidence_trigger_algorithm::reset() {
  DT_THROW_IF(!is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is not initialized, it can't be reset ! ");
  _electronic_mapping_ = 0;
  reset_data();
  _initialized_ = false;
  return;
}

void coincidence_trigger_algorithm::reset_data() {
  DT_THROW_IF(!is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is not initialized, it can't be reset ! ");
  _coincidence_calo_records_.clear();
  _caraco_decision_ = false;
  _delayed_coincidence_decision_ = false;
  _pair_records_.clear();
  _previous_event_records_.reset();
  _L2_coincidence_decison_records_.clear();
  return;
}

const std::vector<coincidence_trigger_algorithm::coincidence_calo_record>
coincidence_trigger_algorithm::get_coincidence_calo_records_vector() const {
  return _coincidence_calo_records_;
}

bool coincidence_trigger_algorithm::get_caraco_decision() const { return _caraco_decision_; }

bool coincidence_trigger_algorithm::get_delayed_coincidence_decision() const {
  return _delayed_coincidence_decision_;
}

void coincidence_trigger_algorithm::_preparing_calo_coincidence(
    const std::vector<calo_trigger_algorithm::calo_summary_record>& calo_records_) {
  std::vector<calo_trigger_algorithm::calo_summary_record>::const_iterator it =
      calo_records_.begin();

  for (; it != calo_records_.end(); it++) {
    const calo_trigger_algorithm::calo_summary_record a_ctrec = *it;

    if (a_ctrec.calo_finale_decision == true) {
      uint32_t ctrec_clocktick_1600ns = 0;
      _compute_clocktick_1600ns(a_ctrec.clocktick_25ns, ctrec_clocktick_1600ns);

      if (_coincidence_calo_records_.size() == 0) {
        // No coincidence calo records, creation of the first and the 10 following EMPTY
        coincidence_trigger_algorithm::coincidence_calo_record first_coincidence_calo_record;
        first_coincidence_calo_record.clocktick_1600ns = ctrec_clocktick_1600ns;
        _coincidence_calo_records_.push_back(first_coincidence_calo_record);

        if (_coincidence_calo_records_.size() != 0) {
          for (unsigned int iclocktick = first_coincidence_calo_record.clocktick_1600ns + 1;
               iclocktick <
               first_coincidence_calo_record.clocktick_1600ns + _coincidence_calorimeter_gate_size_;
               iclocktick++) {
            coincidence_trigger_algorithm::coincidence_calo_record on_gate_coincidence_calo_record;
            on_gate_coincidence_calo_record = first_coincidence_calo_record;
            on_gate_coincidence_calo_record.clocktick_1600ns = iclocktick;
            _coincidence_calo_records_.push_back(on_gate_coincidence_calo_record);
          }
        }
      }

      bool coinc_calo_record_find = false;

      // Search if a coincidence calo record for clocktick ctrec 25 -> 1600 is already existing
      for (unsigned int i = 0; i < _coincidence_calo_records_.size(); i++) {
        if (ctrec_clocktick_1600ns == _coincidence_calo_records_[i].clocktick_1600ns) {
          // Updating existing coincidence calo record
          coinc_calo_record_find = true;
          _update_coinc_calo_record(a_ctrec, _coincidence_calo_records_[i]);

          // Updating the following coincidence calo record
          // Have to check if they are in the gate or outside
          if (i != _coincidence_calo_records_.size() - 1) {
            for (unsigned int j = i + 1; j < _coincidence_calo_records_.size(); j++) {
              if (_coincidence_calo_records_[j].clocktick_1600ns <=
                  _coincidence_calo_records_[i].clocktick_1600ns +
                      _coincidence_calorimeter_gate_size_) {
                uint32_t clocktick_1600_before_modification =
                    _coincidence_calo_records_[j].clocktick_1600ns;
                _coincidence_calo_records_[j] = _coincidence_calo_records_[i];
                _coincidence_calo_records_[j].clocktick_1600ns = clocktick_1600_before_modification;
              }
            }  // end of for j
          }

          // Have to grow back the gate and create new coincidence calo record
          unsigned int actual_index = i;
          unsigned int index_min = std::floor(actual_index / _coincidence_calorimeter_gate_size_) *
                                   _coincidence_calorimeter_gate_size_;
          unsigned int index_max = index_min + _coincidence_calorimeter_gate_size_ - 1;
          uint32_t clocktick_to_create =
              ctrec_clocktick_1600ns + _coincidence_calorimeter_gate_size_;
          uint32_t clocktick_max_1600 = _coincidence_calo_records_[index_max].clocktick_1600ns;

          for (unsigned int j = clocktick_max_1600 + 1; j <= clocktick_to_create; j++) {
            bool no_same_clocktick = true;
            for (unsigned int k = actual_index + 1; k < _coincidence_calo_records_.size(); k++) {
              // Check if the CT i between max and wanted to create exist in the coinc calo record
              // list :
              if (j == _coincidence_calo_records_[k].clocktick_1600ns) {
                no_same_clocktick = false;
              }
            }  // end of for k

            if (no_same_clocktick == true) {
              coincidence_trigger_algorithm::coincidence_calo_record
                  grow_back_coincidence_calo_record = _coincidence_calo_records_[index_max];
              grow_back_coincidence_calo_record.clocktick_1600ns = j;
              _coincidence_calo_records_.push_back(grow_back_coincidence_calo_record);
            }
          }  // end of for j
        }
      }  // end of for i

      // If not, create the first and the 10 following empty then updated
      if (coinc_calo_record_find == false) {
        coincidence_trigger_algorithm::coincidence_calo_record new_coincidence_calo_record;
        new_coincidence_calo_record.clocktick_1600ns = ctrec_clocktick_1600ns;
        _coincidence_calo_records_.push_back(new_coincidence_calo_record);
        for (uint32_t iclocktick = new_coincidence_calo_record.clocktick_1600ns + 1;
             iclocktick <
             new_coincidence_calo_record.clocktick_1600ns + _coincidence_calorimeter_gate_size_;
             iclocktick++) {
          coincidence_trigger_algorithm::coincidence_calo_record
              new_on_gate_coincidence_calo_record;
          new_on_gate_coincidence_calo_record = new_coincidence_calo_record;
          new_on_gate_coincidence_calo_record.clocktick_1600ns = iclocktick;
          _coincidence_calo_records_.push_back(new_on_gate_coincidence_calo_record);
        }
      }
    }  // end of for it calo records
  }

  // std::vector<coincidence_trigger_algorithm::coincidence_calo_record>::const_iterator it_coinc =
  // _coincidence_calo_records_.begin(); for (; it_coinc != _coincidence_calo_records_.end();
  // it_coinc++)
  // 	{
  // 	  const coincidence_trigger_algorithm::coincidence_calo_record a_coinc_calo_record =
  // *it_coinc; 	  a_coinc_calo_record.display();
  // 	}

  return;
}

void coincidence_trigger_algorithm::_update_coinc_calo_record(
    const calo_trigger_algorithm::calo_summary_record& a_calo_summary_record_25ns_,
    coincidence_trigger_algorithm::coincidence_calo_record& a_coinc_calo_record_1600ns_) {
  // Update information but keep the clocktick of the 1600 structure

  // Initial values of coinc calo record 1600 ns :
  unsigned int multiplicity_side_0 =
      a_coinc_calo_record_1600ns_.total_multiplicity_side_0.to_ulong();
  unsigned int multiplicity_side_1 =
      a_coinc_calo_record_1600ns_.total_multiplicity_side_1.to_ulong();
  unsigned int multiplicity_gveto = a_coinc_calo_record_1600ns_.total_multiplicity_gveto.to_ulong();
  bool lto_side_0 = a_coinc_calo_record_1600ns_.LTO_side_0;
  bool lto_side_1 = a_coinc_calo_record_1600ns_.LTO_side_1;
  bool lto_gveto = a_coinc_calo_record_1600ns_.LTO_gveto;
  std::bitset<calo_trigger_algorithm::ZONING_PER_SIDE_BITSET_SIZE> zoning_word_side_0 =
      a_coinc_calo_record_1600ns_.calo_zoning_word[0];
  std::bitset<calo_trigger_algorithm::ZONING_PER_SIDE_BITSET_SIZE> zoning_word_side_1 =
      a_coinc_calo_record_1600ns_.calo_zoning_word[1];
  // std::bitset<calo_trigger_algorithm::XT_INFO_BITSET_SIZE> xt_info =
  // a_coinc_calo_record_1600ns_.xt_info_bitset ; // Unused parameter
  bool single_side = a_coinc_calo_record_1600ns_.single_side_coinc;
  bool total_multiplicity_threshold = a_coinc_calo_record_1600ns_.total_multiplicity_threshold;
  bool calo_decision = a_coinc_calo_record_1600ns_.decision;

  uint32_t clocktick_1600_from_25 = 0;
  _compute_clocktick_1600ns(a_calo_summary_record_25ns_.clocktick_25ns, clocktick_1600_from_25);

  if (clocktick_1600_from_25 <=
      a_coinc_calo_record_1600ns_.clocktick_1600ns + _coincidence_calorimeter_gate_size_) {
    a_coinc_calo_record_1600ns_.calo_zoning_word[0] = a_calo_summary_record_25ns_.zoning_word[0];
    a_coinc_calo_record_1600ns_.calo_zoning_word[1] = a_calo_summary_record_25ns_.zoning_word[1];

    for (unsigned int i = 0; i < calo_trigger_algorithm::ZONING_PER_SIDE_BITSET_SIZE; i++) {
      if (a_coinc_calo_record_1600ns_.calo_zoning_word[0].test(i) == true) {
        zoning_word_side_0.set(i, true);
      }
      if (a_coinc_calo_record_1600ns_.calo_zoning_word[1].test(i) == true) {
        zoning_word_side_1.set(i, true);
      }
    }
    a_coinc_calo_record_1600ns_.calo_zoning_word[0] = zoning_word_side_0;
    a_coinc_calo_record_1600ns_.calo_zoning_word[1] = zoning_word_side_1;

    unsigned int number_of_zone_touch_side_0 =
        a_coinc_calo_record_1600ns_.calo_zoning_word[0].count();
    unsigned int number_of_zone_touch_side_1 =
        a_coinc_calo_record_1600ns_.calo_zoning_word[1].count();

    if (multiplicity_side_0 < a_calo_summary_record_25ns_.total_multiplicity_side_0.to_ulong()) {
      a_coinc_calo_record_1600ns_.total_multiplicity_side_0 =
          a_calo_summary_record_25ns_.total_multiplicity_side_0.to_ulong();
    }
    if (number_of_zone_touch_side_0 > multiplicity_side_0) {
      a_coinc_calo_record_1600ns_.total_multiplicity_side_0 = number_of_zone_touch_side_0;
    }

    if (multiplicity_side_1 < a_calo_summary_record_25ns_.total_multiplicity_side_1.to_ulong()) {
      a_coinc_calo_record_1600ns_.total_multiplicity_side_1 =
          a_calo_summary_record_25ns_.total_multiplicity_side_1.to_ulong();
    }
    if (number_of_zone_touch_side_1 > multiplicity_side_1) {
      a_coinc_calo_record_1600ns_.total_multiplicity_side_1 = number_of_zone_touch_side_1;
    }

    if (multiplicity_gveto < a_calo_summary_record_25ns_.total_multiplicity_gveto.to_ulong()) {
      a_coinc_calo_record_1600ns_.total_multiplicity_gveto =
          a_calo_summary_record_25ns_.total_multiplicity_gveto.to_ulong();
    }

    a_coinc_calo_record_1600ns_.LTO_side_0 =
        a_coinc_calo_record_1600ns_.LTO_side_0 + a_calo_summary_record_25ns_.LTO_side_0;
    a_coinc_calo_record_1600ns_.LTO_side_1 =
        a_coinc_calo_record_1600ns_.LTO_side_1 + a_calo_summary_record_25ns_.LTO_side_1;
    a_coinc_calo_record_1600ns_.LTO_gveto =
        a_coinc_calo_record_1600ns_.LTO_gveto + a_calo_summary_record_25ns_.LTO_gveto;

    if (a_coinc_calo_record_1600ns_.LTO_side_0 || lto_side_0)
      a_coinc_calo_record_1600ns_.LTO_side_0 = true;
    if (a_coinc_calo_record_1600ns_.LTO_side_1 || lto_side_1)
      a_coinc_calo_record_1600ns_.LTO_side_1 = true;
    if (a_coinc_calo_record_1600ns_.LTO_gveto || lto_gveto)
      a_coinc_calo_record_1600ns_.LTO_gveto = true;

    a_coinc_calo_record_1600ns_.xt_info_bitset = a_calo_summary_record_25ns_.xt_info_bitset;
    a_coinc_calo_record_1600ns_.single_side_coinc = a_calo_summary_record_25ns_.single_side_coinc;

    if (!a_calo_summary_record_25ns_.single_side_coinc || !single_side) {
      a_coinc_calo_record_1600ns_.single_side_coinc = false;
    } else if (a_coinc_calo_record_1600ns_.calo_zoning_word[0].any() &&
               a_coinc_calo_record_1600ns_.calo_zoning_word[1].any())
      a_coinc_calo_record_1600ns_.single_side_coinc = false;
    else
      a_coinc_calo_record_1600ns_.single_side_coinc = true;

    a_coinc_calo_record_1600ns_.total_multiplicity_threshold =
        a_calo_summary_record_25ns_.total_multiplicity_threshold;
    if (a_coinc_calo_record_1600ns_.total_multiplicity_threshold || total_multiplicity_threshold)
      a_coinc_calo_record_1600ns_.total_multiplicity_threshold = true;
    a_coinc_calo_record_1600ns_.decision = a_calo_summary_record_25ns_.calo_finale_decision;
    if (a_coinc_calo_record_1600ns_.decision || calo_decision)
      a_coinc_calo_record_1600ns_.decision = true;
  }

  return;
}

void coincidence_trigger_algorithm::_compute_clocktick_1600ns(const uint32_t clocktick_25ns_,
                                                              uint32_t& clocktick_1600ns_) {
  clocktick_1600ns_ =
      (clocktick_25ns_ * clock_utils::MAIN_CLOCKTICK) / clock_utils::TRIGGER_CLOCKTICK;
  clocktick_1600ns_ = clocktick_1600ns_ + clock_utils::TRIGGER_COMPUTING_SHIFT_CLOCKTICK_1600NS;
  return;
}

void coincidence_trigger_algorithm::_creating_pair_per_clocktick(
    const std::vector<coincidence_trigger_algorithm::coincidence_calo_record>& coinc_calo_records_,
    const std::vector<tracker_trigger_algorithm::tracker_record>& tracker_records_) {
  std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
            tracker_trigger_algorithm::tracker_record>
      pair_for_a_clocktick;

  if (coinc_calo_records_.size() != 0 || tracker_records_.size() != 0) {
    std::vector<coincidence_calo_record>::const_iterator it_calo = coinc_calo_records_.begin();
    for (; it_calo != _coincidence_calo_records_.end(); it_calo++) {
      coincidence_calo_record a_coinc_calo_record = *it_calo;
      pair_for_a_clocktick.first = a_coinc_calo_record;
      _pair_records_.push_back(pair_for_a_clocktick);
    }
    std::vector<tracker_trigger_algorithm::tracker_record>::const_iterator it_tracker =
        tracker_records_.begin();
    // Update of the pair if calo CT = tracker CT
    for (; it_tracker != tracker_records_.end(); it_tracker++) {
      tracker_trigger_algorithm::tracker_record a_tracker_record = *it_tracker;
      for (unsigned int i = 0; i < _pair_records_.size(); i++) {
        std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
                  tracker_trigger_algorithm::tracker_record>
            a_pair = _pair_records_[i];
        coincidence_calo_record a_coinc_calo_record = a_pair.first;
        if (a_tracker_record.clocktick_1600ns == a_coinc_calo_record.clocktick_1600ns) {
          a_pair.second = a_tracker_record;
          _pair_records_[i] = a_pair;
        } else {
          a_pair.second.clocktick_1600ns = a_coinc_calo_record.clocktick_1600ns;
        }
      }
    }
  }

  uint32_t last_clocktick = clock_utils::INVALID_CLOCKTICK;
  // Create a new pair if there is only tracker for example :
  if (_pair_records_.size() != 0) {
    std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
              tracker_trigger_algorithm::tracker_record>
        last_pair = _pair_records_.back();
    last_clocktick = last_pair.first.clocktick_1600ns;
  }

  std::vector<tracker_trigger_algorithm::tracker_record>::const_iterator it_tracker =
      tracker_records_.begin();
  for (; it_tracker != tracker_records_.end(); it_tracker++) {
    tracker_trigger_algorithm::tracker_record a_tracker_record = *it_tracker;
    if (last_clocktick == clock_utils::INVALID_CLOCKTICK ||
        a_tracker_record.clocktick_1600ns > last_clocktick) {
      std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
                tracker_trigger_algorithm::tracker_record>
          a_new_pair;
      a_new_pair.second = a_tracker_record;
      a_new_pair.first.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
      _pair_records_.push_back(a_new_pair);
    }
  }
  return;
}

void coincidence_trigger_algorithm::_process_calo_tracker_coincidence(
    const std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
                    tracker_trigger_algorithm::tracker_record>
        a_pair_for_a_clocktick_,
    coincidence_trigger_algorithm::coincidence_event_record& a_coincidence_record_)

{
  coincidence_trigger_algorithm::coincidence_calo_record a_calo_record =
      a_pair_for_a_clocktick_.first;
  tracker_trigger_algorithm::tracker_record a_tracker_record = a_pair_for_a_clocktick_.second;

  a_coincidence_record_.clocktick_1600ns = a_calo_record.clocktick_1600ns;

  for (unsigned int iside = 0; iside < mapping::NUMBER_OF_SIDES; iside++) {
    for (unsigned int izone = 0; izone < mapping::NUMBER_OF_TRIGGER_ZONES; izone++) {
      std::bitset<3> hpattern_for_a_zone = 0x0;

      int right = 0;
      int mid = 1;
      int left = 2;

      hpattern_for_a_zone[right] = a_tracker_record.finale_data_per_zone[iside][izone][right + 2];
      hpattern_for_a_zone[mid] = a_tracker_record.finale_data_per_zone[iside][izone][mid + 2];
      hpattern_for_a_zone[left] = a_tracker_record.finale_data_per_zone[iside][izone][left + 2];
      if (hpattern_for_a_zone.any()) {
        if (hpattern_for_a_zone.test(mid) &&
            a_calo_record.calo_zoning_word[iside].test(izone) != 0) {
          a_coincidence_record_.zoning_word[iside].set(izone, true);
          a_coincidence_record_.decision = true;
          _caraco_decision_ = true;
        }

        if ((hpattern_for_a_zone.test(right) &&
             a_calo_record.calo_zoning_word[iside].test(izone) != 0) ||
            (izone + 1 < 10 && hpattern_for_a_zone.test(right) &&
             a_calo_record.calo_zoning_word[iside].test(izone + 1) != 0)) {
          a_coincidence_record_.zoning_word[iside].set(izone, true);
          a_coincidence_record_.decision = true;
          _caraco_decision_ = true;
        }

        if ((hpattern_for_a_zone.test(left) &&
             a_calo_record.calo_zoning_word[iside].test(izone) != 0) ||
            (int(izone - 1) > -1 && hpattern_for_a_zone.test(left) &&
             a_calo_record.calo_zoning_word[iside].test(izone - 1) != 0)) {
          a_coincidence_record_.zoning_word[iside].set(izone, true);
          a_coincidence_record_.decision = true;
          _caraco_decision_ = true;
        }

      }  // end of hpattern.any()
    }    // end of izone
  }      // end of iside

  if (a_coincidence_record_.decision) {
    a_coincidence_record_.calo_zoning_word[0] = a_calo_record.calo_zoning_word[0];
    a_coincidence_record_.calo_zoning_word[1] = a_calo_record.calo_zoning_word[1];
    a_coincidence_record_.total_multiplicity_side_0 = a_calo_record.total_multiplicity_side_0;
    a_coincidence_record_.total_multiplicity_side_1 = a_calo_record.total_multiplicity_side_1;
    a_coincidence_record_.LTO_side_0 = a_calo_record.LTO_side_0;
    a_coincidence_record_.LTO_side_1 = a_calo_record.LTO_side_1;
    a_coincidence_record_.total_multiplicity_gveto = a_calo_record.total_multiplicity_gveto;
    a_coincidence_record_.LTO_gveto = a_calo_record.LTO_gveto;
    a_coincidence_record_.xt_info_bitset = a_calo_record.xt_info_bitset;
    a_coincidence_record_.single_side_coinc = a_calo_record.single_side_coinc;
    a_coincidence_record_.total_multiplicity_threshold = a_calo_record.total_multiplicity_threshold;

    for (unsigned int iside = 0; iside < mapping::NUMBER_OF_SIDES; iside++) {
      for (unsigned int izone = 0; izone < mapping::NUMBER_OF_TRIGGER_ZONES; izone++) {
        a_coincidence_record_.tracker_finale_data_per_zone[iside][izone] =
            a_tracker_record.finale_data_per_zone[iside][izone];
      }  // end of izone
    }    // end of iside
    a_coincidence_record_.trigger_mode = CARACO;

    bool decision_already_true_in_last_CTs = false;
    for (unsigned int i = 0; i < _L2_coincidence_decison_records_.size(); i++) {
      coincidence_trigger_algorithm::L2_coincidence_decision already_created_L2_coinc_decision =
          _L2_coincidence_decison_records_[i];
      uint32_t clocktick_maximum_for_decision =
          already_created_L2_coinc_decision.L2_clocktick_decision +
          SIZE_OF_L2_COINCIDENCE_DECISION_GATE;
      if (a_coincidence_record_.clocktick_1600ns < clocktick_maximum_for_decision &&
          already_created_L2_coinc_decision.L2_clocktick_decision <
              a_coincidence_record_.clocktick_1600ns) {
        decision_already_true_in_last_CTs = true;
      }
    }  // end of icoinc

    if (decision_already_true_in_last_CTs == false) {
      coincidence_trigger_algorithm::L2_coincidence_decision a_L2_coinc_decision;
      a_L2_coinc_decision.L2_coincidence_decision_bool = true;
      a_L2_coinc_decision.L2_clocktick_decision = a_coincidence_record_.clocktick_1600ns;
      a_L2_coinc_decision.trigger_mode = CARACO;
      _L2_coincidence_decison_records_.push_back(a_L2_coinc_decision);
    }
  }

  return;
}

void coincidence_trigger_algorithm::_build_previous_event_record(
    coincidence_trigger_algorithm::coincidence_event_record& a_coincidence_record_) {
  coincidence_trigger_algorithm::previous_event_record a_previous_event_record;
  unsigned int max_mult_side_0 = a_previous_event_record.total_multiplicity_side_0.to_ulong();
  unsigned int max_mult_side_1 = a_previous_event_record.total_multiplicity_side_1.to_ulong();
  unsigned int max_mult_gveto = a_previous_event_record.total_multiplicity_gveto.to_ulong();

  a_previous_event_record.previous_clocktick_1600ns = a_coincidence_record_.clocktick_1600ns;
  a_previous_event_record.counter_1600ns =
      clock_utils::PREVIOUS_EVENT_RECORD_LIVING_NUMBER_OF_CLOCKTICK;  // Equal to 625 CT 1600
                                                                      // because 625 * 1600 ns = 1
                                                                      // ms

  if (a_coincidence_record_.total_multiplicity_side_0.to_ulong() > max_mult_side_0)
    a_previous_event_record.total_multiplicity_side_0 =
        a_coincidence_record_.total_multiplicity_side_0;
  if (a_coincidence_record_.total_multiplicity_side_1.to_ulong() > max_mult_side_1)
    a_previous_event_record.total_multiplicity_side_1 =
        a_coincidence_record_.total_multiplicity_side_1;

  if (a_coincidence_record_.LTO_side_0) a_previous_event_record.LTO_side_0 = true;
  if (a_coincidence_record_.LTO_side_1) a_previous_event_record.LTO_side_1 = true;

  if (a_coincidence_record_.total_multiplicity_gveto.to_ulong() > max_mult_gveto)
    a_previous_event_record.total_multiplicity_gveto =
        a_coincidence_record_.total_multiplicity_gveto;

  if (a_coincidence_record_.LTO_gveto) a_previous_event_record.LTO_gveto = true;

  for (unsigned int ibit = 0; ibit < calo_trigger_algorithm::XT_INFO_BITSET_SIZE; ibit++) {
    if (a_coincidence_record_.xt_info_bitset.test(ibit))
      a_previous_event_record.xt_info_bitset.set(ibit);
  }
  if (a_coincidence_record_.single_side_coinc) a_previous_event_record.single_side_coinc = true;
  if (a_coincidence_record_.total_multiplicity_threshold)
    a_previous_event_record.total_multiplicity_threshold = true;

  for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
    for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
      if (a_coincidence_record_.zoning_word[iside].test(izone))
        a_previous_event_record.zoning_word[iside].set(izone, true);
      if (a_coincidence_record_.calo_zoning_word[iside].test(izone))
        a_previous_event_record.calo_zoning_word[iside].set(izone, true);
      for (unsigned int ibit = 0; ibit < trigger_info::DATA_FULL_BITSET_SIZE; ibit++) {
        if (a_coincidence_record_.tracker_finale_data_per_zone[iside][izone].test(ibit))
          a_previous_event_record.tracker_finale_data_per_zone[iside][izone].set(ibit);
      }
    }
  }
  _previous_event_records_->push_back(a_previous_event_record);

  return;
}

void coincidence_trigger_algorithm::_process_delayed_coincidence(
    const std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
                    tracker_trigger_algorithm::tracker_record>
        a_pair_for_a_clocktick_,
    const coincidence_trigger_algorithm::previous_event_record& a_previous_event_record_,
    coincidence_trigger_algorithm::coincidence_event_record& a_delayed_record_) {
  if (a_previous_event_record_.counter_1600ns != 0)  // counter != 0 ->   > 1ms (625 * 1600ns)
  {
    tracker_trigger_algorithm::tracker_record a_tracker_record = a_pair_for_a_clocktick_.second;

    for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
      for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
        std::bitset<3> delayed_hpattern_per_zone = 0x0;

        int right = 0;
        int mid = 1;
        int left = 2;

        delayed_hpattern_per_zone[right] =
            a_tracker_record.finale_data_per_zone[iside][izone][right + 2];  // right + 2 -> bit
                                                                             // position to change
                                                                             // (bit enum in
                                                                             // trigger_info.h)
        delayed_hpattern_per_zone[mid] =
            a_tracker_record.finale_data_per_zone[iside][izone][mid + 2];
        delayed_hpattern_per_zone[left] =
            a_tracker_record.finale_data_per_zone[iside][izone][left + 2];

        // APE trigger (Tracker previous / Tracker delayed coincidence)
        if (delayed_hpattern_per_zone.any()) {
          if (delayed_hpattern_per_zone.test(left) && izone == 0 &&
              (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(mid + 2))) {
            a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
            a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                a_tracker_record.finale_data_per_zone[iside][izone];
            a_delayed_record_.trigger_mode = APE;
            a_delayed_record_.decision = true;
            _delayed_coincidence_decision_ = true;
          }

          if (delayed_hpattern_per_zone.test(left) && int(izone - 1) > -1 &&
              (a_previous_event_record_.tracker_finale_data_per_zone[0][izone - 1].test(right +
                                                                                        2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone - 1].test(right +
                                                                                        2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(mid + 2))) {
            a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
            a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                a_tracker_record.finale_data_per_zone[iside][izone];
            a_delayed_record_.trigger_mode = APE;
            a_delayed_record_.decision = true;
            _delayed_coincidence_decision_ = true;
          }

          if (delayed_hpattern_per_zone.test(mid) &&
              (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(right + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(right + 2))) {
            a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
            a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                a_tracker_record.finale_data_per_zone[iside][izone];
            a_delayed_record_.trigger_mode = APE;
            a_delayed_record_.decision = true;
            _delayed_coincidence_decision_ = true;
          }

          if (delayed_hpattern_per_zone.test(right) && izone == 9 &&
              (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(right + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(right + 2))) {
            a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
            a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                a_tracker_record.finale_data_per_zone[iside][izone];
            a_delayed_record_.trigger_mode = APE;
            a_delayed_record_.decision = true;
            _delayed_coincidence_decision_ = true;
          }

          if (delayed_hpattern_per_zone.test(right) && izone + 1 < 10 &&
              (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(right + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[0][izone + 1].test(left + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(mid + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(right + 2) ||
               a_previous_event_record_.tracker_finale_data_per_zone[1][izone + 1].test(left +
                                                                                        2))) {
            a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
            a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                a_tracker_record.finale_data_per_zone[iside][izone];
            a_delayed_record_.trigger_mode = APE;
            a_delayed_record_.decision = true;
            _delayed_coincidence_decision_ = true;
          }
        }
      }  // end of izone
    }    // end of iside

    if (a_delayed_record_.decision) {
      bool decision_already_true_in_last_CTs = false;
      for (unsigned int i = 0; i < _L2_coincidence_decison_records_.size(); i++) {
        coincidence_trigger_algorithm::L2_coincidence_decision already_created_L2_coinc_decision =
            _L2_coincidence_decison_records_[i];
        uint32_t clocktick_maximum_for_decision =
            already_created_L2_coinc_decision.L2_clocktick_decision +
            SIZE_OF_L2_COINCIDENCE_DECISION_GATE;
        if (a_delayed_record_.clocktick_1600ns < clocktick_maximum_for_decision &&
            already_created_L2_coinc_decision.L2_clocktick_decision <
                a_delayed_record_.clocktick_1600ns) {
          decision_already_true_in_last_CTs = true;
        }
      }  // end of icoinc

      if (decision_already_true_in_last_CTs == false) {
        coincidence_trigger_algorithm::L2_coincidence_decision a_L2_coinc_decision;
        a_L2_coinc_decision.L2_coincidence_decision_bool = true;
        a_L2_coinc_decision.L2_clocktick_decision = a_delayed_record_.clocktick_1600ns;
        a_L2_coinc_decision.trigger_mode = APE;
        _L2_coincidence_decison_records_.push_back(a_L2_coinc_decision);
      }
    }

    // Delayed Alpha Veto Event (DAVE) trigger
    else {
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++) {
        for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++) {
          std::bitset<2> delayed_near_source_per_zone = 0x0;
          int near_source_right = 0;
          int near_source_left = 1;
          delayed_near_source_per_zone[near_source_right] =
              a_tracker_record.finale_data_per_zone[iside][izone][near_source_right + 5];
          delayed_near_source_per_zone[near_source_left] =
              a_tracker_record.finale_data_per_zone[iside][izone][near_source_left + 5];

          if (delayed_near_source_per_zone.any()) {
            // NSZ Left && zone = 0 :
            if (delayed_near_source_per_zone.test(near_source_left) && izone == 0 &&
                (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_right + 5))) {
              a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
              a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                  a_tracker_record.finale_data_per_zone[iside][izone];
              a_delayed_record_.trigger_mode = DAVE;
              a_delayed_record_.decision = true;
              _delayed_coincidence_decision_ = true;
            }

            // NSZ Left && zone = [1:9] :
            if (delayed_near_source_per_zone.test(near_source_left) && int(izone - 1) > -1 &&
                (a_previous_event_record_.tracker_finale_data_per_zone[0][izone - 1].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone - 1].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_right + 5)))

            {
              a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
              a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                  a_tracker_record.finale_data_per_zone[iside][izone];
              a_delayed_record_.trigger_mode = DAVE;
              a_delayed_record_.decision = true;
              _delayed_coincidence_decision_ = true;
            }

            // NSZ Right && zone = 0 :
            if (delayed_near_source_per_zone.test(near_source_right) && izone == 9 &&
                (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_right + 5))) {
              a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
              a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                  a_tracker_record.finale_data_per_zone[iside][izone];
              a_delayed_record_.trigger_mode = DAVE;
              a_delayed_record_.decision = true;
              _delayed_coincidence_decision_ = true;
            }

            // NSZ Right && zone = [1:8] :
            if (delayed_near_source_per_zone.test(near_source_right) && izone + 1 < 10 &&
                (a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[0][izone + 1].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_left + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone].test(
                     near_source_right + 5) ||
                 a_previous_event_record_.tracker_finale_data_per_zone[1][izone + 1].test(
                     near_source_left + 5)))

            {
              a_delayed_record_.clocktick_1600ns = a_tracker_record.clocktick_1600ns;
              a_delayed_record_.tracker_finale_data_per_zone[iside][izone] =
                  a_tracker_record.finale_data_per_zone[iside][izone];
              a_delayed_record_.trigger_mode = DAVE;
              a_delayed_record_.decision = true;
              _delayed_coincidence_decision_ = true;
            }

          }  // enf of if delayed any

        }  // end of izone
      }    // end of iside

      if (a_delayed_record_.decision) {
        bool decision_already_true_in_last_CTs = false;
        for (unsigned int i = 0; i < _L2_coincidence_decison_records_.size(); i++) {
          coincidence_trigger_algorithm::L2_coincidence_decision already_created_L2_coinc_decision =
              _L2_coincidence_decison_records_[i];
          uint32_t clocktick_maximum_for_decision =
              already_created_L2_coinc_decision.L2_clocktick_decision +
              SIZE_OF_L2_COINCIDENCE_DECISION_GATE;
          if (a_delayed_record_.clocktick_1600ns < clocktick_maximum_for_decision &&
              already_created_L2_coinc_decision.L2_clocktick_decision <
                  a_delayed_record_.clocktick_1600ns) {
            decision_already_true_in_last_CTs = true;
          }
        }  // end of icoinc

        if (decision_already_true_in_last_CTs == false) {
          coincidence_trigger_algorithm::L2_coincidence_decision a_L2_coinc_decision;
          a_L2_coinc_decision.L2_coincidence_decision_bool = true;
          a_L2_coinc_decision.L2_clocktick_decision = a_delayed_record_.clocktick_1600ns;
          a_L2_coinc_decision.trigger_mode = DAVE;
          _L2_coincidence_decison_records_.push_back(a_L2_coinc_decision);
        }
      }

    }  // end of else

  }  // end of if counter != 0

  return;
}

void coincidence_trigger_algorithm::process(
    const std::vector<calo_trigger_algorithm::calo_summary_record>& calo_records_,
    const std::vector<tracker_trigger_algorithm::tracker_record>& tracker_records_,
    std::vector<coincidence_trigger_algorithm::coincidence_event_record>& coincidence_records_) {
  DT_THROW_IF(!is_initialized(), std::logic_error,
              "Coincidence trigger algorithm is not initialized, it can't process ! ");
  _process(calo_records_, tracker_records_, coincidence_records_);
  return;
}

void coincidence_trigger_algorithm::_process(
    const std::vector<calo_trigger_algorithm::calo_summary_record>& calo_records_,
    const std::vector<tracker_trigger_algorithm::tracker_record>& tracker_records_,
    std::vector<coincidence_trigger_algorithm::coincidence_event_record>& coincidence_records_) {
  reset_data();
  _previous_event_records_.reset(
      new buffer_previous_event_record_type(_previous_event_circular_buffer_depth_));
  _preparing_calo_coincidence(calo_records_);
  _creating_pair_per_clocktick(_coincidence_calo_records_, tracker_records_);

  std::vector<std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
                        tracker_trigger_algorithm::tracker_record> >::iterator it_pair =
      _pair_records_.begin();
  for (; it_pair != _pair_records_.end(); it_pair++) {
    std::pair<coincidence_trigger_algorithm::coincidence_calo_record,
              tracker_trigger_algorithm::tracker_record>
        a_pair = *it_pair;
    uint32_t pair_clocktick_1600ns = a_pair.first.clocktick_1600ns;

    coincidence_event_record a_coincidence_record;
    _process_calo_tracker_coincidence(a_pair, a_coincidence_record);

    if (a_coincidence_record.decision) {
      // Add the coincidence in the records :

      coincidence_records_.push_back(a_coincidence_record);
      coincidence_trigger_algorithm::L2_coincidence_decision last_L2_decision =
          _L2_coincidence_decison_records_.front();
      // Search the last L2 decision (in CT)
      for (unsigned int i = 0; i < _L2_coincidence_decison_records_.size(); i++) {
        coincidence_trigger_algorithm::L2_coincidence_decision a_L2_decision =
            _L2_coincidence_decison_records_[i];
        if (a_L2_decision.L2_clocktick_decision > last_L2_decision.L2_clocktick_decision)
          last_L2_decision = a_L2_decision;
      }

      // For a CARACO L2 decision, create a PER at the end of the L2 decision
      if (last_L2_decision.L2_coincidence_decision_bool &&
          pair_clocktick_1600ns ==
              last_L2_decision.L2_clocktick_decision + SIZE_OF_L2_COINCIDENCE_DECISION_GATE - 1 &&
          last_L2_decision.trigger_mode == CARACO) {
        _build_previous_event_record(a_coincidence_record);
      }
    } else {
      if (_previous_event_records_->size() != 0) {
        boost::circular_buffer<coincidence_trigger_algorithm::previous_event_record>::iterator
            per_it = _previous_event_records_->begin();

        for (; per_it != _previous_event_records_->end(); per_it++) {
          coincidence_trigger_algorithm::previous_event_record a_previous_event_record = *per_it;
          if (pair_clocktick_1600ns < a_previous_event_record.previous_clocktick_1600ns) {
          } else {
            a_previous_event_record.counter_1600ns =
                clock_utils::PREVIOUS_EVENT_RECORD_LIVING_NUMBER_OF_CLOCKTICK -
                (a_pair.first.clocktick_1600ns - a_previous_event_record.previous_clocktick_1600ns);

            if (a_previous_event_record.counter_1600ns > 0) {
              coincidence_event_record a_delayed_event_record;
              // Compare tracker record only with previous event
              _process_delayed_coincidence(a_pair, a_previous_event_record, a_delayed_event_record);

              if (a_delayed_event_record.decision) {
                coincidence_records_.push_back(a_delayed_event_record);
              }
            }

            *per_it = a_previous_event_record;
          }
        }
      }
    }  // end of else
  }    // end of it_pair

  return;
}

}  // end of namespace digitization

}  // end of namespace snemo
