/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_obs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/**
   See the overview documentation of the observation system in enkf_obs.c
*/
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <ert/util/util.h>

#include <ert/enkf/summary_obs.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/active_list.h>


#define SUMMARY_OBS_TYPE_ID 66103
#define OBS_SIZE 1

struct summary_obs_struct {
  UTIL_TYPE_ID_DECLARATION;
  char    * summary_key;    /** The observation, in summary.x syntax, e.g. GOPR:FIELD.    */
  char    * obs_key;
  
  double    value;          /** Observation value. */
  double    std;            /** Standard deviation of observation. */

  auto_corrf_ftype   * auto_corrf;
  double               auto_corrf_param;  
};



static double auto_corrf_exp( double tlag , double param ) {
  return exp(-fabs(tlag) / param );
}

static double auto_corrf_gauss( double tlag , double param ) {
  double x = tlag / param;
  return exp(-0.5 * x * x);
}



static auto_corrf_ftype * summary_obs_lookup_auto_corrf( const char * fname ) {
  if (fname == NULL)
    return NULL;
  else {
    if (strcmp( fname , AUTO_CORRF_EXP) == 0)
      return auto_corrf_exp;
    else if (strcmp( fname , AUTO_CORRF_GAUSS) == 0)
      return auto_corrf_gauss;
    else {
      util_abort("%s: correlation function:%s not recognized \n",__func__ , fname);
      return NULL;  /* Compiler shut up. */
    }
  }
}




/**
  This function allocates a summary_obs instance. The summary_key
  string should be of the format used by the summary.x program.
  E.g., WOPR:P4 would condition on WOPR in well P4.

  Observe that this format is currently *not* checked before the actual
  observation time.

  TODO
  Should check summary_key on alloc.
*/
summary_obs_type * summary_obs_alloc(const char   * summary_key,
                                     const char   * obs_key , 
                                     double value ,
                                     double std   ,
                                     const char * auto_corrf_name , 
                                     double auto_corrf_param) {
  
  summary_obs_type * obs = util_malloc(sizeof * obs );
  UTIL_TYPE_ID_INIT( obs , SUMMARY_OBS_TYPE_ID )

  obs->summary_key      = util_alloc_string_copy( summary_key );
  obs->obs_key          = util_alloc_string_copy( obs_key );
  obs->value            = value;
  obs->std              = std;
  obs->auto_corrf       = summary_obs_lookup_auto_corrf( auto_corrf_name );
  obs->auto_corrf_param = auto_corrf_param;
  
  return obs;
}


static UTIL_SAFE_CAST_FUNCTION_CONST(summary_obs   , SUMMARY_OBS_TYPE_ID);
static UTIL_SAFE_CAST_FUNCTION(summary_obs   , SUMMARY_OBS_TYPE_ID);
UTIL_IS_INSTANCE_FUNCTION(summary_obs , SUMMARY_OBS_TYPE_ID);


void summary_obs_free(summary_obs_type * summary_obs) {
  free(summary_obs->summary_key);
  free(summary_obs->obs_key);
  free(summary_obs);
}





auto_corrf_ftype * summary_obs_get_auto_corrf( const summary_obs_type * summary_obs ) {
  return summary_obs->auto_corrf;
}

double summary_obs_get_auto_corrf_param( const summary_obs_type * summary_obs ) {
  return summary_obs->auto_corrf_param;
}





const char * summary_obs_get_summary_key(const summary_obs_type * summary_obs)
{
  return summary_obs->summary_key;
}


/**
   Hardcodes an assumption that the size of summary data|observations
   is always one; i.e. PARTLY_ACTIVE and ALL_ACTIVE are treated in the
   same manner.
*/
void summary_obs_get_observations(const summary_obs_type * summary_obs,
                                  obs_data_type          * obs_data,
                                  int report_step , 
                                  const active_list_type * __active_list) {

  int active_size              = active_list_get_active_size( __active_list , OBS_SIZE );
  if (active_size == 1) {
    obs_block_type * obs_block   = obs_data_add_block( obs_data , summary_obs->obs_key , OBS_SIZE , NULL , false);
    obs_block_iset( obs_block , 0 , summary_obs->value , summary_obs->std );
  }
}



void summary_obs_measure(const summary_obs_type * obs, const summary_type * summary, node_id_type node_id , meas_data_type * meas_data , const active_list_type * __active_list) {
  int active_size = active_list_get_active_size( __active_list , OBS_SIZE );
  if (active_size == 1) {
    meas_block_type * meas_block = meas_data_add_block( meas_data , obs->obs_key , node_id.report_step , active_size );
    meas_block_iset( meas_block , node_id.iens , 0 , summary_get(summary, node_id.report_step , node_id.state));
  }
}

 

double summary_obs_chi2(const summary_obs_type * obs,
                        const summary_type     * summary, 
                        node_id_type node_id) {
  double x = (summary_get(summary , node_id.report_step , node_id.state) - obs->value) / obs->std;
  return x*x;
}



void summary_obs_user_get(const summary_obs_type * summary_obs , const char * index_key , double * value , double * std, bool * valid) {
  *valid = true;
  *value = summary_obs->value;
  *std   = summary_obs->std;
}



double summary_obs_get_value( const summary_obs_type * summary_obs ) {
  return summary_obs->value;
}

double summary_obs_get_std( const summary_obs_type * summary_obs ) {
  return summary_obs->std;
}

void summary_obs_scale_std(summary_obs_type * summary_obs, double std_multiplier ) {
   summary_obs->std = summary_obs->std * std_multiplier;
}

void summary_obs_scale_std__(void * summary_obs, double std_multiplier ) {
  summary_obs_type * observation = summary_obs_safe_cast(summary_obs);
  summary_obs_scale_std(observation, std_multiplier);
}


/*****************************************************************/

VOID_FREE(summary_obs)
VOID_GET_OBS(summary_obs)
VOID_USER_GET_OBS(summary_obs)
VOID_MEASURE(summary_obs , summary)
VOID_CHI2(summary_obs , summary)
