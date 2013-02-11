/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'block_obs.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __BLOCK_OBS_H__
#define __BLOCK_OBS_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/sched/history.h>

#include <ert/config/conf.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/field.h>
#include <ert/enkf/active_list.h>



typedef struct block_obs_struct block_obs_type;

  typedef enum {
    SOURCE_FIELD = 10,
    SOURCE_SUMMARY = 12
  } block_obs_source_type;


  block_obs_type * block_obs_alloc(const char   * obs_label,
                                   block_obs_source_type source_type , 
                                   const stringlist_type * summary_keys , 
                                   const void * data_config , 
                                   const ecl_grid_type * grid , 
                                   int            size,
                                   const int    * i,
                                   const int    * j,
                                   const int    * k,
                                   const double * obs_value,
                                   const double * obs_std);
  
void block_obs_free(
  block_obs_type * block_obs);


block_obs_type * block_obs_alloc_from_BLOCK_OBSERVATION(const conf_instance_type * conf_instance, const history_type  * history);


int         block_obs_iget_i(const block_obs_type * , int index);
int         block_obs_iget_j(const block_obs_type * , int index);
int         block_obs_iget_k(const block_obs_type * , int index);
int         block_obs_get_size(const block_obs_type * );
void        block_obs_iget(const block_obs_type * block_obs, int  , double * , double * );
void        block_obs_iget_ijk(const block_obs_type * block_obs , int block_nr , int * i , int * j , int * k);

VOID_FREE_HEADER(block_obs);
VOID_GET_OBS_HEADER(block_obs);
UTIL_IS_INSTANCE_HEADER(block_obs);
VOID_MEASURE_HEADER(block_obs);
VOID_USER_GET_OBS_HEADER(block_obs);
VOID_CHI2_HEADER(block_obs);

#ifdef __cplusplus
}
#endif
#endif
