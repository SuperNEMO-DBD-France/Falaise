/* -*- mode: c++ -*- */
#ifndef __sultan__EXPERIMENTALHELIX
#define __sultan__EXPERIMENTALHELIX
#include <iostream>
#include <cmath>
#include <mybhep/utilities.h>
#include <sultan/experimental_point.h>
#include <sultan/cell.h>

namespace SULTAN {
namespace topology {

class experimental_helix : public tracking_object {
  // a experimental_helix is identified by origin,
  // radius
  // and pitch

 private:
  std::string appname_;

  experimental_point ep_;
  experimental_double R_;
  experimental_double H_;

  std::vector<size_t> ids_;

  // points in the helix are given by:
  // x(phi) = x0 + R*cos(phi)
  // y(phi) = y0 + R*sin(phi)
  // z(phi) = z0 + h*phi

 public:
  //! Default constructor
  experimental_helix(mybhep::prlevel level = mybhep::NORMAL, double probmin = 1.e-200) {
    appname_ = "experimental_helix: ";
    ep_ = experimental_point();
    R_ = experimental_double(mybhep::small_neg, mybhep::small_neg);
    H_ = experimental_double(mybhep::small_neg, mybhep::small_neg);
    ids_.clear();
    set_print_level(level);
    set_probmin(probmin);
  }

  //! Default destructor
  virtual ~experimental_helix(){};

  //! constructor
  experimental_helix(experimental_point ep, experimental_double R, experimental_double H,
                     mybhep::prlevel level = mybhep::NORMAL, double probmin = 1.e-200) {
    set_print_level(level);
    set_probmin(probmin);
    appname_ = "experimental_helix: ";
    ep_ = ep;
    R_ = R;
    H_ = H;
  }

  //! constructor
  experimental_helix(experimental_double x0, experimental_double y0, experimental_double z0,
                     experimental_double R, experimental_double H,
                     mybhep::prlevel level = mybhep::NORMAL, double probmin = 1.e-200) {
    set_print_level(level);
    set_probmin(probmin);
    appname_ = "experimental_helix: ";
    ep_.set(x0, y0, z0);
    R_ = R;
    H_ = H;
  }

  /*** dump ***/
  virtual void dump(std::ostream &a_out = std::clog, const std::string &a_title = "",
                    const std::string &a_indent = "", bool /*a_inherit*/ = false) const {
    {
      std::string indent;
      if (!a_indent.empty()) indent = a_indent;
      if (!a_title.empty()) {
        a_out << indent << a_title << std::endl;
      }

      a_out << indent << appname_ << " center [";
      ep_.dump();
      a_out << "] radius: ";
      R_.dump();
      a_out << " pitch: ";
      H_.dump();
      a_out << " ids: ";
      print_ids();
      a_out << " " << std::endl;

      return;
    }
  }

  void set_x0(experimental_double a) { ep_.set_x(a); }
  void set_y0(experimental_double a) { ep_.set_y(a); }
  void set_z0(experimental_double a) { ep_.set_z(a); }
  void set_ep(experimental_point a) { ep_ = a; }
  void set_R(experimental_double a) { R_ = a; }
  void set_H(experimental_double a) { H_ = a; }
  void set(experimental_point ep, experimental_double R, experimental_double H) {
    ep_ = ep;
    R_ = R;
    H_ = H;
  }
  void set(experimental_double x0, experimental_double y0, experimental_double z0,
           experimental_double R, experimental_double H) {
    ep_.set(x0, y0, z0);
    R_ = R;
    H_ = H;
  }

  std::vector<size_t> ids() const { return ids_; }
  void print_ids() const {
    std::clog << " ( ";
    for (std::vector<size_t>::const_iterator id = ids_.begin(); id != ids_.end(); ++id)
      std::clog << *id << " ";
    std::clog << ") ";
  }

  experimental_point center() const { return ep_; }
  experimental_double x0() const { return ep_.x(); }
  experimental_double y0() const { return ep_.y(); }
  experimental_double z0() const { return ep_.z(); }
  experimental_double R() const { return R_; }
  experimental_double H() const { return H_; }

  experimental_point position(const experimental_point &ep) const;

  // get the position at parameter phi
  experimental_point position(const experimental_double &phi) const {
    experimental_double deltax = experimental_cos(phi) * R();
    experimental_double deltay = experimental_sin(phi) * R();
    experimental_double deltaz = phi * H();

    return (experimental_vector(center()) + experimental_vector(deltax, deltay, deltaz))
        .point_from_vector();
  }

  void reset_ids() { ids_.clear(); }

  void add_id(size_t id);

  void add_ids(std::vector<size_t> ids);

  void distance_from_cell_measurement(topology::cell c, experimental_double *DR,
                                      experimental_double *DH) const;

  void distance_from_cell_center(topology::cell c, experimental_double *DR,
                                 experimental_double *DH) const;

  bool different_cells(topology::experimental_helix b) const;

  experimental_double phi_of_point(const experimental_point &ep, double phi_ref) const {
    // if no ref is given, phi is in [-pi, pi]
    // if ref is given is in [ref - \pi, ref + \pi]

    experimental_double phi = experimental_vector(center(), ep).phi();

    while (phi.value() - phi_ref > M_PI) {
      phi.set_value(phi.value() - 2. * M_PI);
    }

    while (phi.value() - phi_ref < -M_PI) {
      phi.set_value(phi.value() + 2. * M_PI);
    }

    return phi;
  }

  void get_phi_of_point(topology::experimental_point input_p, topology::experimental_point *p,
                        double *angle);

  bool is_less_than__optimist(const topology::experimental_helix a, double nsigma) const;

  bool is_more_than__optimist(const topology::experimental_helix a, double nsigma) const;

  bool isnan() const;

  bool isinf() const;

  experimental_vector direction_at(const experimental_point &ep) const;
};

}  // namespace topology

}  // namespace SULTAN

#endif
