/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_analysis.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __ENKF_ANALYSIS_H__
#define __ENKF_ANALYSIS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <ert/util/matrix.h>
#include <ert/util/int_vector.h>

#include <ert/enkf/obs_data.h>



void          enkf_analysis_fprintf_obs_summary(const obs_data_type * obs_data , 
                                                const meas_data_type * meas_data  , 
                                                const int_vector_type * step_list , 
                                                const char * ministep_name ,
                                                FILE * stream );
  
void          enkf_analysis_deactivate_outliers(obs_data_type * obs_data , 
                                                meas_data_type * meas_data , 
                                                double std_cutoff , 
                                                double alpha);



#ifdef __cplusplus
}
#endif

#endif
