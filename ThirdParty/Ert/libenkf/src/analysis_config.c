/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'analysis_config.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>
#include <ert/util/rng.h>
#include <ert/util/type_macros.h>

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>

#include <ert/analysis/analysis_module.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/analysis_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/analysis_iter_config.h>


#define ANALYSIS_CONFIG_TYPE_ID 64431306

struct analysis_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type                     * analysis_modules;
  analysis_module_type          * analysis_module;
  char                          * log_path;                    /* Points to directory with update logs. */
  bool                            merge_observations;          /* When observing from time1 to time2 - should ALL observations in between be used? */
  bool                            rerun;                       /* Should we rerun the simulator when the parameters have been updated? */
  int                             rerun_start;                 /* When rerunning - from where should we start? */

  double                          overlap_alpha;
  double                          std_cutoff;

  char                          * PC_filename;
  char                          * PC_path;
  bool                            store_PC;
  bool                            single_node_update;          /* When creating the default ALL_ACTIVE local configuration. */
  rng_type                      * rng;
  analysis_iter_config_type     * iter_config;
  int                             min_realisations;
  bool                            stop_long_running;
  bool                            std_scale_correlated_obs;
  int                             max_runtime;
  double                          global_std_scaling;
};




UTIL_IS_INSTANCE_FUNCTION( analysis_config , ANALYSIS_CONFIG_TYPE_ID )

/*****************************************************************/
/*

Interacting with modules
------------------------

The modules which are included in the build must be installed/loaded with a
hard-coded call to analysis_config_load_internal_module(). External modules
are loaded with the config statement:

   ANALYSIS_LOAD    ModuleName   libfile

Where 'ModuleName is the name you want to use to refer to the module, and
libfile is the name of the library file which implements the analysis
module[1].

It is possible to create a copy of an analysis module under a different
name, this can be convenient when trying out the same algorithm with
different parameter settings. I.e. based on the built in module STD_ENKF we
can create two copies with high and low truncation respectively:

   ANALYSIS_COPY  STD_ENKF  ENKF_HIGH_TRUNCATION
   ANALYSIS_COPY  STD_ENKF  ENKF_LOW_TRUNCATION

The copy operation does not differentiate between external and internal
modules. When a module has been loaded you can set internal parameters for
the module with the config command:

   ANALYSIS_SET_VAR  ModuleName  VariableName   Value

The module will be called with a function for setting variables which gets
the VariableName and value parameters as input; if the module recognizes
VariableName and Value is of the right type the module should set the
internal variable accordingly. If the module does not recognize the
variable name a warning will be printed on stderr, but no further action.

The actual analysis module to use is selected with the statement:

ANALYSIS_SELECT  ModuleName

[1] The libfile argument should include the '.so' extension, and can
    optionally contain a path component. The libfile will be passed directly to
    the dlopen() library call, this implies that normal runtime linking
    conventions apply - i.e. you have three options:

     1. The library name is given with a full path.
     2. The library is in a standard location for shared libraries.
     3. The library is in one of the directories mentioned in the
        LD_LIBRARY_PATH environment variable.

*/


/*****************************************************************/

bool analysis_config_have_enough_realisations( const analysis_config_type * config , int realisations, int ensemble_size) {
  if (config->min_realisations > 0) {
    /* A value > 0 has been set in the config; compare with this value. */
    return realisations >= config->min_realisations;
  } 
  else {
    return realisations >= ensemble_size;
  }
}

void analysis_config_set_stop_long_running( analysis_config_type * config, bool stop_long_running ) {
  config->stop_long_running = stop_long_running;
}

bool analysis_config_get_stop_long_running( const analysis_config_type * config) {
  return config->stop_long_running;
}

bool analysis_config_get_std_scale_correlated_obs( const analysis_config_type * config) {
  return config->std_scale_correlated_obs;
}

void analysis_config_set_std_scale_correlated_obs( analysis_config_type * config, bool std_scale_correlated_obs) {
  config->std_scale_correlated_obs = std_scale_correlated_obs;
}

double analysis_config_get_global_std_scaling(const analysis_config_type * config) {
  return config->global_std_scaling;
}

void analysis_config_set_global_std_scaling(analysis_config_type * config, double global_std_scaling) {
  config->global_std_scaling = global_std_scaling;
}

int analysis_config_get_max_runtime( const analysis_config_type * config ) {
  return config->max_runtime;
}

void analysis_config_set_max_runtime( analysis_config_type * config, int max_runtime ) {
  config->max_runtime = max_runtime;
}

