// snemo/digitization/sd_to_calo_signal_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Third party:
// - Bayeux/datatools :
#include <datatools/clhep_units.h>
// Ourselves:
#include <snemo/digitization/sd_to_calo_signal_algo.h>

namespace snemo {

  namespace digitization {

    sd_to_calo_signal_algo::sd_to_calo_signal_algo()
    {
      _initialized_ = false;
      _geo_manager_ = 0;
      return;
    }

    sd_to_calo_signal_algo::sd_to_calo_signal_algo(const geomtools::manager & mgr_)
    {
      _initialized_ = false;
      set_geo_manager(mgr_);
      return;
    }

    sd_to_calo_signal_algo::~sd_to_calo_signal_algo()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void sd_to_calo_signal_algo::initialize()
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "SD to calo signal algorithm is already initialized ! ");
      _initialized_ = true;
      return;
    }

    bool sd_to_calo_signal_algo::is_initialized() const
    {
      return _initialized_;
    }

    void sd_to_calo_signal_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _geo_manager_ = 0;
      return;
    }

    void sd_to_calo_signal_algo::set_geo_manager(const geomtools::manager & mgr_ )
    {
      _geo_manager_ = & mgr_;
      return;
    }

    double sd_to_calo_signal_algo::_convert_energy_to_amplitude(const double energy_)
    {
      // 1 MeV is equivalent to 300 mV
      return energy_ * 300; // maybe units problem for the moment
    }

    void sd_to_calo_signal_algo::_process_by_calo_type(const mctools::simulated_data & sd_,
						       std::string & calo_type_,
						       int32_t     & calo_signal_hit_id_,
						       signal_data & signal_data_)
    {
      const std::size_t number_of_calo_hits = sd_.get_number_of_step_hits(calo_type_);

      for (size_t ihit = 0; ihit < number_of_calo_hits; ihit++) {
	const mctools::base_step_hit & calo_hit = sd_.get_step_hit(calo_type_, ihit);
	const double signal_time    = calo_hit.get_time_stop();
	const double energy_deposit = calo_hit.get_energy_deposit();
	const double amplitude      = _convert_energy_to_amplitude(energy_deposit);
	const geomtools::geom_id & calo_gid = calo_hit.get_geom_id();

	bool calo_already_hit_in_time = false;
	int hit_calo_signal_position = -1;
	for (unsigned int jsig = 0; jsig < signal_data_.get_calo_signals().size(); jsig++)
	  {
	    calo_signal a_calo_signal = signal_data_.grab_calo_signals()[jsig].grab();
	    double a_calo_signal_time_limit = a_calo_signal.get_signal_time() - calo_signal::DELAYED_PM_TIME + calo_signal::SIGNAL_MAX_TIME;
	    if (calo_gid == a_calo_signal.get_geom_id() && signal_time <=  a_calo_signal_time_limit) {
	      calo_already_hit_in_time = true;
	      hit_calo_signal_position = jsig;
	    }
	  }

	if (calo_already_hit_in_time) {
	  // Update calo signal (amplitude only for the moment (will be in ASB module))
	  calo_signal & already_hit_calo_signal = signal_data_.grab_calo_signals()[hit_calo_signal_position].grab();
	  double amplitude_sum = amplitude + already_hit_calo_signal.get_amplitude();
	  already_hit_calo_signal.set_amplitude(amplitude_sum);
	}
	else {
	  // Create new calo signal
	  calo_signal & calo_signal = signal_data_.add_calo_signal();
	  calo_signal.set_header(calo_signal_hit_id_, calo_gid);
	  calo_signal.set_data(signal_time, amplitude);
	  calo_signal.grab_auxiliaries().store("hit.id", calo_hit.get_hit_id());
	  calo_signal_hit_id_++;
	}

      }

      return;
    }

    void sd_to_calo_signal_algo::process(const mctools::simulated_data & sd_,
					 signal_data & signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized ! ");
      _process(sd_, signal_data_);
      return ;
    }

    int sd_to_calo_signal_algo::_process(const mctools::simulated_data & sd_,
					 signal_data & signal_data)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized ! ");
      int32_t calo_signal_hit_id = 0;
      std::string calo_type = "calo";
      if (sd_.has_step_hits(calo_type))
	{
	  _process_by_calo_type(sd_,
				calo_type,
				calo_signal_hit_id,
				signal_data);
	}

      calo_type = "xcalo";
      if (sd_.has_step_hits(calo_type))
	{
	  _process_by_calo_type(sd_,
				calo_type,
				calo_signal_hit_id,
				signal_data);
	}

      calo_type = "gveto";
      if (sd_.has_step_hits(calo_type))
	{
	  _process_by_calo_type(sd_,
				calo_type,
				calo_signal_hit_id,
				signal_data);
	}

      return 0;
    }

  } // end of namespace digitization

} // end of namespace snemo
