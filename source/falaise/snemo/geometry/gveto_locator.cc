// falaise/snemo/geometry/gveto_locator.cc
/* Copyright (C) 2011-2014 Francois Mauger <mauger@lpccaen.in2p3.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or(at
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
 *
 *
 */

// Ourselves:
#include <falaise/snemo/geometry/gveto_locator.h>

// Standard library:
#include <stdexcept>

// Third party
// - Bayeux/datatools :
#include <datatools/utils.h>
#include <datatools/version_id.h>
// - Bayeux/geomtools :
#include <geomtools/box.h>
#include <geomtools/i_composite_shape_3d.h>
#include <geomtools/intersection_3d.h>
#include <geomtools/manager.h>
#include <geomtools/subtraction_3d.h>

namespace snemo {

namespace geometry {

bool gveto_locator::is_initialized() const { return _initialized_; }

bool gveto_locator::has_submodule(uint32_t side_) const {
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Submodule number " << side_ << " makes no sense !");
  return _submodules_[side_];
}

double gveto_locator::get_block_width() const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return _block_width_;
}

double gveto_locator::get_block_height() const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return _block_height_;
}

double gveto_locator::get_block_thickness() const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return _block_thickness_;
}

size_t gveto_locator::get_number_of_sides() const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return utils::NSIDES;
}

size_t gveto_locator::get_number_of_walls() const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return NWALLS_PER_SIDE;
}

double gveto_locator::get_wall_z(uint32_t side_, uint32_t wall_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">" << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");
  return _block_z_[side_][wall_];
}

double gveto_locator::get_wall_window_z(uint32_t side_, uint32_t wall_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">" << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");
  return _block_window_z_[side_][wall_];
}

double gveto_locator::get_column_x(uint32_t side_, uint32_t wall_, uint32_t column_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">" << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");

  if (side_ == (uint32_t)utils::SIDE_BACK) {
    DT_THROW_IF(
        column_ >= _back_block_x_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _back_block_x_[wall_].size() - 1 << ")!");
    return _back_block_x_[wall_][column_];
  } else {
    DT_THROW_IF(
        column_ >= _front_block_x_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _front_block_x_[wall_].size() - 1 << ")!");
    return _front_block_x_[wall_][column_];
  }
}

double gveto_locator::get_column_y(uint32_t side_, uint32_t wall_, uint32_t column_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">" << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");

  if (side_ == (uint32_t)utils::SIDE_BACK) {
    DT_THROW_IF(
        column_ >= _back_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _back_block_y_[wall_].size() - 1 << ")!");
    return _back_block_y_[wall_][column_];
  } else {
    DT_THROW_IF(
        column_ >= _front_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _front_block_y_[wall_].size() - 1 << ")!");
    return _front_block_y_[wall_][column_];
  }
}

void gveto_locator::compute_block_position(uint32_t side_, uint32_t wall_, uint32_t column_,
                                           geomtools::vector_3d &module_position_) const {
  geomtools::invalidate(module_position_);
  module_position_.set(get_column_x(side_, wall_, column_), get_column_y(side_, wall_, column_),
                       get_wall_z(side_, wall_));
  return;
}

void gveto_locator::compute_block_window_position(uint32_t side_, uint32_t wall_, uint32_t column_,
                                                  geomtools::vector_3d &module_position_) const {
  geomtools::invalidate(module_position_);
  module_position_.set(get_column_x(side_, wall_, column_), get_column_y(side_, wall_, column_),
                       get_wall_z(side_, wall_));
  return;
}

geomtools::vector_3d gveto_locator::get_block_position(uint32_t side_, uint32_t wall_,
                                                       uint32_t column_) const {
  geomtools::vector_3d module_block_pos;
  compute_block_position(side_, wall_, column_, module_block_pos);
  return module_block_pos;
}

geomtools::vector_3d gveto_locator::get_block_window_position(uint32_t side_, uint32_t wall_,
                                                              uint32_t column_) const {
  geomtools::vector_3d module_block_pos;
  compute_block_window_position(side_, wall_, column_, module_block_pos);
  return module_block_pos;
}

size_t gveto_locator::get_number_of_neighbours(const geomtools::geom_id &gid_,
                                               uint8_t mask_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(gid_.get_depth() != 4, std::logic_error,
              "Invalid depth(" << gid_.get_depth() << " != 4)!");
  DT_THROW_IF(gid_.get(_module_address_index_) != _module_number_, std::logic_error,
              "Invalid module number(" << gid_.get(_module_address_index_)
                                       << "!=" << _module_number_ << ")!");
  return get_number_of_neighbours(gid_.get(_side_address_index_), gid_.get(_wall_address_index_),
                                  gid_.get(_column_address_index_), mask_);
}

void gveto_locator::get_block_position(const geomtools::geom_id &gid_,
                                       geomtools::vector_3d &position_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(gid_.get_depth() != 4, std::logic_error,
              "Invalid depth(" << gid_.get_depth() << " != 4)!");
  DT_THROW_IF(gid_.get(_module_address_index_) != _module_number_, std::logic_error,
              "Invalid module number(" << gid_.get(_module_address_index_)
                                       << "!=" << _module_number_ << ")!");
  return get_block_position(gid_.get(_side_address_index_), gid_.get(_wall_address_index_),
                            gid_.get(_column_address_index_), position_);
}

void gveto_locator::get_block_position(uint32_t side_, uint32_t wall_, uint32_t column_,
                                       geomtools::vector_3d &position_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">" << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");
  geomtools::invalidate(position_);
  if (side_ == (uint32_t)utils::SIDE_BACK) {
    DT_THROW_IF(
        column_ >= _back_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _back_block_y_[wall_].size() - 1 << ")!");
    position_.set(_back_block_x_[wall_][0], _back_block_y_[wall_][column_],
                  _block_z_[side_][wall_]);
  } else {
    DT_THROW_IF(
        column_ >= _front_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _front_block_y_[wall_].size() - 1 << ")!");
    position_.set(_front_block_x_[wall_][0], _front_block_y_[wall_][column_],
                  _block_z_[side_][wall_]);
  }
  return;
}

