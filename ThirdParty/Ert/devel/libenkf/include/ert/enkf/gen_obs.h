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

#ifndef __GEN_OBS_H__
#define __GEN_OBS_H__

#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/obs_data.h>

typedef struct gen_obs_struct gen_obs_type;

gen_obs_type * gen_obs_alloc(gen_data_config_type * config , const char * obs_key , const char * , double , double , const char * , const char * , const char  * );
void           gen_obs_user_get_with_data_index(const gen_obs_type * gen_obs , const char * index_key , double * value , double * std , bool * valid);

VOID_CHI2_HEADER(gen_obs);
UTIL_IS_INSTANCE_HEADER(gen_obs);
VOID_FREE_HEADER(gen_obs);
VOID_GET_OBS_HEADER(gen_obs);
VOID_MEASURE_HEADER(gen_obs);
VOID_USER_GET_OBS_HEADER(gen_obs);

#endif
