#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_DATA_IPP
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_DATA_IPP

// Ourselves:
#include <snemo/digitization/signal_data.h>

// Third party:
// - Boost:
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
// - Bayeux/datatools:
// Datatools support for serializable objects :
#include <datatools/i_serializable.ipp>
#include <datatools/event_id.ipp>

// This project :
#include <snemo/digitization/geiger_signal.ipp>
#include <snemo/digitization/signal_data.ipp>

namespace snemo {

  namespace digitization {

    template<class Archive>
    void signal_data::serialize (Archive            & ar,
				 const unsigned int /* version_ */)
    {
      ar & DATATOOLS_SERIALIZATION_I_SERIALIZABLE_BASE_OBJECT_NVP;
      ar & boost::serialization::make_nvp ("geiger_signals", _geiger_signals_);
      ar & boost::serialization::make_nvp ("calo_signals", _calo_signals_);

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_DATA_IPP

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
