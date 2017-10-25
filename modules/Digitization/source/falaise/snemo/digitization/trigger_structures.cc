// snemo/digitization/trigger_structures.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Francois MAUGER <mauger@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Ourselves:
#include <snemo/digitization/trigger_structures.h>

#include <snemo/digitization/clock_utils.h>

namespace snemo {

  namespace digitization {

   trigger_structures::calo_record::calo_record()
    {
      calo_record::reset();
      return;
    }

    void trigger_structures::calo_record::reset()
    {
      clocktick_25ns = clock_utils::INVALID_CLOCKTICK;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  zoning_word[iside].reset();
	  zoning_word[iside].reset();
	}
      total_multiplicity_side_0.reset();
      total_multiplicity_side_1.reset();
      LTO_side_0 = false;
      LTO_side_1 = false;
      total_multiplicity_gveto.reset();
      LTO_gveto = false;
      xt_info_bitset.reset();
      return;
    }

    void trigger_structures::calo_record::display(std::ostream & out_) const
    {
      out_ << "Calo Trigger info record : " << std::endl;
      out_ << "CT |XTS|L|HG|L|L|H1|H0| ZONING S1| ZONING S0 " << std::endl;
      out_ << clocktick_25ns << ' ';
      out_ << xt_info_bitset << ' ';
      out_ << LTO_gveto << ' ';
      out_ << total_multiplicity_gveto << ' ';
      out_ << LTO_side_1 << ' ';
      out_ << LTO_side_0 << ' ';
      out_ << total_multiplicity_side_1 << ' ';
      out_ << total_multiplicity_side_0 << ' ';

      for (unsigned int iside = trigger_info::NSIDES-1; iside != (unsigned)0 - 1; iside--)
      	{
      	  for (unsigned int izone = trigger_info::NZONES-1; izone != (unsigned)0 - 1 ; izone--)
      	    {
      	      out_ << zoning_word[iside][izone];
      	    }
      	  out_ << ' ';
      	}
      out_ << std::endl << std::endl;
      return;
    }

    trigger_structures::calo_summary_record::calo_summary_record()
    {
      calo_summary_record::reset();
      return;
    }

    void trigger_structures::calo_summary_record::reset()
    {
      calo_record::reset();
      single_side_coinc = false;
      total_multiplicity_threshold = false;
      calo_finale_decision = false;
      return;
    }

    void trigger_structures::calo_summary_record::reset_summary_boolean_only()
    {
      single_side_coinc = false;
      total_multiplicity_threshold = false;
      calo_finale_decision = false;
      return;
    }

    void trigger_structures::calo_summary_record::display(std::ostream & out_) const
    {
      calo_record::display(out_);
      out_ << "Single Side coinc           : " << single_side_coinc << std::endl;
      out_ << "Threshold total mult        : " << total_multiplicity_threshold << std::endl;
      out_ << "Calo trigger final decision : " << calo_finale_decision  << std::endl;
      out_ << std::endl;
      return;
    }

    bool trigger_structures::calo_summary_record::is_empty() const
    {
      if (zoning_word[0].any() || zoning_word[1].any()
	  || total_multiplicity_side_0.any() || total_multiplicity_side_1.any() || total_multiplicity_gveto.any()
	  || LTO_side_0 || LTO_side_1 || LTO_gveto || xt_info_bitset.any())
	{
	  return false;
	}

      else return true;
    }


    trigger_structures::tracker_record::tracker_record()
    {
      tracker_record::reset();
      return;
    }

