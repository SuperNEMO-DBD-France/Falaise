/* cylinder_volume.cc
 *
 * Copyright (C) 2011 Xavier Garrido <garrido@lal.in2p3.fr>
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
 */

#include <falaise/snemo/detector/cylinder_volume.h>

#include <geomtools/cylinder.h>
#include <geomtools/geom_info.h>

#include <TGeoManager.h>

namespace snemo {

namespace visualization {

namespace detector {

// ctor:
cylinder_volume::cylinder_volume(const std::string &name_, const std::string &category_)
    : i_root_volume(name_, category_) {
  _type = "cylinder";
  _composite = false;

  _inner_radius_ = 0.0;
  _outer_radius_ = 0.0;
  _height_ = 0.0;

  return;
}

// dtor:
cylinder_volume::~cylinder_volume() { return; }

void cylinder_volume::_construct(const geomtools::i_shape_3d &shape_3d_) {
  const geomtools::cylinder &mcylinder = dynamic_cast<const geomtools::cylinder &>(shape_3d_);

  _inner_radius_ = 0.0;
  _outer_radius_ = mcylinder.get_radius();
  _height_ = mcylinder.get_z();

  TGeoMaterial *material = new TGeoMaterial("Dummy");
  TGeoMedium *medium = new TGeoMedium("Dummy", 1, material);

  _geo_volume =
      gGeoManager->MakeTube(_name.c_str(), medium, _inner_radius_, _outer_radius_, _height_ / 2.);
  return;
}

void cylinder_volume::tree_dump(std::ostream &out_, const std::string &title_,
                                const std::string &indent_, bool inherit_) const {
  std::string indent;
  if (!indent_.empty()) indent = indent_;
  i_root_volume::tree_dump(out_, title_, indent_, true);

  out_ << indent << datatools::i_tree_dumpable::inherit_tag(inherit_) << "(r1, r2, h) : ("
       << _inner_radius_ << ", " << _outer_radius_ << ", " << _height_ << ")" << std::endl;
  return;
}

void cylinder_volume::dump() const {
  this->tree_dump(std::clog, "snemo::visualization::detector::cylinder_volume");
  return;
}

}  // end of namespace detector

}  // end of namespace visualization

}  // end of namespace snemo

// end of cylinder_volume.cc
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
