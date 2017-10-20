// snemo/digitization/calo_tp.ipp
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_TP_IPP
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_TP_IPP

// Ourselves:
#include <snemo/digitization/calo_tp.h>

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
		void calo_tp::serialize (Archive & ar,
														 const unsigned int /* version_ */)
		{
			// inherit from the 'base_hit' mother class:
			ar & boost::serialization::make_nvp ("geomtools__base_hit",
																					 boost::serialization::base_object<geomtools::base_hit>(*this));

			// the '_store' field member from the 'base_hit' mother class
			// knows about the field to be stored/loaded
			// from the archive.

			if (_store & STORE_CLOCKTICK_25NS)
				{
					ar & boost::serialization::make_nvp ("clocktick_25ns", _clocktick_25ns_);
				}

			if (_store & STORE_TP)
				{
					ar & boost::serialization::make_nvp ("TP", _tp_);
				}

			return;
		}

  } // end of namespace digitization

} // end of namespace snemo

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_TP_IPP

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