static void analysis_config_set_min_realisations( analysis_config_type * config , int min_realisations) {
  config->min_realisations = min_realisations;
}

void analysis_config_set_alpha( analysis_config_type * config , double alpha) {
  config->overlap_alpha = alpha;
}

stringlist_type * analysis_config_alloc_module_names( analysis_config_type * config ) {
  return hash_alloc_stringlist( config->analysis_modules );
}


double analysis_config_get_alpha(const analysis_config_type * config) {
  return config->overlap_alpha;
}

void analysis_config_set_store_PC( analysis_config_type * config , bool store_PC) {
  config->store_PC = store_PC;
}

bool analysis_config_get_store_PC( const analysis_config_type * config ) {
  return config->store_PC;
}

void analysis_config_set_PC_filename( analysis_config_type * config , const char * filename ) {
  config->PC_filename = util_realloc_string_copy( config->PC_filename , filename );
}

const char * analysis_config_get_PC_filename( const analysis_config_type * config ) {
  return config->PC_filename;
}


void analysis_config_set_PC_path( analysis_config_type * config , const char * path ) {
  config->PC_path = util_realloc_string_copy( config->PC_path , path );
}

const char * analysis_config_get_PC_path( const analysis_config_type * config ) {
  return config->PC_path;
}

void analysis_config_set_std_cutoff( analysis_config_type * config , double std_cutoff ) {
  config->std_cutoff = std_cutoff;
}

double analysis_config_get_std_cutoff(const analysis_config_type * config) {
  return config->std_cutoff;
}


void analysis_config_set_log_path(analysis_config_type * config , const char * log_path ) {
  config->log_path        = util_realloc_string_copy(config->log_path , log_path);
}


/**
   Will in addition create the path.
*/
const char * analysis_config_get_log_path( const analysis_config_type * config ) {
  util_make_path( config->log_path );
  return config->log_path;
}



void analysis_config_set_rerun_start( analysis_config_type * config , int rerun_start ) {
  config->rerun_start = rerun_start;
}

void analysis_config_set_rerun(analysis_config_type * config , bool rerun) {
  config->rerun = rerun;
}

bool analysis_config_get_rerun(const analysis_config_type * config) {
  return config->rerun;
}

void analysis_config_set_single_node_update(analysis_config_type * config , bool single_node_update) {
  config->single_node_update = single_node_update;
}

bool analysis_config_get_single_node_update(const analysis_config_type * config) {
  return config->single_node_update;
}


int analysis_config_get_rerun_start(const analysis_config_type * config) {
  return config->rerun_start;
}


void analysis_config_set_merge_observations( analysis_config_type * config , bool merge_observations) {
  config->merge_observations = merge_observations;
}



/*****************************************************************/

void analysis_config_load_internal_module( analysis_config_type * config ,
                                           const char * symbol_table ) {
  analysis_module_type * module = analysis_module_alloc_internal( config->rng , symbol_table );
  if (module != NULL)
    hash_insert_hash_owned_ref( config->analysis_modules , analysis_module_get_name( module ) , module , analysis_module_free__ );
  else
    fprintf(stderr,"** Warning: failed to load module %s from %s.\n", analysis_module_get_name( module ) , symbol_table);
}


void analysis_config_load_all_external_modules_from_config ( analysis_config_type * analysis, const config_content_type * config) {

  if (config_content_has_item( config, ANALYSIS_LOAD_KEY)) {
    const config_content_item_type * load_item = config_content_get_item( config , ANALYSIS_LOAD_KEY );
    for (int i=0; i < config_content_item_get_size( load_item ); i++) {
      const config_content_node_type * load_node = config_content_item_iget_node( load_item , i );
      const char * user_name = config_content_node_iget( load_node , 0 );
      const char * lib_name  = config_content_node_iget( load_node , 1 );

      analysis_config_load_external_module( analysis , lib_name , user_name);
    }
  }
}


bool analysis_config_load_external_module( analysis_config_type * config ,
                                           const char * lib_name,
                                           const char * user_name) {
  analysis_module_type * module = analysis_module_alloc_external( config->rng , lib_name );
  if (module != NULL) {
    if (user_name)
      analysis_module_set_name(module, user_name);
    hash_insert_hash_owned_ref( config->analysis_modules , analysis_module_get_name( module ) ,  module , analysis_module_free__ );
    return true;
  } else {
    fprintf(stderr,"** Warning: failed to load module from %s.\n",lib_name);
    return false;
  }
}


