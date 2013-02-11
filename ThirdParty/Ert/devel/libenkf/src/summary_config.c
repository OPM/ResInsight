/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/set.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_smspec.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/summary_config.h>
#include <ert/enkf/enkf_macros.h>


#define SUMMARY_CONFIG_TYPE_ID 63106

struct summary_config_struct {
  int                   __type_id;
  bool                  vector_storage;
  load_fail_type        load_fail;
  ecl_smspec_var_type   var_type;         /* The type of the variable - according to ecl_summary nomenclature. */
  char * var;                             /* This is ONE variable of summary.x format - i.e. WOPR:OP_2, RPR:4, ... */
  set_type            * obs_set;          /* Set of keys (which fit in enkf_obs) which are observations of this node. */ 
};


/*****************************************************************/



const char * summary_config_get_var(const summary_config_type * config) {
  return config->var;
}


ecl_smspec_var_type summary_config_get_var_type(summary_config_type * config , const ecl_sum_type * ecl_sum) {
  return config->var_type;
}


bool summary_config_get_vector_storage( const summary_config_type * config) {
  return config->vector_storage;
}


load_fail_type summary_config_get_load_fail_mode( const summary_config_type * config) {
  return config->load_fail;
}

/**
   Unfortunately it is a bit problematic to set the required flag to
   TRUE for well and group variables because they do not exist in the
   summary results before the well has actually opened, i.e. for a
   partial summary case the results will not be there, and the loader
   will incorrectly(?) signal failure.
*/
   
void summary_config_set_load_fail_mode( summary_config_type * config , load_fail_type load_fail) {
  if ((config->var_type == ECL_SMSPEC_WELL_VAR) || (config->var_type == ECL_SMSPEC_GROUP_VAR))
    // For well and group variables load_fail will be LOAD_FAIL_SILENT anyway. 
    config->load_fail = LOAD_FAIL_SILENT;
  else
    config->load_fail = load_fail;
}


/**
   This can only be used to increase the load_fail strictness.
*/

void summary_config_update_load_fail_mode( summary_config_type * config , load_fail_type load_fail) {
  if (load_fail > config->load_fail)
    summary_config_set_load_fail_mode( config , load_fail );
}


summary_config_type * summary_config_alloc(const char * var , bool vector_storage , load_fail_type load_fail) {
  summary_config_type * config = util_malloc(sizeof *config );
  config->__type_id            = SUMMARY_CONFIG_TYPE_ID;
  config->var                  = util_alloc_string_copy( var );
  config->var_type             = ecl_smspec_identify_var_type( var );
  config->obs_set              = set_alloc_empty(); 
  config->vector_storage       = vector_storage;
  summary_config_set_load_fail_mode( config , load_fail);
  return config;
}


void summary_config_add_obs_key(summary_config_type * config, const char * obs_key) {
  set_add_key(config->obs_set , obs_key);
}



void summary_config_free(summary_config_type * config) {
  free(config->var);
  set_free(config->obs_set);
  free(config);
}



int summary_config_get_byte_size(const summary_config_type * config) {
  return sizeof(double);
}


int summary_config_get_data_size( const summary_config_type * config) {
  return 1;
}






/*****************************************************************/
UTIL_SAFE_CAST_FUNCTION(summary_config , SUMMARY_CONFIG_TYPE_ID)
UTIL_SAFE_CAST_FUNCTION_CONST(summary_config , SUMMARY_CONFIG_TYPE_ID)
VOID_GET_DATA_SIZE(summary)
VOID_CONFIG_FREE(summary)

