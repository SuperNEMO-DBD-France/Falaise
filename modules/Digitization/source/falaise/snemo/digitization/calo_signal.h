// snemo/digitization/calo_signal.h
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_SIGNAL_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_SIGNAL_H

// Standard library :
#include <bitset>

// Third party:
// - Bayeux/geomtools :
#include <bayeux/geomtools/base_hit.h>
// - Bayeux/datatools :
#include <bayeux/datatools/clhep_units.h>
#include <bayeux/datatools/handle.h>

namespace snemo {

  namespace digitization {

    /// \brief The calo tracker signal
    class calo_signal
			: public geomtools::base_hit
    {
    public :

			/// Shift to take account the transit time from the scintillator to the PMT anode
      static constexpr double DELAYED_PM_TIME = 62.35; // nanoseconds, ref : PHD A.Huber p.174

			/// Maximum time to combine 2 calo step hits (to be moved to the ASB module later). Related to calo FEB sampling.
			static constexpr double SIGNAL_MAX_TIME = 400; // nanoseconds, 1024 samples * 0.390625 ps

			struct compare_handle_by_timestamp {

				bool operator()(const datatools::handle<calo_signal> & ha,
												const datatools::handle<calo_signal> & hb)
				{
					return (ha.get().get_signal_time() < hb.get().get_signal_time());
				}

			};

      /// Default constructor
      calo_signal();

      /// Destructor
      virtual ~calo_signal();

			/// Set the header with valid values
			void set_header(int32_t geiger_signal_hit_id_,
											const geomtools::geom_id & electronic_id_);

			/// Set the data with values
			void set_data(const double & signal_time_,
										const double & amplitude_);

      /// Return the const time with the photomultiplier delay
      double get_signal_time() const;

      // Set the time with the photomultiplier delay
      void set_signal_time(const double & signal_time_);

      /// Return the const energy
      double get_amplitude() const;

      // Set the energy
      void set_amplitude(const double & amplitude_);

      /// Check if the internal data of calo signal is valid
      bool is_valid() const;

      /// Reset the internal data of calo signal
      void reset();

      /// Smart print
      virtual void tree_dump(std::ostream      & a_out    = std::clog,
														 const std::string & a_title  = "",
														 const std::string & a_indent = "",
														 bool a_inherit               = false) const;
    private :

			double _signal_time_; //!< Calorimeter signal time
      double _amplitude_;   //!< Amplitude

			DATATOOLS_SERIALIZATION_DECLARATION()

    };

  } // end of namespace digitization

} // end of namespace snemo

#endif /* FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_SIGNAL_H */

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