void analysis_config_add_module_copy( analysis_config_type * config ,
                                      const char * src_name ,
                                      const char * target_name) {
  const analysis_module_type * src_module = analysis_config_get_module( config , src_name );
  analysis_module_type * target_module;

  if (analysis_module_internal( src_module )) {
    const char * symbol_table = analysis_module_get_table_name( src_module );
    target_module = analysis_module_alloc_internal( config->rng , symbol_table );
  } else {
    const char * lib_name = analysis_module_get_lib_name( src_module );
    target_module = analysis_module_alloc_external( config->rng , lib_name );
  }

  hash_insert_hash_owned_ref( config->analysis_modules , target_name , target_module , analysis_module_free__ );
  analysis_module_set_name( target_module , target_name );
}



/*
  If module_name == NULL we will reload the current module. Unloading
  modules is based on the dlclose() system call; internally the
  dlopen() / dlclose() systems implements a reference counting to the
  shared objects, i.e. if you have several modules defined in the same
  shared library the dlclose() call will not unload the module
  completely, and a subsequent dlopen() call will be satisfied by the
  stale shared objects still mapped. Practically this means that:

  * Internal modules (all in libanalysis.so) can not be reloaded.

  * Modules which have been involved in copy operations, either as
    source or target can not be reloaded.
*/

void analysis_config_reload_module( analysis_config_type * config , const char * module_name) {
  analysis_module_type * module;
  if (module_name != NULL)
    module = analysis_config_get_module( config , module_name );
  else
    module = config->analysis_module;

  if (!analysis_module_internal( module )) {
    char * user_name = util_alloc_string_copy(analysis_module_get_name( module ));
    char * lib_name  = util_alloc_string_copy(analysis_module_get_lib_name( module ));

    bool is_current        = false;
    if (module == config->analysis_module) {
      config->analysis_module = NULL;
      is_current = true;
    }

    hash_del( config->analysis_modules , user_name );
    analysis_config_load_external_module( config , user_name , lib_name );
    if (is_current)
      analysis_config_select_module( config , user_name );

    free( lib_name );
    free( user_name );
  } else
    fprintf(stderr,"** Warning: Internal modules can not be reloaded.\n");
}


analysis_module_type * analysis_config_get_module( analysis_config_type * config , const char * module_name ) {
  return hash_get( config->analysis_modules , module_name );
}

bool analysis_config_has_module(analysis_config_type * config , const char * module_name) {
  return hash_has_key( config->analysis_modules , module_name );
}

bool analysis_config_get_module_option( const analysis_config_type * config , long flag) {
  if (config->analysis_module)
    return analysis_module_check_option(config->analysis_module , flag);
  else
    return false;
}


bool analysis_config_select_module( analysis_config_type * config , const char * module_name ) {
  if (analysis_config_has_module( config , module_name )) {
    analysis_module_type * module = analysis_config_get_module( config , module_name );

    if (analysis_module_check_option( module , ANALYSIS_ITERABLE)) {
      if (analysis_config_get_single_node_update( config )) {
        fprintf(stderr," ** Warning: the module:%s requires the setting \"SINGLE_NODE_UPDATE FALSE\" in the config file.\n" , module_name);
        fprintf(stderr," **          the module has NOT been selected. \n");
        return false;
      }
    }

    config->analysis_module = module;
    return true;
  } else {
    if (config->analysis_module == NULL)
      util_abort("%s: sorry module:%s does not exist - and no module currently selected\n",__func__ , module_name);
    else
      fprintf(stderr , "** Warning: analysis module:%s does not exist - current selection unchanged:%s\n",
              module_name ,
              analysis_module_get_name( config->analysis_module ));
    return false;
  }
}


analysis_module_type * analysis_config_get_active_module( analysis_config_type * config ) {
  return config->analysis_module;
}


const char * analysis_config_get_active_module_name( const analysis_config_type * config ) {
  if (config->analysis_module)
    return analysis_module_get_name( config->analysis_module );
  else
    return NULL;
}



/*****************************************************************/


void analysis_config_load_internal_modules( analysis_config_type * config ) {
  analysis_config_load_internal_module( config , "STD_ENKF");
  analysis_config_load_internal_module( config , "NULL_ENKF");
  analysis_config_load_internal_module( config , "SQRT_ENKF");
  analysis_config_load_internal_module( config , "CV_ENKF");
  analysis_config_load_internal_module( config , "BOOTSTRAP_ENKF");
  analysis_config_load_internal_module( config , "FWD_STEP_ENKF");
  analysis_config_select_module( config , DEFAULT_ANALYSIS_MODULE);
}