    void trigger_structures::tracker_record::reset()
    {
      clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      finale_data_per_zone[iside][izone].reset();
	    }
	  zoning_word_pattern[iside].reset();
	  zoning_word_near_source[iside].reset();
	}
      single_side_coinc = false;
      finale_decision = false;
      return;
    }

    bool trigger_structures::tracker_record::is_empty() const
    {
      for (unsigned int i = 0; i < trigger_info::NSIDES; i++)
	{
	  for (unsigned int j = 0; j < trigger_info::NZONES; j++)
	    {
	      if (finale_data_per_zone[i][j].any()) return false;
	    }
	  if (zoning_word_pattern[i].any() || zoning_word_near_source[i].any() || single_side_coinc || finale_decision) return false;
	}
      return true;
    }

    void trigger_structures::tracker_record::display(std::ostream & out_)
    {
      out_ << "Tracker Trigger info record : " << std::endl;
      out_ << "Clocktick 1600    : " << clocktick_1600ns << std::endl;;

      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "Side = " << iside << " | ";
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      out_ << "[" << finale_data_per_zone[iside][izone] << "] ";
	    } // end of izone
	  out_ << std::endl;
	}

      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW pattern     : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < zoning_word_pattern[0].size(); ibit++)
	    {
	      out_ << zoning_word_pattern[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW near source : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < zoning_word_near_source[0].size(); ibit++)
	    {
	      out_ << zoning_word_near_source[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      out_ << "Level one tracker decision : [" << finale_decision << "]" <<  std::endl << std::endl;
      return;
    }

    trigger_structures::geiger_matrix::geiger_matrix()
    {
      for (unsigned int iside = 0; iside <trigger_info::NSIDES; iside++)
	{
	  for (unsigned int jlayer = 0; jlayer < trigger_info::NLAYERS; jlayer++)
	    {
	      for (unsigned int krow = 0; krow < trigger_info::NROWS; krow++)
		{
		  matrix[iside][jlayer][krow] = false;
		} // end of krow
	    } // end of jlayer
	} // end of iside
      clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
    }

    void trigger_structures::geiger_matrix::display(std::ostream & out_) const
    {
      out_ << " CLOCKTICK 1600 ns = " << clocktick_1600ns << std::endl;
      out_ << "  |-Zone-0-|---Zone-1--|---Zone-2--|---Zone-3--|---Zone-4--|--Zone-5--|---Zone-6--|---Zone-7--|--Zone-8---|--Zone-9-|" << std::endl;

      for (unsigned int i = 0; i < trigger_info::NSIDES; i++)
	{
	  if (i == 0)
	    {
	      for (unsigned int j = trigger_info::NLAYERS - 1; j != (unsigned) 0 - 1; j--) // Value NLAYERS = 9
		{
		  out_ << j << ' ';
		  for (unsigned int k = 0; k < trigger_info::NROWS; k++)
		    {
		      if( k == 0 )        out_<<"|";

		      if (matrix[i][j][k] ) out_ << "*";

		      if(!matrix[i][j][k])  out_ << ".";

		      if( k == 112)     out_<<"|";

		    } // end of row loop
		  out_<<std::endl;

		  if (j == 0)
		    {
		      out_ << "  |_________________________________________________________________________________________________________________|" << std::endl;
		    }

		} // end of layer loop

	    } // end of if == 0

	  if (i == 1)
	    {
	      for (unsigned int j = 0; j < trigger_info::NLAYERS; j++)
		{
		  out_ << j << ' ' ;
		  for (unsigned int k = 0; k < trigger_info::NROWS; k++)
		    {
		      if( k == 0 )        out_<<"|";

		      if (matrix[i][j][k] ) out_ << "*";

		      if(!matrix[i][j][k])  out_ << ".";

		      if( k == 112)     out_<<"|";

		    } // end of row loop
		  out_<<std::endl;

		} // end of layer loop

	    } // end of if i==1

	} // end of side loop

      out_ << "  |-0-1-2-3-4-5-6-7-8-9-1-2-3-4-5-6-7-8-9-0-1-2-3-4-5-6-7-89-1-2-3-4-5-6-7-8-9-0-1-2-3-4-5-6-7-8-9-1-2-3-4-5-6-7-8-9| Board IDs " << std::endl;

      out_ << "  |-Zone-0-|---Zone-1--|---Zone-2--|---Zone-3--|---Zone-4--|--Zone-5--|---Zone-6--|---Zone-7--|--Zone-8---|--Zone-9-|" << std::endl;
      out_ << "  |                                     |                                    |                                      |" << std::endl;
      out_ << "  |---------------Crate-0---------------|--------------Crate-1---------------|---------------Crate-2----------------|" << std::endl;
      out_ << "  |                                     |                                    |                                      |" << std::endl;
      out_ << std::endl;

      return;
    }

    void trigger_structures::geiger_matrix::reset()
    {
      clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  for (unsigned int jlayer = 0; jlayer < trigger_info::NLAYERS; jlayer++)
	    {
	      for (unsigned int krow = 0; krow < trigger_info::NROWS; krow++)
		{
		  matrix[iside][jlayer][krow] = false;
		} // end of krow
	    } // end of jlayer
	} // end of iside

      return;
    }

    bool trigger_structures::geiger_matrix::is_empty() const
    {
      bool empty = true;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  for (unsigned int jlayer = 0; jlayer < trigger_info::NLAYERS; jlayer++)
	    {
	      for (unsigned int krow = 0; krow < trigger_info::NROWS; krow++)
		{
		  if (matrix[iside][jlayer][krow] == true) empty = false;
		} // end of krow
	    } // end of jlayer
	} // end of iside
      return empty;
    }

    trigger_structures::coincidence_base_record::coincidence_base_record()
    {
      coincidence_base_record::reset();
      return;
    }

    void trigger_structures::coincidence_base_record::reset()
    {
      calo_zoning_word[0].reset();
      calo_zoning_word[1].reset();
      total_multiplicity_side_0.reset();
      total_multiplicity_side_1.reset();
      LTO_side_0 = false;
      LTO_side_1 = false;
      total_multiplicity_gveto.reset();
      LTO_gveto = false;
      xt_info_bitset.reset();
      single_side_coinc = true;
      total_multiplicity_threshold = false;
      decision = false;
      return;
    }

    void trigger_structures::coincidence_base_record::display(std::ostream & out_) const
    {
      out_ << "XTS|L|HG|L|L|H1|H0| ZONING S1| ZONING S0 " << std::endl;
      out_ << xt_info_bitset << ' ';
      out_ << LTO_gveto << ' ';
      out_ << total_multiplicity_gveto << ' ';
      out_ << LTO_side_1 << ' ';
      out_ << LTO_side_0 << ' ';
      out_ << total_multiplicity_side_1 << ' ';
      out_ << total_multiplicity_side_0 << ' ';
      for (unsigned int iside = trigger_info::NSIDES-1; iside != (unsigned)0-1; iside--)
      	{
      	  for (unsigned int izone = trigger_info::NZONES-1; izone != (unsigned)0-1 ; izone--)
      	    {
      	      out_ << calo_zoning_word[iside][izone];
      	    }
      	  out_ << ' ';
      	}
      out_ << std::endl;
      out_ << "Single Side coinc : " << single_side_coinc
		<< "  |  Threshold total mult : "   << total_multiplicity_threshold << std::endl;
      return;
    }

    trigger_structures::coincidence_calo_record::coincidence_calo_record()
    {
      coincidence_calo_record::reset();
      return;
    }

    void trigger_structures::coincidence_calo_record::reset()
    {
      coincidence_base_record::reset();
      clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
      return;
    }

    void trigger_structures::coincidence_calo_record::display(std::ostream & out_) const
    {
      out_ << "************************************************************************************" << std::endl;
      out_ << "*************************** Coincidence calo record ********************" << std::endl;
      out_ << "*************************** Clocktick 1600 = " << clocktick_1600ns << " ***************************" << std::endl;
      coincidence_base_record::display(out_);
      out_ << "Coincidence calo record decision : [" << decision << "]" << std::endl;
      out_ << std::endl;
      return;
    }

    bool trigger_structures::coincidence_calo_record::is_empty() const
    {
      if (calo_zoning_word[0].any() || calo_zoning_word[1].any()
	  || total_multiplicity_side_0.any() || total_multiplicity_side_1.any() || total_multiplicity_gveto.any()
	  || LTO_side_0 || LTO_side_1 || LTO_gveto || xt_info_bitset.any() || total_multiplicity_threshold || decision)
	{
	  return false;
	}

      else return true;
    }

    trigger_structures::coincidence_event_record::coincidence_event_record()
    {
      coincidence_event_record::reset();
      return;
    }

    void trigger_structures::coincidence_event_record::reset()
    {
      coincidence_base_record::reset();
      clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
      trigger_mode = INVALID;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      tracker_finale_data_per_zone[iside][izone].reset();
	    }

	  coincidence_zoning_word[iside].reset();
	  tracker_zoning_word_pattern[iside].reset();
	  tracker_zoning_word_near_source[iside].reset();
	}
      return;
    }

    void trigger_structures::coincidence_event_record::display(std::ostream & out_) const
    {
      out_ << "************************************************************************************" << std::endl;
      out_ << "*************************** Coincidence event record ********************" << std::endl;
      out_ << "*************************** Clocktick 1600 = " << clocktick_1600ns << " ***************************" << std::endl;

      coincidence_base_record::display(out_);
      out_ << "Bitset : [NSZL NSZR L M R O I] " << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "Side = " << iside << " | ";
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      out_ << "[" << tracker_finale_data_per_zone[iside][izone] << "] ";
	    } // end of izone
	  out_ << std::endl;
	}
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW coincidence : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < coincidence_zoning_word[0].size(); ibit++)
	    {
	      out_ << coincidence_zoning_word[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW pattern     : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < tracker_zoning_word_pattern[0].size(); ibit++)
	    {
	      out_ << tracker_zoning_word_pattern[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW near source : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < tracker_zoning_word_near_source[0].size(); ibit++)
	    {
	      out_ << tracker_zoning_word_near_source[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      out_ << "Coincidence trigger mode : [" << trigger_mode << "]" << std::endl;
      out_ << "Coincidence event record decision : [" << decision << "]" << std::endl;
      return;
    }

    bool trigger_structures::coincidence_event_record::is_empty() const
    {
      if (calo_zoning_word[0].any() || calo_zoning_word[1].any()
	  || total_multiplicity_side_0.any() || total_multiplicity_side_1.any() || total_multiplicity_gveto.any()
	  || LTO_side_0 || LTO_side_1 || LTO_gveto || xt_info_bitset.any() || total_multiplicity_threshold || decision)
	{
	  return false;
	}
      else return true;

      for (unsigned int i = 0; i < trigger_info::NSIDES; i++)
	{
	  for (unsigned int j = 0; j < trigger_info::NZONES; j++)
	    {
	      if (tracker_finale_data_per_zone[i][j].any()) return false;
	    }
	  if (coincidence_zoning_word[i].any() || tracker_zoning_word_pattern[i].any() || tracker_zoning_word_near_source[i].any() || single_side_coinc || trigger_mode != INVALID) return false;
	}

      return true;
    }

    trigger_structures::L2_decision::L2_decision()
    {
      L2_decision::reset();
      return;
    }

    void trigger_structures::L2_decision::reset()
    {
      L2_decision_bool = false;
      L2_ct_decision = clock_utils::INVALID_CLOCKTICK;
      L2_trigger_mode = INVALID;
    }

    void trigger_structures::L2_decision::display(std::ostream & out_) const
    {
      out_ << "Display L2 decision @ 1600 ns" << std::endl;
      out_ << "Decision clocktick 1600 ns =  " << L2_ct_decision << std::endl;
      out_ << "L2 Trigger mode            = [" << L2_trigger_mode << "]" << std::endl;
      out_ << "L2 decision                = [" << L2_decision_bool << "]" << std::endl << std::endl;
      return;
    }

    trigger_structures::L1_calo_decision::L1_calo_decision()
    {
      L1_calo_decision::reset();
      return;
    }

    void trigger_structures::L1_calo_decision::reset()
    {
      L1_calo_decision_bool = false;
      L1_calo_ct_decision = clock_utils::INVALID_CLOCKTICK;
    }

    void trigger_structures::L1_calo_decision::display(std::ostream & out_) const
    {
      out_ << "Display L1 calo decision @ 25 ns" << std::endl;
      out_ << "Decision clocktick 25 ns =  " << L1_calo_ct_decision << std::endl;
      out_ << "L1 calo decision         = [" << L1_calo_decision_bool << "]" << std::endl << std::endl;
      return;
    }

    trigger_structures::L1_tracker_decision::L1_tracker_decision()
    {
      L1_tracker_decision::reset();
      return;
    }

    void trigger_structures::L1_tracker_decision::reset()
    {
      L1_tracker_decision_bool = false;
      L1_tracker_ct_decision = clock_utils::INVALID_CLOCKTICK;
    }

    void trigger_structures::L1_tracker_decision::display(std::ostream & out_) const
    {
      out_ << "Display L1 tracker decision @ 1600 ns" << std::endl;
      out_ << "Decision clocktick 1600 ns  =  " << L1_tracker_ct_decision << std::endl;
      out_ << "L1 tracker decision         = [" << L1_tracker_decision_bool << "]" << std::endl << std::endl;
      return;
    }

    trigger_structures::previous_event_record::previous_event_record()
    {
      previous_event_record::reset();
      return;
    }
    // CARE OF CHANGES :
    void trigger_structures::previous_event_record::reset()
    {
      previous_clocktick_1600ns = clock_utils::INVALID_CLOCKTICK;
      counter_1600ns = 0;
      calo_zoning_word[0].reset();
      calo_zoning_word[1].reset();
      total_multiplicity_side_0.reset();
      total_multiplicity_side_1.reset();
      LTO_side_0 = false;
      LTO_side_1 = false;
      total_multiplicity_gveto.reset();
      LTO_gveto = false;
      xt_info_bitset.reset();
      single_side_coinc = true;
      total_multiplicity_threshold = false;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      tracker_finale_data_per_zone[iside][izone].reset();
	    }
	  coincidence_zoning_word[iside].reset();
	  tracker_zoning_word_pattern[iside].reset();
	  tracker_zoning_word_near_source[iside].reset();
	}

      return;
    }

    void trigger_structures::previous_event_record::display(std::ostream & out_) const
    {
      out_ << "************************************************************************************" << std::endl;
      out_ << "*************************** Previous event record ********************" << std::endl;
      out_ << "*************************** Previous clocktick 1600 = " << previous_clocktick_1600ns << " ********************" << std::endl;
      out_ << "*************************** Counter 1600 = " << counter_1600ns << " ***************************" << std::endl;
      coincidence_base_record::display(out_);
      out_ << "Bitset : [NSZL NSZR L M R O I] " << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "Side = " << iside << " | ";
	  for (unsigned int izone = 0; izone < trigger_info::NZONES; izone++)
	    {
	      out_ << "[" << tracker_finale_data_per_zone[iside][izone] << "] ";
	    } // end of izone
	  out_ << std::endl;
	}

      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW coincidence : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < coincidence_zoning_word[0].size(); ibit++)
	    {
	      out_ << coincidence_zoning_word[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW pattern     : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < tracker_zoning_word_pattern[0].size(); ibit++)
	    {
	      out_ << tracker_zoning_word_pattern[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      for (unsigned int iside = 0; iside < trigger_info::NSIDES; iside++)
	{
	  out_ << "ZW near source : S" << iside << " : [";
	  for (unsigned int ibit = 0; ibit < tracker_zoning_word_near_source[0].size(); ibit++)
	    {
	      out_ << tracker_zoning_word_near_source[iside][ibit];
	    }
	  out_ << "] ";
	}
      out_ << std::endl;
      out_ << "Coincidence trigger mode : [" << trigger_mode << "]" << std::endl;
      out_ << "Coincidence event record decision : [" << decision << "]" << std::endl;

      out_ << std::endl;
      return;
    }

  } // end of namespace digitization

} // end of namespace snemo
