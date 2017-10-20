#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_GEIGER_SIGNAL_IPP
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_GEIGER_SIGNAL_IPP

// Ourselves:
#include <snemo/digitization/geiger_signal.h>

// Third party:
// - Boost:
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/bitset.hpp>
// - Bayeux/geomtools
#include <geomtools/base_hit.ipp>

namespace snemo {

  namespace digitization {

    template<class Archive>
    void geiger_signal::serialize (Archive            & ar,
				   const unsigned int /* version_ */)
    {
      // inherit from the 'base_hit' mother class:
      ar & boost::serialization::make_nvp ("geomtools__base_hit",
					   boost::serialization::base_object<geomtools::base_hit>(*this));

      ar & boost::serialization::make_nvp ("anode_avalanche_time", _anode_avalanche_time_);
      ar & boost::serialization::make_nvp ("plasma_top_time",      _plasma_top_time_);
      ar & boost::serialization::make_nvp ("plasma_bottom_time",   _plasma_bottom_time_);

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_GEIGER_SIGNAL_IPP

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