/**
   The analysis_config object is instantiated with the default values
   for enkf_defaults.h
*/

void analysis_config_init( analysis_config_type * analysis , const config_content_type * config ) {
  if (config_content_has_item( config , UPDATE_LOG_PATH_KEY ))
    analysis_config_set_log_path( analysis , config_content_get_value( config , UPDATE_LOG_PATH_KEY ));

  if (config_content_has_item( config , STD_CUTOFF_KEY ))
    analysis_config_set_std_cutoff( analysis , config_content_get_value_as_double( config , STD_CUTOFF_KEY ));

  if (config_content_has_item( config , ENKF_ALPHA_KEY ))
    analysis_config_set_alpha( analysis , config_content_get_value_as_double( config , ENKF_ALPHA_KEY ));

  if (config_content_has_item( config , ENKF_MERGE_OBSERVATIONS_KEY ))
    analysis_config_set_merge_observations( analysis , config_content_get_value_as_bool( config , ENKF_MERGE_OBSERVATIONS_KEY ));

  if (config_content_has_item( config , ENKF_RERUN_KEY ))
    analysis_config_set_rerun( analysis , config_content_get_value_as_bool( config , ENKF_RERUN_KEY ));

  if (config_content_has_item( config , SINGLE_NODE_UPDATE_KEY ))
    analysis_config_set_single_node_update( analysis , config_content_get_value_as_bool( config , SINGLE_NODE_UPDATE_KEY ));

  if (config_content_has_item( config , STD_SCALE_CORRELATED_OBS_KEY ))
    analysis_config_set_std_scale_correlated_obs( analysis , config_content_get_value_as_bool( config , STD_SCALE_CORRELATED_OBS_KEY ));

  if (config_content_has_item( config , RERUN_START_KEY ))
    analysis_config_set_rerun_start( analysis , config_content_get_value_as_int( config , RERUN_START_KEY ));

  if (config_content_has_item( config , MIN_REALIZATIONS_KEY )) {
    
    config_content_node_type * config_content = config_content_get_value_node(config , MIN_REALIZATIONS_KEY);
    char * min_realizations_string            = config_content_node_alloc_joined_string(config_content, " ");
    
    int num_realizations = config_content_get_value_as_int(config, NUM_REALIZATIONS_KEY);
    int min_realizations                      = DEFAULT_ANALYSIS_MIN_REALISATIONS;
    double percent                            = 0.0;
    if (util_sscanf_percent(min_realizations_string, &percent)) {
      
      min_realizations = num_realizations * percent/100;
    } else {
      bool min_realizations_int_exists = util_sscanf_int(min_realizations_string, &min_realizations);
      if (!min_realizations_int_exists)
        fprintf(stderr, "Method %s: failed to read integer value for MIN_REALIZATIONS_KEY\n", __func__);
    }
    
    if (min_realizations > num_realizations)
      min_realizations = num_realizations;
    
    analysis_config_set_min_realisations(analysis, min_realizations);
    free(min_realizations_string);
  }

  if (config_content_has_item( config , STOP_LONG_RUNNING_KEY ))
    analysis_config_set_stop_long_running( analysis , config_content_get_value_as_bool( config , STOP_LONG_RUNNING_KEY ));

  if (config_content_has_item( config, MAX_RUNTIME_KEY)) {
    analysis_config_set_max_runtime( analysis, config_content_get_value_as_int( config, MAX_RUNTIME_KEY ));
  }


  /* Loading external modules */
  analysis_config_load_all_external_modules_from_config(analysis, config);


  /* Reload/copy modules. */
  {
    if (config_content_has_item( config , ANALYSIS_COPY_KEY )) {
      const config_content_item_type * copy_item = config_content_get_item( config , ANALYSIS_COPY_KEY );
      for (int i=0; i < config_content_item_get_size( copy_item ); i++) {
        const config_content_node_type * copy_node = config_content_item_iget_node( copy_item , i );
        const char * src_name = config_content_node_iget( copy_node , 0 );
        const char * target_name  = config_content_node_iget( copy_node , 1 );

        analysis_config_add_module_copy( analysis , src_name , target_name);
      }
    }
  }


  /* Setting variables for analysis modules */
  {
    if (config_content_has_item( config , ANALYSIS_SET_VAR_KEY )) {
      const config_content_item_type * assign_item = config_content_get_item( config , ANALYSIS_SET_VAR_KEY );
      for (int i=0; i < config_content_item_get_size( assign_item ); i++) {
        const config_content_node_type * assign_node = config_content_item_iget_node( assign_item , i );

        const char * module_name = config_content_node_iget( assign_node , 0 );
        const char * var_name    = config_content_node_iget( assign_node , 1 );
        analysis_module_type * module = analysis_config_get_module( analysis , module_name );
        {
          char * value = NULL;

          for (int j=2; j < config_content_node_get_size( assign_node ); j++) {
            const char * config_value = config_content_node_iget( assign_node , j );
            if (value == NULL)
              value = util_alloc_string_copy( config_value );
            else {
              value = util_strcat_realloc( value , " " );
              value = util_strcat_realloc( value , config_value );
            }
          }

          analysis_module_set_var( module , var_name , value );
          free( value );
        }
      }
    }
  }

  if (config_content_has_item( config, ANALYSIS_SELECT_KEY ))
    analysis_config_select_module( analysis , config_content_get_value( config , ANALYSIS_SELECT_KEY ));

  analysis_iter_config_init( analysis->iter_config , config );
}



