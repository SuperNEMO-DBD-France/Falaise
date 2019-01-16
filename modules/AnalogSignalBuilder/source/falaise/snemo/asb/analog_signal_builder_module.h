// snemo/asb/analog_signal_builder_module.h
// Author(s): F. Mauger <mauger@lpccaen.in2p3.fr>
// Author(s): G. Oliviéro <goliviero@lpccaen.in2p3.fr>
// Date: 2016-11-01

#ifndef FALAISE_ASB_PLUGIN_SNEMO_ASB_ANALOG_SIGNAL_BUILDER_MODULE_H
#define FALAISE_ASB_PLUGIN_SNEMO_ASB_ANALOG_SIGNAL_BUILDER_MODULE_H

// Standard library:
#include <string>

// Third party:
// - Bayeux/datatools:
#include <bayeux/datatools/properties.h>
// - Bayeux/dpp :
#include <bayeux/dpp/base_module.h>
// - Bayeux/mctools :
#include <bayeux/mctools/signal/signal_data.h>
#include <bayeux/mctools/simulated_data.h>

// This project:
#include <falaise/snemo/asb/base_signal_generator_driver.h>

namespace snemo {

namespace asb {

class base_signal_generator_driver;

/// \brief The data processing module for building simulated signal hits
class analog_signal_builder_module : public dpp::base_module {
 public:
  struct driver_entry;
  typedef std::map<std::string, driver_entry> driver_dict_type;

  /// Constructor
  analog_signal_builder_module(datatools::logger::priority = datatools::logger::PRIO_FATAL);

  /// Destructor
  virtual ~analog_signal_builder_module();

  /// Initialization
  ///
  /// Example of configuration:
  /// \code
  /// SD_label  : string = "SD"
  /// SSD_label : string = "SSD"
  /// Geo_label : string = "Geo"
  /// abort_at_missing_input : boolean = true
  /// abort_at_former_output : boolean = false
  /// preserve_former_output : boolean = false
  ///
  /// drivers : string[4] = "calo" "xcalo" "gveto" "gg"
  ///
  /// driver.calo.type_id : string = "snemo::asb::calo_signal_builder"
  /// driver.calo.config.gain : real = 1.2e5
  /// driver.calo.config.db_access : boolean = true
  ///
  /// driver.xcalo.type_id : string = "snemo::asb::calo_signal_builder"
  /// driver.xcalo.config.gain : real = 0.95e5
  /// driver.xcalo.config.db_access : boolean = true
  ///
  /// driver.gveto.type_id : string = "snemo::asb::calo_signal_builder"
  /// driver.gveto.config.gain : real = 0.93e5
  /// driver.gveto.config.db_access : boolean = true
  ///
  /// driver.gg.type_id : string = "snemo::asb::tracker_signal_builder"
  /// driver.gg.config.gain : real = 0.93e5
  /// driver.gg.config.db_access : boolean = true
  ///
  /// \endcode
  ///
  ///
  virtual void initialize(const datatools::properties &setup_,
                          datatools::service_manager &service_manager_,
                          dpp::module_handle_dict_type &module_dict_);

  /// Reset
  virtual void reset();

  /// Data record processing
  virtual process_status process(datatools::things &data_);

  /// Set the 'simulated data' bank label
  void set_sd_label(const std::string &);

  /// Return the 'simulated data' bank label
  const std::string &get_sd_label() const;

  /// Set the 'simulated signal data' bank label
  void set_ssd_label(const std::string &);

  /// Return the 'simulated signal data' bank label
  const std::string &get_ssd_label() const;

  /// Set the 'geometry service' label
  void set_geo_label(const std::string &);

  /// Return the 'geometry service' label
  const std::string &get_geo_label() const;

  /// Set the 'database service' label
  void set_db_label(const std::string &);

  /// Return the 'database service' label
  const std::string &get_db_label() const;

  /// Check database manager
  bool has_database_manager() const;

  // /// Setting database manager
  // void set_database_manager(const database::manager & gmgr_);

  // /// Getting database manager
  // const database::manager & get_database_manager() const;

  /// Check geometry manager
  bool has_geometry_manager() const;

  /// Setting geometry manager
  void set_geometry_manager(const geomtools::manager &gmgr_);

  /// Getting geometry manager
  const geomtools::manager &get_geometry_manager() const;

  bool is_abort_at_missing_input() const;
  void set_abort_at_missing_input(bool);
  bool is_abort_at_former_output() const;
  void set_abort_at_former_output(bool);
  bool is_preserve_former_output() const;
  void set_preserve_former_output(bool);

  /// Check if a driver with given name is set
  bool has_driver(const std::string &name_) const;

  /// Add a driver
  void add_driver(const std::string &name_, const std::string &type_id_,
                  const datatools::properties &config_);

  /// Remove a driver
  void remove_driver(const std::string &name_);

 private:
  void _init_drivers_(const datatools::properties &setup_,
                      datatools::service_manager &service_manager_);

  /// Main process function
  void _process_(const mctools::simulated_data &sim_data_,
                 mctools::signal::signal_data &analog_signal_builder_data_);

  /// Give default values to specific class members.
  void _set_defaults_();

 private:
  // Configuration:
  std::string _SD_label_;   //!< The label of the input simulated data bank
  std::string _SSD_label_;  //!< The label of the output simulated signal data bank
  std::string _Geo_label_;  //!< The label of the geometry service
  std::string _Db_label_;   //!< The label of the database service
  bool _abort_at_missing_input_ = true;
  bool _abort_at_former_output_ = false;
  bool _preserve_former_output_ = false;

  // Working data:
  const geomtools::manager *_geometry_manager_ = nullptr;  //!< The geometry manager
  // const snemo::XXX::manager * _database_manager_ = nullptr; //!< The database manager
  driver_dict_type _drivers_;  //!< Dictionary of drivers (embedded generator of signal hits)

  // Macro to automate the registration of the module :
  DPP_MODULE_REGISTRATION_INTERFACE(analog_signal_builder_module)
};

}  // end of namespace asb

}  // end of namespace snemo

#include <datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::asb::analog_signal_builder_module)

#endif  // FALAISE_ASB_PLUGIN_SNEMO_ASB_ANALOG_SIGNAL_BUILDER_MODULE_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
