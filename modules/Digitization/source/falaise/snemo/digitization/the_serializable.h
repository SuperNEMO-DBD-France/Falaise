/// \file falaise/snemo/digitization/the_serializable.h

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_THE_SERIALIZABLE_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_THE_SERIALIZABLE_H

// Third party:
// - Boost :
#include <boost/serialization/export.hpp>
// - Bayeux/datatools :
#include <datatools/archives_instantiation.h>

// This project:
// Include the specific serialization code of serializable classes:
#include <snemo/digitization/calo_signal.ipp>
#include <snemo/digitization/geiger_signal.ipp>
#include <snemo/digitization/signal_data.ipp>
#include <snemo/digitization/calo_tp.ipp>
#include <snemo/digitization/calo_tp_data.ipp>
#include <snemo/digitization/geiger_tp.ipp>
#include <snemo/digitization/geiger_tp_data.ipp>
#include <snemo/digitization/calo_ctw.ipp>
#include <snemo/digitization/calo_ctw_data.ipp>
#include <snemo/digitization/geiger_ctw.ipp>
#include <snemo/digitization/geiger_ctw_data.ipp>

// Force instantiation of template code associated to the classes above
// for the Boost archive classes defined in Bayeux/datatools:
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::calo_signal)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::geiger_signal)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::signal_data)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::calo_tp)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::calo_tp_data)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::calo_ctw)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::calo_ctw_data)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::geiger_tp)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::geiger_tp_data)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::geiger_ctw)
DATATOOLS_SERIALIZATION_CLASS_SERIALIZE_INSTANTIATE_ALL(snemo::digitization::geiger_ctw_data)


// Special implentation macro to register the class through the
// Boost/Serialization system :
BOOST_CLASS_EXPORT_IMPLEMENT(snemo::digitization::signal_data)
BOOST_CLASS_EXPORT_IMPLEMENT(snemo::digitization::calo_tp_data)
BOOST_CLASS_EXPORT_IMPLEMENT(snemo::digitization::calo_ctw_data)
BOOST_CLASS_EXPORT_IMPLEMENT(snemo::digitization::geiger_tp_data)
BOOST_CLASS_EXPORT_IMPLEMENT(snemo::digitization::geiger_ctw_data)

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_THE_SERIALIZABLE_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