bool analysis_config_get_merge_observations(const analysis_config_type * config) {
  return config->merge_observations;
}


analysis_iter_config_type * analysis_config_get_iter_config( const analysis_config_type * config ) {
  return config->iter_config;
}


void analysis_config_free(analysis_config_type * config) {
  analysis_iter_config_free( config->iter_config );
  hash_free( config->analysis_modules );
  free( config->log_path );
  free( config->PC_filename );
  free( config->PC_path );
  free( config );
}



analysis_config_type * analysis_config_alloc( rng_type * rng ) {
  analysis_config_type * config = util_malloc( sizeof * config );
  UTIL_TYPE_ID_INIT( config , ANALYSIS_CONFIG_TYPE_ID );

  config->log_path                  = NULL;
  config->PC_filename               = NULL;
  config->PC_path                   = NULL;

  analysis_config_set_alpha( config                    , DEFAULT_ENKF_ALPHA );
  analysis_config_set_std_cutoff( config               , DEFAULT_ENKF_STD_CUTOFF );
  analysis_config_set_merge_observations( config       , DEFAULT_MERGE_OBSERVATIONS );
  analysis_config_set_rerun( config                    , DEFAULT_RERUN );
  analysis_config_set_rerun_start( config              , DEFAULT_RERUN_START );
  analysis_config_set_single_node_update( config       , DEFAULT_SINGLE_NODE_UPDATE );
  analysis_config_set_log_path( config                 , DEFAULT_UPDATE_LOG_PATH);

  analysis_config_set_store_PC( config                 , DEFAULT_STORE_PC );
  analysis_config_set_PC_filename( config              , DEFAULT_PC_FILENAME );
  analysis_config_set_PC_path( config                  , DEFAULT_PC_PATH );
  analysis_config_set_min_realisations( config         , DEFAULT_ANALYSIS_MIN_REALISATIONS );
  analysis_config_set_stop_long_running( config        , DEFAULT_ANALYSIS_STOP_LONG_RUNNING );
  analysis_config_set_max_runtime( config              , DEFAULT_MAX_RUNTIME );

  config->analysis_module      = NULL;
  config->analysis_modules     = hash_alloc();
  config->rng                  = rng;
  config->iter_config          = analysis_iter_config_alloc();
  config->std_scale_correlated_obs = false;
  config->global_std_scaling   = 1.0;
  return config;
}



/*****************************************************************/
/*
  Keywords for the analysis - all optional. The analysis_config object
  is instantiated with defaults from enkf_defaults.h
*/

