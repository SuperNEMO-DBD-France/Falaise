// snemo/digitization/signal_to_calo_tp_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// This project :
#include <snemo/digitization/clock_utils.h>

// Ourselves:
#include <snemo/digitization/signal_to_calo_tp_algo.h>

namespace snemo {

  namespace digitization {

    signal_to_calo_tp_algo::signal_to_calo_tp_algo()
    {
      _initialized_ = false;
      _electronic_mapping_ = 0;
      _clocktick_ref_ = clock_utils::INVALID_CLOCKTICK;
      datatools::invalidate(_clocktick_shift_);
      return;
    }

    signal_to_calo_tp_algo::~signal_to_calo_tp_algo()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void signal_to_calo_tp_algo::initialize(electronic_mapping & my_electronic_mapping_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "SD to calo tp algorithm is already initialized ! ");
      _electronic_mapping_ = & my_electronic_mapping_;
      _initialized_ = true;
      return;
    }

    bool signal_to_calo_tp_algo::is_initialized() const
    {
      return _initialized_;
    }

    void signal_to_calo_tp_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo tp algorithm is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _electronic_mapping_ = 0;
      _clocktick_ref_ = clock_utils::INVALID_CLOCKTICK;
      datatools::invalidate(_clocktick_shift_);
      return;
    }

    void signal_to_calo_tp_algo::set_clocktick_reference(uint32_t clocktick_ref_)
    {
      _clocktick_ref_ = clocktick_ref_;
      return;
    }

    void signal_to_calo_tp_algo::set_clocktick_shift(double clocktick_shift_)
    {
      _clocktick_shift_ = clocktick_shift_;
      return;
    }

    void signal_to_calo_tp_algo::process(const signal_data & signal_data_,
					 calo_tp_data & my_calo_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo TP algorithm is not initialized ! ");
      _process(signal_data_, my_calo_tp_data_);
      return;
    }

    void signal_to_calo_tp_algo::_process(const signal_data & signal_data_,
					  calo_tp_data & my_calo_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo TP algorithm is not initialized ! ");

      std::size_t number_of_hits = signal_data_.get_calo_signals().size();

      for (std::size_t i = 0; i < number_of_hits; i++)
	{
	  const calo_signal & a_calo_signal  = signal_data_.get_calo_signals()[i].get();
	  const geomtools::geom_id & geom_id = a_calo_signal.get_geom_id();
	  const double calo_hit_amplitude    = a_calo_signal.get_amplitude();

	  if (calo_hit_amplitude >= calo_tp::LOW_THRESHOLD)
	    {
	      geomtools::geom_id temporary_electronic_id;
	      _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE,
						       geom_id,
						       temporary_electronic_id);
	      uint32_t electronic_type = temporary_electronic_id.get_type();
	      geomtools::geom_id electronic_id;
	      electronic_id.set_depth(mapping::BOARD_DEPTH);
	      electronic_id.set_type(electronic_type);
	      temporary_electronic_id.extract_to(electronic_id);

	      // These bits have to be checked
	      bool calo_xt_bit    = 0;
	      bool calo_spare_bit = 0;

	      bool existing = false;
	      unsigned int existing_index = 0;

	      uint32_t a_calo_signal_clocktick = _clocktick_ref_ + clock_utils::CALO_FEB_SHIFT_CLOCKTICK_NUMBER;

	      // Compute calo signal CT25
	      if (a_calo_signal.get_signal_time() > 25) // nanseconds
		{
		  a_calo_signal_clocktick += static_cast<uint32_t>(a_calo_signal.get_signal_time()) / 25;
		}

	      for (unsigned int j = 0; j < my_calo_tp_data_.get_calo_tps().size(); j++)
	      	{
	      	  if (my_calo_tp_data_.get_calo_tps()[j].get().get_geom_id() == electronic_id
	      	      && my_calo_tp_data_.get_calo_tps()[j].get().get_clocktick_25ns() == a_calo_signal_clocktick )
	      	    {
	      	      existing = true;
	      	      existing_index = j;
	      	    }
	      	}

	      if (existing == false)
		{
		  // Create new calo TP
		  snemo::digitization::calo_tp & calo_tp = my_calo_tp_data_.add();
		  calo_tp.set_header(a_calo_signal.get_hit_id(),
				     electronic_id,
				     a_calo_signal_clocktick);
		  calo_tp.set_data(calo_hit_amplitude,
				   calo_xt_bit,
				   calo_spare_bit);
		  // calo_tp.tree_dump(std::clog, "Calo TP first creation : ", "INFO : ");
		}

	      else
		{
		  // Update existing calo TP
		  snemo::digitization::calo_tp & existing_calo_tp = my_calo_tp_data_.grab_calo_tps()[existing_index].grab();
		  existing_calo_tp.update_data(calo_hit_amplitude,
					       calo_xt_bit,
					       calo_spare_bit);
		  // existing_calo_tp.tree_dump(std::clog, "Calo TP Update : ", "INFO : ");
		}
	    }
	}
      return;
    }



  } // end of namespace digitization

} // end of namespace snemo
