/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_obs.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_GEN_OBS_H
#define ERT_GEN_OBS_H

#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/active_list.h>

typedef struct gen_obs_struct gen_obs_type;

gen_obs_type * gen_obs_alloc(gen_data_config_type * config , const char * obs_key , const char * , double , double , const char * , const char * , const char  * );
void           gen_obs_user_get_with_data_index(const gen_obs_type * gen_obs , const char * index_key , double * value , double * std , bool * valid);

void           gen_obs_update_std_scale(gen_obs_type * gen_obs, double std_multiplier , const active_list_type * active_list);
int            gen_obs_get_size(const gen_obs_type * gen_obs);
double         gen_obs_iget_std(const gen_obs_type * gen_obs, int index);
double         gen_obs_iget_value(const gen_obs_type * gen_obs, int index);
double         gen_obs_iget_std_scaling(const gen_obs_type * gen_obs, int index);
int            gen_obs_get_obs_index(const gen_obs_type * gen_obs, int index);
void           gen_obs_load_observation(gen_obs_type * gen_obs, const char * obs_file);
void           gen_obs_set_scalar( gen_obs_type * gen_obs , double scalar_value , double scalar_std);
void 	       gen_obs_attach_data_index( gen_obs_type * gen_obs , const int_vector_type * data_index );
void 	       gen_obs_load_data_index( gen_obs_type * obs , const char * data_index_file);
void 	       gen_obs_parse_data_index( gen_obs_type * obs , const char * data_index_string);




VOID_CHI2_HEADER(gen_obs);
UTIL_IS_INSTANCE_HEADER(gen_obs);
VOID_FREE_HEADER(gen_obs);
VOID_GET_OBS_HEADER(gen_obs);
VOID_MEASURE_HEADER(gen_obs);
VOID_USER_GET_OBS_HEADER(gen_obs);
VOID_UPDATE_STD_SCALE_HEADER(gen_obs);

#endif