void analysis_config_add_config_items( config_parser_type * config ) {
  config_schema_item_type * item;

  config_add_key_value( config , ENKF_ALPHA_KEY              , false , CONFIG_FLOAT);
  config_add_key_value( config , STD_CUTOFF_KEY              , false , CONFIG_FLOAT);
  config_add_key_value( config , ENKF_MERGE_OBSERVATIONS_KEY , false , CONFIG_BOOL);
  config_add_key_value( config , SINGLE_NODE_UPDATE_KEY      , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_CROSS_VALIDATION_KEY   , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_LOCAL_CV_KEY           , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_PEN_PRESS_KEY          , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_SCALING_KEY            , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_KERNEL_REG_KEY         , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_KERNEL_FUNC_KEY        , false , CONFIG_INT);
  config_add_key_value( config , ENKF_KERNEL_PARAM_KEY       , false , CONFIG_INT);
  config_add_key_value( config , ENKF_FORCE_NCOMP_KEY        , false , CONFIG_BOOL);
  config_add_key_value( config , ENKF_NCOMP_KEY              , false , CONFIG_INT);
  config_add_key_value( config , ENKF_CV_FOLDS_KEY           , false , CONFIG_INT);
  config_add_key_value( config , ENKF_RERUN_KEY              , false , CONFIG_BOOL);
  config_add_key_value( config , RERUN_START_KEY             , false , CONFIG_INT);
  config_add_key_value( config , UPDATE_LOG_PATH_KEY         , false , CONFIG_STRING);
  config_add_key_value( config , MIN_REALIZATIONS_KEY        , false , CONFIG_STRING );
  config_add_key_value( config , MAX_RUNTIME_KEY             , false , CONFIG_INT );
  config_add_key_value( config , STD_SCALE_CORRELATED_OBS_KEY, false , CONFIG_BOOL );

  item = config_add_key_value( config , STOP_LONG_RUNNING_KEY, false,  CONFIG_BOOL );
  stringlist_type * child_list = stringlist_alloc_new();
  stringlist_append_ref(child_list, MIN_REALIZATIONS_KEY);
  config_schema_item_set_required_children_on_value(item , "TRUE" , child_list);
  stringlist_free(child_list);

  config_add_key_value( config , ANALYSIS_SELECT_KEY         , false , CONFIG_STRING);

  item = config_add_schema_item( config , ANALYSIS_LOAD_KEY , false  );
  config_schema_item_set_argc_minmax( item , 2 , 2);

  item = config_add_schema_item( config , ANALYSIS_COPY_KEY , false  );
  config_schema_item_set_argc_minmax( item , 2 , 2);


  item = config_add_schema_item( config , ANALYSIS_SET_VAR_KEY , false  );
  config_schema_item_set_argc_minmax( item , 3 , CONFIG_DEFAULT_ARG_MAX);
  analysis_iter_config_add_config_items( config );
}



void analysis_config_fprintf_config( analysis_config_type * config , FILE * stream) {
  fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
  fprintf( stream , CONFIG_COMMENT_FORMAT , "Here comes configuration information related to the EnKF analysis.");


  if (config->merge_observations != DEFAULT_MERGE_OBSERVATIONS) {
    fprintf( stream , CONFIG_KEY_FORMAT        , ENKF_MERGE_OBSERVATIONS_KEY);
    fprintf( stream , CONFIG_ENDVALUE_FORMAT   , CONFIG_BOOL_STRING( config->merge_observations ));
  }


  if (config->std_cutoff != DEFAULT_ENKF_STD_CUTOFF) {
    fprintf( stream , CONFIG_KEY_FORMAT   , STD_CUTOFF_KEY );
    fprintf( stream , CONFIG_FLOAT_FORMAT , config->std_cutoff );
    fprintf( stream , "\n");
  }

  if (config->overlap_alpha != DEFAULT_ENKF_ALPHA ) {
    fprintf( stream , CONFIG_KEY_FORMAT   , ENKF_ALPHA_KEY );
    fprintf( stream , CONFIG_FLOAT_FORMAT , config->overlap_alpha );
    fprintf( stream , "\n");
  }

  if (config->single_node_update != DEFAULT_SINGLE_NODE_UPDATE) {
    fprintf( stream , CONFIG_KEY_FORMAT        , SINGLE_NODE_UPDATE_KEY);
    fprintf( stream , CONFIG_ENDVALUE_FORMAT   , CONFIG_BOOL_STRING( config->single_node_update ));
  }

  if (config->rerun) {
    fprintf( stream , CONFIG_KEY_FORMAT        , ENKF_RERUN_KEY);
    fprintf( stream , CONFIG_ENDVALUE_FORMAT   , CONFIG_BOOL_STRING( config->rerun ));
  }

  if (config->rerun_start != DEFAULT_RERUN_START) {
    fprintf( stream , CONFIG_KEY_FORMAT   , RERUN_START_KEY);
    fprintf( stream , CONFIG_INT_FORMAT   , config->rerun_start );
    fprintf( stream , "\n");
  }

  if (config->log_path != NULL) {
    fprintf( stream , CONFIG_KEY_FORMAT      , UPDATE_LOG_PATH_KEY);
    fprintf( stream , CONFIG_ENDVALUE_FORMAT , config->log_path );
  }

  fprintf(stream , "\n\n");
}



