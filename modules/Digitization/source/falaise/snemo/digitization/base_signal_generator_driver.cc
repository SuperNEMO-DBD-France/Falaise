/// \file falaise/snemo/digitization/base_signal_generator_driver.cc

// Ourselves:
#include <falaise/snemo/digitization/base_signal_generator_driver.h>

// Standard library:
#include <sstream>
#include <stdexcept>

// Third party:
// - Bayeux/datatools :
#include <bayeux/datatools/exception.h>
#include <bayeux/datatools/properties.h>
// // - Bayeux/geomtools :
// #include <bayeux/geomtools/manager.h>

// This project:

namespace snemo {

namespace digitization {

DATATOOLS_FACTORY_SYSTEM_REGISTER_IMPLEMENTATION(
    base_signal_generator_driver, "snemo::digitization::base_signal_generator_driver/__system__")

void base_signal_generator_driver::_set_defaults() {
  _logging_priority = datatools::logger::PRIO_FATAL;
  return;
}

base_signal_generator_driver::base_signal_generator_driver(const std::string& id_) {
  _initialized_ = false;
  _set_defaults();
  set_id(id_);
  return;
}

base_signal_generator_driver::~base_signal_generator_driver() {
  // Should not be initialized here...
  DT_THROW_IF(is_initialized(), std::logic_error, "Missing explicit reset call in child class!");
  return;
}

datatools::logger::priority base_signal_generator_driver::get_logging_priority() const {
  return _logging_priority;
}

void base_signal_generator_driver::set_logging_priority(datatools::logger::priority priority_) {
  _logging_priority = priority_;
  return;
}

bool base_signal_generator_driver::has_id() const { return !_id_.empty(); }

const std::string& base_signal_generator_driver::get_id() const { return _id_; }

void base_signal_generator_driver::set_id(const std::string& id_) {
  _id_ = id_;
  return;
}

bool base_signal_generator_driver::has_signal_category() const {
  return !_signal_category_.empty();
}

const std::string& base_signal_generator_driver::get_signal_category() const {
  return _signal_category_;
}

void base_signal_generator_driver::set_signal_category(const std::string& category_) {
  _signal_category_ = category_;
  return;
}

bool base_signal_generator_driver::has_geo_manager() const { return _geo_manager_ != nullptr; }

void base_signal_generator_driver::set_geo_manager(const geomtools::manager& mgr_) {
  _geo_manager_ = &mgr_;
  return;
}

const geomtools::manager& base_signal_generator_driver::get_geo_manager() const {
  DT_THROW_IF(!has_geo_manager(), std::logic_error, "Missing geometry initialized !");
  return *_geo_manager_;
}

bool base_signal_generator_driver::is_initialized() const { return _initialized_; }

void base_signal_generator_driver::_set_initialized_(bool i_) {
  _initialized_ = i_;
  return;
}

void base_signal_generator_driver::initialize(const datatools::properties& config_) {
  DT_THROW_IF(is_initialized(), std::logic_error, "Already initialized !");

  // Logging priority:
  datatools::logger::priority lp = datatools::logger::extract_logging_configuration(config_);
  DT_THROW_IF(lp == datatools::logger::PRIO_UNDEFINED, std::logic_error,
              "Invalid logging priority level !");
  set_logging_priority(lp);

  if (_id_.empty()) {
    if (config_.has_key("id")) {
      set_id(config_.fetch_string("id"));
    }
  }

  if (_signal_category_.empty()) {
    if (config_.has_key("signal_category")) {
      set_signal_category(config_.fetch_string("signal_category"));
    }
  }

  DT_THROW_IF(_signal_category_.empty(), std::logic_error, "Missing signal category!");

  _initialize(config_);

  _set_initialized_(true);
  return;
}

void base_signal_generator_driver::reset() {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
  _set_initialized_(false);
  _reset();
  return;
}

void base_signal_generator_driver::process(const mctools::simulated_data& sim_data_,
                                           mctools::signal::signal_data& sim_signal_data_) {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
  _process(sim_data_, sim_signal_data_);
  return;
}

void base_signal_generator_driver::tree_dump(std::ostream& out_, const std::string& title_,
                                             const std::string& indent_, bool inherit_) const {
  if (!title_.empty()) {
    out_ << indent_ << title_ << std::endl;
  }

  out_ << indent_ << datatools::i_tree_dumpable::tag << "Logging : '"
       << datatools::logger::get_priority_label(_logging_priority) << "'" << std::endl;

  out_ << indent_ << datatools::i_tree_dumpable::tag << "Algorithm ID : '" << get_id() << "'"
       << std::endl;

  out_ << indent_ << datatools::i_tree_dumpable::tag << "Signal category ID : '"
       << get_signal_category() << "'" << std::endl;

  out_ << indent_ << datatools::i_tree_dumpable::tag << "Geometry manager : ";
  if (has_geo_manager()) {
    out_ << "<yes>";
  } else {
    out_ << "<no>";
  }
  out_ << std::endl;

  out_ << indent_ << datatools::i_tree_dumpable::inherit_tag(inherit_)
       << "Initialized : " << is_initialized() << std::endl;

  return;
}

}  // end of namespace digitization

}  // end of namespace snemo
