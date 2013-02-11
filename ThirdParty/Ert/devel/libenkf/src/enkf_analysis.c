/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_analysis.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_lapack.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/timer.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/enkf_linalg.h>

#include <ert/enkf/meas_data.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/analysis_config.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_analysis.h>






/*****************************************************************/

void enkf_analysis_fprintf_obs_summary(const obs_data_type * obs_data , const meas_data_type * meas_data , const int_vector_type * step_list , const char * ministep_name , FILE * stream ) {
  const char * float_fmt = "%15.3f";
  fprintf(stream , "===============================================================================================================================\n");
  fprintf(stream , "Report step...: %04d",int_vector_iget( step_list , 0));
  if (int_vector_size( step_list ) == 1)
    fprintf(stream , "\n");
  else
    fprintf(stream , " - %04d \n",int_vector_get_last( step_list ));
  
  
  fprintf(stream , "Ministep......: %s   \n",ministep_name);  
  fprintf(stream , "-------------------------------------------------------------------------------------------------------------------------------\n");
  {
    char * obs_fmt = util_alloc_sprintf("  %%-3d : %%-32s %s +/-  %s" , float_fmt , float_fmt);
    char * sim_fmt = util_alloc_sprintf("   %s +/- %s  \n"            , float_fmt , float_fmt);

    fprintf(stream , "                                                         Observed history               |             Simulated data        \n");  
    fprintf(stream , "-------------------------------------------------------------------------------------------------------------------------------\n");
    
    {
      int block_nr;
      int obs_count = 1;  /* Only for printing */
      for (block_nr =0; block_nr < obs_data_get_num_blocks( obs_data ); block_nr++) {
        const obs_block_type  * obs_block  = obs_data_iget_block_const( obs_data , block_nr);
        const meas_block_type * meas_block = meas_data_iget_block_const( meas_data , block_nr );
        const char * obs_key = obs_block_get_key( obs_block );
        
        for (int iobs = 0; iobs < obs_block_get_size( obs_block ); iobs++) {
          const char * print_key;
          if (iobs == 0)
            print_key = obs_key;
          else
            print_key = "  ...";
          
          fprintf(stream , obs_fmt , obs_count , print_key , obs_block_iget_value( obs_block , iobs ) , obs_block_iget_std( obs_block , iobs ));
          {
            active_type active_mode = obs_block_iget_active_mode( obs_block , iobs );
            if (active_mode == ACTIVE)
              fprintf(stream , "  Active   |");
            else if (active_mode == DEACTIVATED)
              fprintf(stream , "  Inactive |");
            else if (active_mode == MISSING)
              fprintf(stream , "           |");
            else
              util_abort("%s: enum_value:%d not handled - internal error\n" , __func__ , active_mode);
            if (active_mode == MISSING)
              fprintf(stream , "                  Missing\n");
            else
              fprintf(stream , sim_fmt, meas_block_iget_ens_mean( meas_block , iobs ) , meas_block_iget_ens_std( meas_block , iobs ));
          }
          obs_count++;
        }
      }
    }
    
    free( obs_fmt );
    free( sim_fmt );
  } 
  fprintf(stream , "===============================================================================================================================\n");
  fprintf(stream , "\n\n\n");
}




void enkf_analysis_deactivate_outliers(obs_data_type * obs_data , meas_data_type * meas_data , double std_cutoff , double alpha) {
  for (int block_nr =0; block_nr < obs_data_get_num_blocks( obs_data ); block_nr++) {
    obs_block_type  * obs_block  = obs_data_iget_block( obs_data , block_nr);
    meas_block_type * meas_block = meas_data_iget_block( meas_data , block_nr );
    
    meas_block_calculate_ens_stats( meas_block );
    {
      int iobs;
      for (iobs =0; iobs < meas_block_get_total_size( meas_block ); iobs++) {
        if (meas_block_iget_active( meas_block , iobs )) {
          double ens_std  = meas_block_iget_ens_std( meas_block , iobs );
          if (ens_std < std_cutoff) {
            /*
              De activated because the ensemble has to small
              variation for this particular measurement.
            */
            obs_block_deactivate( obs_block , iobs , "No ensemble variation");
            meas_block_deactivate( meas_block , iobs );
          } else {
            double ens_mean  = meas_block_iget_ens_mean( meas_block , iobs );
            double obs_std   = obs_block_iget_std( obs_block , iobs );
            double obs_value = obs_block_iget_value( obs_block , iobs );
            double innov     = obs_value - ens_mean;
            
            /* 
               Deactivated because the distance between the observed data
               and the ensemble prediction is to large. Keeping these
               outliers will lead to numerical problems.
            */

            if (fabs( innov ) > alpha * (ens_std + obs_std)) {
              obs_block_deactivate(obs_block , iobs , "No overlap");
              meas_block_deactivate(meas_block , iobs);
            }
          }
        }
      }
    }
  }
}