void gveto_locator::get_neighbours_ids(const geomtools::geom_id &gid_,
                                       std::vector<geomtools::geom_id> &ids_, uint8_t mask_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(gid_.get_depth() != 4, std::logic_error,
              "Invalid depth(" << gid_.get_depth() << " != 4)!");
  DT_THROW_IF(gid_.get(_module_address_index_) != _module_number_, std::logic_error,
              "Invalid module number(" << gid_.get(_module_address_index_)
                                       << "!=" << _module_number_ << ")!");
  get_neighbours_ids(gid_.get(_side_address_index_), gid_.get(_wall_address_index_),
                     gid_.get(_column_address_index_), ids_, mask_);
  return;
}

void gveto_locator::get_neighbours_ids(uint32_t side_, uint32_t wall_, uint32_t column_,
                                       std::vector<geomtools::geom_id> &ids_, uint8_t mask_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">= " << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");

  ids_.clear();
  ids_.reserve(8);

  const bool sides = mask_ & utils::NEIGHBOUR_SIDE;
  const bool diagonal = mask_ & utils::NEIGHBOUR_DIAG;
  const bool second = mask_ & utils::NEIGHBOUR_SECOND;
  if (second) {
    DT_LOG_NOTICE(get_logging_priority(),
                  "Looking for second order neighbour of 'gveto' locator is not implemented !");
  }

  // prepare neighbour GID :
  geomtools::geom_id gid;
  gid.set_type(_block_type_);
  gid.set(_module_address_index_, _module_number_);
  gid.set(_wall_address_index_, wall_);
  gid.set(_side_address_index_, side_);
  gid.set(_column_address_index_, geomtools::geom_id::INVALID_ADDRESS);
  if (is_block_partitioned()) {
    gid.set(_part_address_index_, _block_part_);
  }
  // back
  if (side_ == (uint32_t)utils::SIDE_BACK) {
    DT_THROW_IF(
        column_ >= _back_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _back_block_y_[wall_].size() - 1 << ")!");
    if (sides && (column_ < (_back_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][ ]
       *  [ ][.][x]
       *  [ ][ ][ ]
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }
    if (sides && (column_ > 0)) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [x][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ < (_back_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][x] R+1
       *  [ ][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ > 0)) {
      /*  C-1 C C+1
       *  [x][ ][ ] R+1
       *  [ ][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ > 0)) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [ ][.][ ] R
       *  [x][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ < (_back_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [ ][.][ ] R
       *  [ ][ ][x] R-1
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }
  }

  // front:
  if (side_ == (uint32_t)utils::SIDE_FRONT) {
    DT_THROW_IF(
        column_ >= _front_block_y_[wall_].size(), std::logic_error,
        "Invalid column number(" << column_ << ">" << _front_block_y_[wall_].size() - 1 << ")!");
    if (sides && (column_ < (_front_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][ ]
       *  [ ][.][x]
       *  [ ][ ][ ]
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }
    if (sides && (column_ > 0)) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [x][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ < (_front_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][x] R+1
       *  [ ][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ > 0)) {
      /*  C-1 C C+1
       *  [x][ ][ ] R+1
       *  [ ][.][ ] R
       *  [ ][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ > 0)) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [ ][.][ ] R
       *  [x][ ][ ] R-1
       */
      gid.set(_column_address_index_, column_ - 1);
      ids_.push_back(gid);
    }

    if (diagonal && (column_ < (_front_block_y_[wall_].size() - 1))) {
      /*  C-1 C C+1
       *  [ ][ ][ ] R+1
       *  [ ][.][ ] R
       *  [ ][ ][x] R-1
       */
      gid.set(_column_address_index_, column_ + 1);
      ids_.push_back(gid);
    }
  }

  return;
}

size_t gveto_locator::get_number_of_neighbours(uint32_t side_, uint32_t wall_, uint32_t column_,
                                               uint8_t mask_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">= " << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");

  bool corner = false;
  bool side = false;
  const bool sides = mask_ & utils::NEIGHBOUR_SIDE;
  const bool diagonal = mask_ & utils::NEIGHBOUR_DIAG;
  const bool second = mask_ & utils::NEIGHBOUR_SECOND;
  if (second) {
    DT_LOG_NOTICE(get_logging_priority(),
                  "Looking for second order neighbour of 'gveto' locator is not implemented !");
  }

  if (side_ == (uint32_t)utils::SIDE_BACK) {
    if ((column_ == 0) || (column_ == _back_block_y_[wall_].size() - 1)) {
      side = true;
    }
  }
  if (side_ == (uint32_t)utils::SIDE_FRONT) {
    if ((column_ == 0) || (column_ == _front_block_y_[wall_].size() - 1)) {
      side = true;
    }
  }
  size_t number = 0;
  if (corner) {
    if (sides) number += 2;
    if (diagonal) number += 1;
  } else if (side) {
    if (sides) number += 3;
    if (diagonal) number += 2;
  } else {
    if (sides) number += 4;
    if (diagonal) number += 4;
  }
  return number;
}

size_t gveto_locator::get_number_of_columns(uint32_t side_, uint32_t wall_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(side_ >= utils::NSIDES, std::logic_error,
              "Invalid side number(" << side_ << ">= " << utils::NSIDES << ")!");
  DT_THROW_IF(wall_ >= NWALLS_PER_SIDE, std::logic_error,
              "Invalid wall number(" << wall_ << ">" << NWALLS_PER_SIDE << ")!");
  size_t nbr_columns = 0;
  if (side_ == (uint32_t)utils::SIDE_BACK) {
    nbr_columns = _back_block_y_[wall_].size();
  } else if (side_ == (uint32_t)utils::SIDE_FRONT) {
    nbr_columns = _front_block_y_[wall_].size();
  }
  return nbr_columns;
}

void gveto_locator::_set_defaults_() {
  _module_type_ = geomtools::geom_id::INVALID_TYPE;
  _tracker_submodule_type_ = geomtools::geom_id::INVALID_TYPE;
  _block_type_ = geomtools::geom_id::INVALID_TYPE;
  _wrapper_type_ = geomtools::geom_id::INVALID_TYPE;
  _module_number_ = geomtools::geom_id::INVALID_ADDRESS;
  _block_part_ = geomtools::geom_id::INVALID_ADDRESS;
  _block_partitioned_ = false;

  _mapping_ = 0;
  _id_manager_ = 0;
  _module_ginfo_ = 0;
  _module_world_placement_ = 0;
  _module_box_ = 0;
  _block_shape_ = 0;
  _composite_block_shape_ = false;
  _block_box_ = 0;

  for (unsigned int i = 0; i < utils::NSIDES; i++) {
    for (unsigned int j = 0; j < NWALLS_PER_SIDE; j++) {
      datatools::invalidate(_block_z_[i][j]);
      datatools::invalidate(_block_window_z_[i][j]);
    }
    _submodules_[i] = false;
  }

  datatools::invalidate(_block_width_);
  datatools::invalidate(_block_height_);
  datatools::invalidate(_block_thickness_);

  _module_address_index_ = geomtools::geom_id::INVALID_ADDRESS;
  _wall_address_index_ = geomtools::geom_id::INVALID_ADDRESS;
  _side_address_index_ = geomtools::geom_id::INVALID_ADDRESS;
  _column_address_index_ = geomtools::geom_id::INVALID_ADDRESS;
  _part_address_index_ = geomtools::geom_id::INVALID_ADDRESS;

  _initialized_ = false;
  return;
}

// Constructor:
gveto_locator::gveto_locator() : base_locator() {
  _set_defaults_();
  return;
}

// Constructor:
gveto_locator::gveto_locator(const ::geomtools::manager &mgr_, uint32_t module_number_)
    : base_locator() {
  _set_defaults_();

  set_geo_manager(mgr_);
  set_module_number(module_number_);

  return;
}

// dtor:
gveto_locator::~gveto_locator() {
  if (is_initialized()) {
    reset();
  }
  return;
}

bool gveto_locator::is_block_partitioned() const { return _block_partitioned_; }

void gveto_locator::_construct() {
  // analyse the geometry versioning :
  datatools::version_id geom_mgr_setup_vid;
  get_geo_manager().fetch_setup_version_id(geom_mgr_setup_vid);

  const std::string module_cat = "module";
  const std::string tracker_submodule_cat = "tracker_submodule";
  const std::string gveto_block_cat = "gveto_block";
  const std::string gveto_wrapper_cat = "gveto_wrapper";

  _mapping_ = &get_geo_manager().get_mapping();
  _id_manager_ = &get_geo_manager().get_id_mgr();

  const geomtools::id_mgr::categories_by_name_col_type &categories =
      _id_manager_->categories_by_name();

  DT_THROW_IF(categories.find(module_cat) == categories.end(), std::logic_error,
              "No category named '" << module_cat << "' !");
  _module_type_ = categories.find(module_cat)->second.get_type();

  DT_THROW_IF(categories.find(tracker_submodule_cat) == categories.end(), std::logic_error,
              "No category named '" << tracker_submodule_cat << "' !");
  _tracker_submodule_type_ = categories.find(tracker_submodule_cat)->second.get_type();

  DT_THROW_IF(categories.find(gveto_block_cat) == categories.end(), std::logic_error,
              "No category named '" << gveto_block_cat << "' !");
  _block_type_ = categories.find(gveto_block_cat)->second.get_type();

  // Analyse the layout of the calo block's geometry category :
  const geomtools::id_mgr::category_info &block_ci = categories.find(gveto_block_cat)->second;
  DT_THROW_IF(!block_ci.has_subaddress("module"), std::logic_error,
              "Category '" << gveto_block_cat << "' has no subaddress 'module' !");
  _module_address_index_ = block_ci.get_subaddress_index("module");

  DT_THROW_IF(!block_ci.has_subaddress("side"), std::logic_error,
              "Category '" << gveto_block_cat << "' has no subaddress 'side' !");
  _side_address_index_ = block_ci.get_subaddress_index("side");

  DT_THROW_IF(!block_ci.has_subaddress("wall"), std::logic_error,
              "Category '" << gveto_block_cat << "' has no subaddress 'wall' !");
  _wall_address_index_ = block_ci.get_subaddress_index("wall");

  DT_THROW_IF(!block_ci.has_subaddress("column"), std::logic_error,
              "Category '" << gveto_block_cat << "' has no subaddress 'column' !");
  _column_address_index_ = block_ci.get_subaddress_index("column");

  _part_address_index_ = geomtools::geom_id::INVALID_ADDRESS;
  if (block_ci.has_subaddress("part")) {
    _block_partitioned_ = true;
    _part_address_index_ = block_ci.get_subaddress_index("part");
  } else {
    _block_partitioned_ = false;
  }

  DT_THROW_IF(categories.find(gveto_wrapper_cat) == categories.end(), std::logic_error,
              "No category named '" << gveto_wrapper_cat << "' !");
  _wrapper_type_ = categories.find(gveto_wrapper_cat)->second.get_type();

  // Fetch the GID of the requested module :
  geomtools::geom_id module_gid(_module_type_, _module_number_);
  DT_THROW_IF(!_mapping_->validate_id(module_gid), std::logic_error,
              "No module with ID = " << module_gid << " !");
  _module_ginfo_ = &_mapping_->get_geom_info(module_gid);
  const geomtools::i_shape_3d *a_shape = &_module_ginfo_->get_logical().get_shape();
  DT_THROW_IF(a_shape->get_shape_name() != "box", std::logic_error,
              "Cannot extract the box shape from module with ID = " << module_gid << " !");
  _module_box_ = dynamic_cast<const geomtools::box *>(a_shape);
  _module_world_placement_ = &_module_ginfo_->get_world_placement();

  if (is_block_partitioned()) {
    // Check the requested block part number if any
  }

  // Search for tracker submodules :
  geomtools::geom_id side_gid;
  side_gid.set_type(_tracker_submodule_type_);
  uint32_t side = geomtools::geom_id::INVALID_ADDRESS;
  //_mapping_->smart_print(std::std::clog, "*** ");
  for (uint32_t iside = 0; iside < utils::NSIDES; iside++) {
    side_gid.set_address(_module_number_, iside);
    if (_mapping_->validate_id(side_gid)) {
      _submodules_[iside] = true;
      side = iside;
    }
  }
  DT_THROW_IF(side == geomtools::geom_id::INVALID_ADDRESS, std::logic_error,
              "Cannot extract information about any tracker submodules !");

  // Pick up the first available block in a submodule at given side :
  geomtools::geom_id block_gid;
  block_gid.set_type(_block_type_);
  // gid is module.side.wall.column:
  if (_block_partitioned_) {
    block_gid.set_address(_module_number_, side, 0, 0, _block_part_);
  } else {
    block_gid.set_address(_module_number_, side, 0, 0);
  }

  DT_THROW_IF(!_mapping_->validate_id(block_gid), std::logic_error,
              "Cannot extract information about a block with ID = " << block_gid << " !");
  const geomtools::geom_info &block_ginfo = _mapping_->get_geom_info(block_gid);
  const geomtools::i_shape_3d &b_shape = block_ginfo.get_logical().get_shape();
  _block_shape_ = &b_shape;
  if (_block_shape_->get_shape_name() == "subtraction_3d") {
    // Example : 'calo_scin_box_model' case :
    _composite_block_shape_ = true;

    const geomtools::subtraction_3d &ref_s3d =
        dynamic_cast<const geomtools::subtraction_3d &>(*_block_shape_);
    const geomtools::i_composite_shape_3d::shape_type &sht1 = ref_s3d.get_shape1();
    const geomtools::i_shape_3d &sh1 = sht1.get_shape();
    DT_THROW_IF(sh1.get_shape_name() != "box", std::logic_error,
                "Do not support non-box shaped block with ID = " << block_gid << " !");
    _block_box_ = dynamic_cast<const geomtools::box *>(&sh1);
  } else if (_block_shape_->get_shape_name() == "intersection_3d") {
    // Example : 'calo_tapered_scin_box_model' case :
    _composite_block_shape_ = true;

    const geomtools::intersection_3d &ref_i3d =
        dynamic_cast<const geomtools::intersection_3d &>(*_block_shape_);
    const geomtools::i_composite_shape_3d::shape_type &sht1 = ref_i3d.get_shape1();
    const geomtools::i_shape_3d &sh1 = sht1.get_shape();
    DT_THROW_IF(sh1.get_shape_name() != "subtraction_3d", std::logic_error,
                "Do not support non-subtraction_3d shaped block with ID = " << block_gid << " !");
    const geomtools::subtraction_3d &ref_s3d = dynamic_cast<const geomtools::subtraction_3d &>(sh1);
    const geomtools::i_composite_shape_3d::shape_type &sht11 = ref_s3d.get_shape1();
    const geomtools::i_shape_3d &sh11 = sht11.get_shape();
    DT_THROW_IF(sh11.get_shape_name() != "box", std::logic_error,
                "Do not support non-box shaped block with ID = " << block_gid << " !");
    _block_box_ = dynamic_cast<const geomtools::box *>(&sh11);
  } else if (_block_shape_->get_shape_name() == "box") {
    // Simple box case :
    _composite_block_shape_ = false;
    _block_box_ = dynamic_cast<const geomtools::box *>(_block_shape_);
  } else {
    DT_THROW_IF(true, std::logic_error,
                "Cannot extract the shape from block with ID = " << block_gid << " !");
  }
  std::vector<double> *vcx[utils::NSIDES][NWALLS_PER_SIDE];
  vcx[utils::SIDE_BACK][WALL_TOP] = &_back_block_x_[WALL_TOP];
  vcx[utils::SIDE_BACK][WALL_BOTTOM] = &_back_block_x_[WALL_BOTTOM];
  vcx[utils::SIDE_FRONT][WALL_TOP] = &_front_block_x_[WALL_TOP];
  vcx[utils::SIDE_FRONT][WALL_BOTTOM] = &_front_block_x_[WALL_BOTTOM];
  std::vector<double> *vcy[utils::NSIDES][NWALLS_PER_SIDE];
  vcy[utils::SIDE_BACK][WALL_TOP] = &_back_block_y_[WALL_TOP];
  vcy[utils::SIDE_BACK][WALL_BOTTOM] = &_back_block_y_[WALL_BOTTOM];
  vcy[utils::SIDE_FRONT][WALL_TOP] = &_front_block_y_[WALL_TOP];
  vcy[utils::SIDE_FRONT][WALL_BOTTOM] = &_front_block_y_[WALL_BOTTOM];
  for (unsigned int iside = 0; iside < utils::NSIDES; iside++) {
    if (!_submodules_[iside]) continue;
    for (unsigned int wall = 0; wall < NWALLS_PER_SIDE; wall++) {
      size_t i_column = 0;
      vcx[iside][wall]->reserve(1);
      vcy[iside][wall]->reserve(16);
      while (true) {
        geomtools::geom_id a_block_gid;
        a_block_gid.set_type(_block_type_);
        a_block_gid.set(_module_address_index_, _module_number_);
        a_block_gid.set(_side_address_index_, iside);
        a_block_gid.set(_wall_address_index_, wall);
        a_block_gid.set(_column_address_index_, i_column);
        if (is_block_partitioned()) {
          a_block_gid.set(_part_address_index_, _block_part_);
        }
        if (!_mapping_->validate_id(a_block_gid)) {
          break;
        }
        const geomtools::geom_info &a_block_ginfo = _mapping_->get_geom_info(a_block_gid);
        const geomtools::placement &a_block_world_placement = a_block_ginfo.get_world_placement();
        geomtools::placement a_block_module_placement;
        _module_world_placement_->relocate(a_block_world_placement, a_block_module_placement);
        const double y = a_block_module_placement.get_translation().y();
        vcy[iside][wall]->push_back(y);
        if (i_column == 0) {
          const double x = a_block_module_placement.get_translation().x();
          vcx[iside][wall]->push_back(x);
          const double z = a_block_module_placement.get_translation().z();
          _block_z_[iside][wall] = z;

          geomtools::geom_id a_block_window_gid(_wrapper_type_, _module_number_, iside, wall,
                                                i_column);
          const geomtools::geom_info &a_block_window_ginfo =
              _mapping_->get_geom_info(a_block_window_gid);
          const geomtools::placement &a_block_window_world_placement =
              a_block_window_ginfo.get_world_placement();
          geomtools::placement a_block_window_module_placement;
          _module_world_placement_->relocate(a_block_window_world_placement,
                                             a_block_window_module_placement);
          const double z2 = a_block_window_module_placement.get_translation().z();
          _block_window_z_[iside][wall] = z2;
        }
        i_column++;
      }
    }
  }

  _block_width_ = _block_box_->get_x();
  _block_height_ = _block_box_->get_y();
  _block_thickness_ = _block_box_->get_z();

  return;
}

int gveto_locator::get_module_address_index() const { return _module_address_index_; }

int gveto_locator::get_side_address_index() const { return _side_address_index_; }

int gveto_locator::get_wall_address_index() const { return _wall_address_index_; }

int gveto_locator::get_column_address_index() const { return _column_address_index_; }

int gveto_locator::get_part_address_index() const { return _part_address_index_; }

uint32_t gveto_locator::extract_module(const geomtools::geom_id &gid_) const {
  return gid_.get(_module_address_index_);
}

uint32_t gveto_locator::extract_side(const geomtools::geom_id &gid_) const {
  return gid_.get(_side_address_index_);
}

uint32_t gveto_locator::extract_wall(const geomtools::geom_id &gid_) const {
  return gid_.get(_wall_address_index_);
}

uint32_t gveto_locator::extract_column(const geomtools::geom_id &gid_) const {
  return gid_.get(_column_address_index_);
}

uint32_t gveto_locator::extract_part(const geomtools::geom_id &gid_) const {
  if (is_block_partitioned()) {
    return gid_.get(_part_address_index_);
  }
  return geomtools::geom_id::INVALID_ADDRESS;
}

bool gveto_locator::is_calo_block(const geomtools::geom_id &gid_) const {
  return gid_.get_type() == _block_type_;
}

bool gveto_locator::is_calo_block_in_current_module(const geomtools::geom_id &gid_) const {
  return is_calo_block(gid_) && (extract_module(gid_) == _module_number_);
}

void gveto_locator::set_module_number(uint32_t a_module_number) {
  DT_THROW_IF(is_initialized(), std::logic_error, "Locator is already initialized !");
  _module_number_ = a_module_number;
  return;
}

uint32_t gveto_locator::get_module_number() const { return _module_number_; }

void gveto_locator::initialize() {
  datatools::properties dummy;
  initialize(dummy);
  return;
}

void gveto_locator::initialize(const datatools::properties &config_) {
  base_locator::_basic_initialize(config_);
  DT_THROW_IF(_module_number_ == geomtools::geom_id::INVALID_ADDRESS, std::logic_error,
              "Missing module number ! Use the 'set_module_number' method before !");
  _hack_trace();
  _construct();
  _initialized_ = true;
  if (datatools::logger::is_trace(get_logging_priority())) {
    tree_dump(std::cerr, "Gamma-veto locator : ", "[trace] ");
  }
  return;
}

void gveto_locator::dump(std::ostream &out_) const {
  gveto_locator::tree_dump(out_, "snemo::geometry:gveto_locator::dump: ");
  return;
}

void gveto_locator::tree_dump(std::ostream &out_, const std::string &title_,
                              const std::string &indent_, bool inherit_) const {
  const std::string itag = datatools::i_tree_dumpable::tags::item();
  const std::string stag = datatools::i_tree_dumpable::tags::skip_item();
  std::string indent;
  if (!indent_.empty()) {
    indent = indent_;
  }
  if (!title_.empty()) {
    out_ << indent << title_ << std::endl;
  }
  if (!is_initialized()) {
    out_ << indent << datatools::i_tree_dumpable::inherit_tag(inherit_)
         << "Initialized        = " << _initialized_ << std::endl;
    return;
  }
  out_ << indent << itag << "Initialized                = " << _initialized_ << std::endl;
  out_ << indent << itag << "Logging priority threshold = "
       << datatools::logger::get_priority_label(get_logging_priority()) << std::endl;
  out_ << indent << itag << "Module number              = " << _module_number_ << std::endl;
  out_ << indent << itag << "Manager @                  = " << &get_geo_manager() << std::endl;
  out_ << indent << itag << "Mapping @                  = " << _mapping_ << std::endl;
  out_ << indent << itag << "ID manager @               = " << _id_manager_ << std::endl;
  out_ << indent << itag << "Module type                = " << _module_type_ << std::endl;
  out_ << indent << itag << "Tracker submodule type     = " << _tracker_submodule_type_
       << std::endl;
  out_ << indent << itag << "Calorimeter block type     = " << _block_type_ << std::endl;
  out_ << indent << itag << "Calorimeter wrapper type   = " << _wrapper_type_ << std::endl;
  out_ << indent << itag << "Block partitioned          = " << _block_partitioned_ << std::endl;
  if (is_block_partitioned())
    out_ << indent << itag << "Block part                 = " << _block_part_ << std::endl;
  out_ << indent << itag << "Module ginfo @             = " << _module_ginfo_ << std::endl;
  out_ << indent << itag << "Module placement : " << std::endl;
  if (_module_world_placement_ != 0) {
    _module_world_placement_->tree_dump(out_, "", indent + stag);
  }
  out_ << indent << itag << "Module box : " << std::endl;
  if (_module_box_ != 0) {
    _module_box_->tree_dump(out_, "", indent + stag);
  }
  out_ << indent << itag << "Back  submodule : " << _submodules_[utils::SIDE_BACK] << std::endl;
  out_ << indent << itag << "Front submodule : " << _submodules_[utils::SIDE_FRONT] << std::endl;
  out_ << indent << itag << "Block shape : " << _block_shape_->get_shape_name() << std::endl;
  out_ << indent << itag << "Composite block shape = " << _composite_block_shape_ << std::endl;
  out_ << indent << itag << "Block box : " << std::endl;
  if (_block_box_ != 0) {
    _block_box_->tree_dump(out_, "", indent + stag);
  }
  for (size_t i = 0; i < NWALLS_PER_SIDE; ++i) {
    const std::string wall_name = (i == (unsigned int)WALL_TOP) ? "top wall" : "bottom wall";
    out_ << indent << itag << "Back block X-pos on " << wall_name << " ["
         << _back_block_x_[i].size() << "] = ";
    for (size_t j = 0; j < _back_block_x_[i].size(); j++) {
      out_ << _back_block_x_[i][j] / CLHEP::mm << " ";
    }
    out_ << "(mm)" << std::endl;
    out_ << indent << itag << "Front block X-pos on " << wall_name << " ["
         << _front_block_x_[i].size() << "] = ";
    for (size_t j = 0; j < _front_block_x_[i].size(); j++) {
      out_ << _front_block_x_[i][j] / CLHEP::mm << " ";
    }
    out_ << "(mm)" << std::endl;
    out_ << indent << itag << "Back block Y-pos on " << wall_name << " ["
         << _back_block_y_[i].size() << "] = ";
    for (size_t j = 0; j < _back_block_y_[i].size(); j++) {
      if ((j < 4) || (j > _back_block_y_[i].size() - 4)) {
        out_ << _back_block_y_[i][j] / CLHEP::mm << " ";
      } else if (j == 4) {
        out_ << " ... ";
      }
    }
    out_ << "(mm)" << std::endl;
    out_ << indent << itag << "Front block Y-pos on " << wall_name << " ["
         << _front_block_y_[i].size() << "] = ";
    for (size_t j = 0; j < _front_block_y_[i].size(); j++) {
      if ((j < 4) || (j > _front_block_y_[i].size() - 4)) {
        out_ << _front_block_y_[i][j] / CLHEP::mm << " ";
      } else if (j == 4) {
        out_ << " ... ";
      }
    }
    out_ << "(mm)" << std::endl;
    out_ << indent << itag << "Back block Z-pos on " << wall_name << "  = "
         << _block_z_[utils::SIDE_BACK][i] / CLHEP::mm << "(mm) \n";
    out_ << indent << itag << "Front block Z-pos on " << wall_name << " = "
         << _block_z_[utils::SIDE_FRONT][i] / CLHEP::mm << "(mm) \n";
    out_ << indent << itag << "Back block window Z-pos on " << wall_name << " = "
         << _block_window_z_[utils::SIDE_BACK][i] / CLHEP::mm << "(mm) \n";
    out_ << indent << itag << "Front block window Z-pos on " << wall_name << " = "
         << _block_window_z_[utils::SIDE_FRONT][i] / CLHEP::mm << "(mm) \n";
  }

  out_ << indent << itag << "Block width              = " << _block_width_ / CLHEP::mm << "(mm)"
       << std::endl;
  out_ << indent << itag << "Block height             = " << _block_height_ / CLHEP::mm << "(mm)"
       << std::endl;
  out_ << indent << itag << "Block thickness          = " << _block_thickness_ / CLHEP::mm << "(mm)"
       << std::endl;
  out_ << indent << itag << "Module address GID index = " << _module_address_index_ << std::endl;
  out_ << indent << itag << "Side address GID index   = " << _side_address_index_ << std::endl;
  out_ << indent << itag << "Wall address GID index   = " << _wall_address_index_ << std::endl;
  out_ << indent << itag << "Column address GID index = " << _column_address_index_ << std::endl;
  if (is_block_partitioned()) {
    out_ << indent << datatools::i_tree_dumpable::inherit_tag(inherit_)
         << "Part address GID index   = " << _part_address_index_ << std::endl;
  }
  return;
}

void gveto_locator::reset() {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  for (unsigned int i = 0; i < NWALLS_PER_SIDE; i++) {
    _back_block_x_[i].clear();
    _front_block_x_[i].clear();
    _back_block_y_[i].clear();
    _front_block_y_[i].clear();
  }
  _set_defaults_();
  return;
}

void gveto_locator::transform_world_to_module(const geomtools::vector_3d &world_position_,
                                              geomtools::vector_3d &module_position_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  _module_world_placement_->mother_to_child(world_position_, module_position_);
  return;
}

void gveto_locator::transform_module_to_world(const geomtools::vector_3d &module_position_,
                                              geomtools::vector_3d &world_position_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  _module_world_placement_->child_to_mother(module_position_, world_position_);
  return;
}

bool gveto_locator::is_in_module(const geomtools::vector_3d &module_position_,
                                 double tolerance_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  return _module_box_->is_inside(module_position_, tolerance_);
}

bool gveto_locator::is_in_block(const geomtools::vector_3d &module_position_, uint32_t side_,
                                uint32_t wall_, uint32_t column_, double tolerance_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  geomtools::vector_3d to_block_pos = module_position_;
  to_block_pos -= get_block_position(side_, wall_, column_);
  // here one misses one transformation step(rotation) but it is ok :
  return _block_box_->is_inside(to_block_pos, tolerance_);
}

bool gveto_locator::is_world_position_in_block(const geomtools::vector_3d &world_position_,
                                               uint32_t side_, uint32_t wall_, uint32_t column_,
                                               double tolerance_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  geomtools::vector_3d in_module_position;
  this->transform_world_to_module(world_position_, in_module_position);
  return is_in_block(in_module_position, side_, wall_, column_, tolerance_);
}

bool gveto_locator::is_world_position_in_module(const geomtools::vector_3d &world_position_,
                                                double tolerance_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  geomtools::vector_3d in_module_position;
  this->transform_world_to_module(world_position_, in_module_position);
  return is_in_module(in_module_position, tolerance_);
}

bool gveto_locator::find_geom_id(const geomtools::vector_3d &world_position_, int type_,
                                 geomtools::geom_id &gid_, double tolerance_) const {
  DT_THROW_IF(!is_initialized(), std::logic_error, "Locator is not initialized !");
  DT_THROW_IF(type_ != (int)_block_type_, std::logic_error,
              "Only works with type " << _block_type_ << " for now !");

  gid_.invalidate();

  // Compute the position in the module coordinate system :
  geomtools::vector_3d in_module_position;
  this->transform_world_to_module(world_position_, in_module_position);

  // First check if it is inside the module :
  if (!_module_box_->is_inside(in_module_position, tolerance_)) {
    // Not in this module :
    return false;
  }
  return const_cast<gveto_locator *>(this)->gveto_locator::find_block_geom_id_(in_module_position,
                                                                               gid_, tolerance_);
}

bool gveto_locator::id_is_valid(uint32_t side_, uint32_t wall_, uint32_t column_) const {
  if (side_ >= utils::NSIDES) {
    return false;
  }
  if (wall_ >= NWALLS_PER_SIDE) {
    return false;
  }
  if (column_ >= get_number_of_columns(side_, wall_)) {
    return false;
  }
  return true;
}

void gveto_locator::_hack_trace() {
  char *ev = getenv("FLGEOMLOCATOR");
  if (ev != 0) {
    std::string evstr(ev);
    if (evstr == "trace") {
      set_logging_priority(datatools::logger::PRIO_TRACE);
      DT_LOG_TRACE(get_logging_priority(),
                   "Trace logging activated through env 'FLGEOMLOCATOR'...");
    }
  }
  return;
}

bool gveto_locator::find_block_geom_id(const geomtools::vector_3d &world_position_,
                                       geomtools::geom_id &gid_, double tolerance_) const {
  return find_geom_id(world_position_, _block_type_, gid_, tolerance_);
}

bool gveto_locator::find_block_geom_id_(const geomtools::vector_3d &in_module_position_,
                                        geomtools::geom_id &gid_, double tolerance_) {
  DT_LOG_TRACE_ENTERING(get_logging_priority());

  double tolerance = tolerance_;
  if (tolerance == GEOMTOOLS_PROPER_TOLERANCE) {
    tolerance = _block_box_->get_tolerance();
  }

  geomtools::geom_id &gid = gid_;
  gid.reset();
  uint32_t side_number(geomtools::geom_id::INVALID_ADDRESS);
  uint32_t wall_number(geomtools::geom_id::INVALID_ADDRESS);
  uint32_t column_number(geomtools::geom_id::INVALID_ADDRESS);
  const double x = in_module_position_.x();
  const double xlim = 1000 * CLHEP::m;
  if (std::abs(x) < xlim) {
    gid.set_type(_block_type_);
    gid.set(_module_address_index_, _module_number_);
    if (is_block_partitioned()) {
      gid.set(_part_address_index_, _block_part_);
    }
    const double y = in_module_position_.y();
    const double z = in_module_position_.z();
    DT_LOG_TRACE(get_logging_priority(), "x = " << x / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "y = " << y / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "z = " << z / CLHEP::mm);
    double first_block_y;
    double block_delta_y;
    size_t ncolumns = 0;

    // Find the side:
    if (side_number == geomtools::geom_id::INVALID_ADDRESS && _submodules_[utils::SIDE_BACK]) {
      double xmax0 = _back_block_x_[WALL_BOTTOM].front() + 0.5 * get_block_width() + tolerance;
      double xmax1 = _back_block_x_[WALL_TOP].front() + 0.5 * get_block_width() + tolerance;
      double xmax = std::max(xmax0, xmax1);
      if (x <= xmax) {
        side_number = utils::SIDE_BACK;
      }
    }
    if (side_number == geomtools::geom_id::INVALID_ADDRESS && _submodules_[utils::SIDE_FRONT]) {
      double xmin0 = _front_block_x_[WALL_BOTTOM].front() - 0.5 * get_block_width() - tolerance;
      double xmin1 = _front_block_x_[WALL_TOP].front() - 0.5 * get_block_width() - tolerance;
      double xmin = std::min(xmin0, xmin1);
      if (x >= xmin) {
        side_number = utils::SIDE_FRONT;
      }
    }
    DT_LOG_TRACE(get_logging_priority(), "side_number = " << side_number);
    if (side_number == geomtools::geom_id::INVALID_ADDRESS) {
      DT_LOG_TRACE(get_logging_priority(), "Not a G-veto!");
      gid.invalidate();
      DT_LOG_TRACE_EXITING(get_logging_priority());
      return false;
    }

    // Find the wall:
    // 2012-06-13 XG: For config 2.0 the 16 gveto block
    // are separated into two series of 8 blocks : between
    // these series there is a gap arround y=0. To
    // determine the column number then we have to take
    // care of this gap by splitting the column range into
    // two separate vectors. This of course does not
    // change anything to config 1.0
    if (wall_number == geomtools::geom_id::INVALID_ADDRESS && z < 0.0) {
      const double delta_z =
          std::abs(z - _block_z_[side_number][WALL_BOTTOM]) - 0.5 * get_block_thickness();
      if (delta_z < tolerance) {
        wall_number = WALL_BOTTOM;
        DT_LOG_TRACE(get_logging_priority(), "WALL_BOTTOM: wall_number=" << wall_number);
        const std::vector<double> *block_y_ptr = 0;
        if (_submodules_[utils::SIDE_BACK] && side_number == utils::SIDE_BACK) {
          block_y_ptr = &_back_block_y_[wall_number];
        }
        if (_submodules_[utils::SIDE_FRONT] && side_number == utils::SIDE_FRONT) {
          block_y_ptr = &_front_block_y_[wall_number];
        }
        ncolumns = block_y_ptr->size() / 2;
        DT_LOG_TRACE(get_logging_priority(), "WALL_BOTTOM: ncolumns=" << ncolumns);
        if (y < 0.0) {
          first_block_y = block_y_ptr->front();
          block_delta_y = (block_y_ptr->at(ncolumns - 1) - block_y_ptr->front()) / (ncolumns - 1);
        } else {
          first_block_y = block_y_ptr->at(ncolumns);
          block_delta_y = (block_y_ptr->back() - block_y_ptr->at(ncolumns)) / (ncolumns - 1);
        }
      }
    }
    if (wall_number == geomtools::geom_id::INVALID_ADDRESS && z > 0.0) {
      const double delta_z =
          std::abs(z - _block_z_[side_number][WALL_TOP]) - 0.5 * get_block_thickness();
      if (delta_z < tolerance) {
        wall_number = WALL_TOP;
        DT_LOG_TRACE(get_logging_priority(), "WALL_TOP:  wall_number=" << wall_number);
        const std::vector<double> *block_y_ptr = 0;
        if (_submodules_[utils::SIDE_BACK] && side_number == utils::SIDE_BACK) {
          block_y_ptr = &_back_block_y_[wall_number];
        }
        if (_submodules_[utils::SIDE_FRONT] && side_number == utils::SIDE_FRONT) {
          block_y_ptr = &_front_block_y_[wall_number];
        }
        ncolumns = block_y_ptr->size() / 2;
        DT_LOG_TRACE(get_logging_priority(), "WALL_TOP:  ncolumns=" << ncolumns);
        if (y < 0.0) {
          first_block_y = _front_block_y_[wall_number].front();
          block_delta_y = (_front_block_y_[wall_number].at(ncolumns - 1) -
                           _front_block_y_[wall_number].front()) /
                          (ncolumns - 1);
        } else {
          first_block_y = _front_block_y_[wall_number].at(ncolumns);
          block_delta_y =
              (_front_block_y_[wall_number].back() - _front_block_y_[wall_number].at(ncolumns)) /
              (ncolumns - 1);
        }
      }
    }
    if (wall_number == geomtools::geom_id::INVALID_ADDRESS) {
      DT_LOG_TRACE(get_logging_priority(), "Not a G-veto!");
      gid.invalidate();
      DT_LOG_TRACE_EXITING(get_logging_priority());
      return false;
    }

    DT_LOG_TRACE(get_logging_priority(), "side_number   = " << side_number);
    DT_LOG_TRACE(get_logging_priority(), "wall_number   = " << wall_number);
    DT_LOG_TRACE(get_logging_priority(), "ncolumns      = " << ncolumns);
    DT_LOG_TRACE(get_logging_priority(), "first_block_y = " << first_block_y / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "block_delta_y = " << block_delta_y / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "x             = " << x / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "y             = " << y / CLHEP::mm);
    DT_LOG_TRACE(get_logging_priority(), "z             = " << z / CLHEP::mm);

    gid.set(_side_address_index_, side_number);
    gid.set(_wall_address_index_, wall_number);
    DT_LOG_TRACE(get_logging_priority(), "gid = " << gid);
    const int iy = (int)(((y - first_block_y) / block_delta_y) + 0.5);
    if ((iy >= 0) && (iy < (int)ncolumns)) {
      column_number = iy;
      if (y > 0.0) column_number += ncolumns;
    }
    gid.set(_column_address_index_, column_number);
    DT_LOG_TRACE(get_logging_priority(), "gid = " << gid);
    if (gid.is_valid()) {
      // 2012-05-31 FM : use ginfo from mapping(see below)
      const geomtools::geom_info *ginfo_ptr = _mapping_->get_geom_info_ptr(gid);
      if (ginfo_ptr == 0) {
        DT_LOG_TRACE(get_logging_priority(), "Unmapped gid = " << gid);
        DT_LOG_TRACE(get_logging_priority(), "Not a G-veto!");
        gid.invalidate();
        DT_LOG_TRACE_EXITING(get_logging_priority());
        return false;
      }
      DT_LOG_TRACE(get_logging_priority(), "Valid mapped gid = " << gid);
      // 2012-05-31 FM : we check if the 'world' position is in the volume:
      geomtools::vector_3d world_position;
      transform_module_to_world(in_module_position_, world_position);
      double tolerance_2 = 1.e-7 * CLHEP::mm;
      if (_mapping_->check_inside(*ginfo_ptr, world_position, tolerance_2)) {
        DT_LOG_TRACE(get_logging_priority(), "INSIDE " << gid);
        DT_LOG_TRACE_EXITING(get_logging_priority());
        return true;
      }
    }
    DT_LOG_TRACE(get_logging_priority(), "Not a G-veto!");
    gid.invalidate();
  }
  DT_LOG_TRACE_EXITING(get_logging_priority());
  return false;
}

}  // end of namespace geometry

}  // end of namespace snemo

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
