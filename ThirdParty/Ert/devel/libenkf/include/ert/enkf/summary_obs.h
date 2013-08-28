/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_obs.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SUMMARY_OBS_H__
#define __SUMMARY_OBS_H__

#ifdef __cplusplus 
extern "C" {
#endif
#include <stdbool.h>

#include <ert/sched/history.h>

#include <ert/config/conf.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/summary_config.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/active_list.h>


#define AUTO_CORRF_EXP     "EXP"
#define AUTO_CORRF_GAUSS   "GAUSS"


typedef struct summary_obs_struct summary_obs_type;

typedef double (auto_corrf_ftype) ( double , double );


void summary_obs_free(
  summary_obs_type * summary_obs);

summary_obs_type * summary_obs_alloc(
  const char   * summary_key,
  const char * obs_key , 
  double  value ,
  double  std,
  const char * auto_corrf_name , 
  double auto_corrf_param);


double summary_obs_get_value( const summary_obs_type * summary_obs );
double summary_obs_get_std( const summary_obs_type * summary_obs );

auto_corrf_ftype * summary_obs_get_auto_corrf( const summary_obs_type * summary_obs );
double             summary_obs_get_auto_corrf_param( const summary_obs_type * summary_obs );

bool summary_obs_default_used(
  const summary_obs_type * summary_obs,
  int                      restart_nr);

const char * summary_obs_get_summary_key(
  const summary_obs_type * summary_obs);

summary_obs_type * summary_obs_alloc_from_HISTORY_OBSERVATION(
  const conf_instance_type * conf_instance,
  const history_type       * history);

summary_obs_type * summary_obs_alloc_from_SUMMARY_OBSERVATION(
  const conf_instance_type * conf_instance,
  const history_type       * history);

void summary_obs_set(summary_obs_type * , double , double );

void summary_obs_scale_std(summary_obs_type * summary_obs, double std_multiplier );
void summary_obs_scale_std__(void * summary_obs, double std_multiplier );

VOID_FREE_HEADER(summary_obs);
VOID_GET_OBS_HEADER(summary_obs);
VOID_MEASURE_HEADER(summary_obs);
UTIL_IS_INSTANCE_HEADER(summary_obs);
VOID_USER_GET_OBS_HEADER(summary_obs);
VOID_CHI2_HEADER(summary_obs);

#ifdef __cplusplus
}
#endif
#endif
