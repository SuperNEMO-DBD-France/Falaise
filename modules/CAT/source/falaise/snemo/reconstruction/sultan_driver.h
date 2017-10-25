/** \file falaise/snemo/reconstruction/sultan_driver.h
 * Author(s)     : Federico Nova <nova@physics.utexas.edu>
 * Creation date : 2013-10-29
 * Last modified : 2014-02-22
 *
 * Copyright (C) 2013-2014 Federico Nova <nova@physics.utexas.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Description:
 *
 *   A driver class that wraps the Channel/sultan tracker clustering algorithm.
 *
 * History:
 *
 */

#ifndef FALAISE_CAT_PLUGIN_SNEMO_RECONSTRUCTION_SULTAN_DRIVER_H
#define FALAISE_CAT_PLUGIN_SNEMO_RECONSTRUCTION_SULTAN_DRIVER_H 1

// Standard library:
#include <string>

// This project
#include <falaise/snemo/processing/base_tracker_clusterizer.h>
#include <sultan/SULTAN_interface.h>

// Forward declaration :
namespace datatools {
class properties;
}

namespace snemo {

namespace geometry {
class calo_locator;
class xcalo_locator;
class gveto_locator;
}  // namespace geometry

namespace reconstruction {

/// Driver for the Channel/sultan clustering algorithms
class sultan_driver : public ::snemo::processing::base_tracker_clusterizer {
 public:
  static const std::string SULTAN_ID;  /// The SULTAN string identifier

  /// Set the mag field
  void set_magfield(double);

  /// Return the mag field
  double get_magfield() const;

  /// Set the mag field direction
  void set_magfield_direction(double);

  /// Return the mag field
  double get_magfield_direction() const;

  /// Default constructor
  sultan_driver();

  /// Destructor
  virtual ~sultan_driver();

  /// Initialize the clusterizer through configuration properties
  virtual void initialize(const datatools::properties& setup_);

  /// Reset the clusterizer
  virtual void reset();

  /// OCD support:
  static void init_ocd(datatools::object_configuration_description& ocd_);

 protected:
  /// Set default attributes
  void _set_defaults();

  /// Main clustering method
  virtual int _process_algo(const base_tracker_clusterizer::hit_collection_type& gg_hits_,
                            const base_tracker_clusterizer::calo_hit_collection_type& calo_hits_,
                            snemo::datamodel::tracker_clustering_data& clustering_);

 private:
  SULTAN::setup_data _SULTAN_setup_;         /// Configuration data
  SULTAN::input_data _SULTAN_input_;         /// Input data
  SULTAN::output_data _SULTAN_output_;       /// Output data
  SULTAN::clusterizer _SULTAN_clusterizer_;  /// SULTAN clusterizer
  SULTAN::sultan _SULTAN_sultan_;            /// SULTAN algorithm
  double _sigma_z_factor_;                   /// Factor for longitudinal error
  double _magfield_;                         /// Enforced magnetic field
  double _magfield_dir_;     ///< Enforced magnetic field (direction along the Z axis +1/-1)
  bool _process_calo_hits_;  /// Flag to process associated calorimeter hits

  /// Calorimeter locators
  const snemo::geometry::calo_locator* _calo_locator_;
  const snemo::geometry::xcalo_locator* _xcalo_locator_;
  const snemo::geometry::gveto_locator* _gveto_locator_;
};

}  // end of namespace reconstruction

}  // end of namespace snemo

#include <datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::reconstruction::sultan_driver)

#endif  // FALAISE_CAT_PLUGIN_SNEMO_RECONSTRUCTION_SULTAN_DRIVER_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
