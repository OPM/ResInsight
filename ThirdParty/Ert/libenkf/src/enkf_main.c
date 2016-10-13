/*
   Copyright (C) 2011  Statoil ASA, Norway.
   The file 'enkf_main.c' is part of ERT - Ensemble based Reservoir Tool.

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



#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#define HAVE_THREAD_POOL 1
#include <ert/util/matrix.h>
#include <ert/util/subst_list.h>
#include <ert/util/rng.h>
#include <ert/util/subst_func.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/path_fmt.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>
#include <ert/util/msg.h>
#include <ert/util/stringlist.h>
#include <ert/util/set.h>
#include <ert/util/node_ctype.h>
#include <ert/util/string_util.h>
#include <ert/util/type_vector_functions.h>

#include <ert/config/config_parser.h>
#include <ert/config/config_schema_item.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_io_config.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/job_queue_manager.h>
#include <ert/job_queue/local_driver.h>
#include <ert/job_queue/rsh_driver.h>
#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/forward_model.h>
#include <ert/job_queue/queue_driver.h>

#include <ert/sched/history.h>
#include <ert/sched/sched_file.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/module_info.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/plot_config.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/model_config.h>
#include <ert/enkf/hook_manager.h>
#include <ert/enkf/site_config.h>
#include <ert/enkf/active_config.h>
#include <ert/enkf/enkf_analysis.h>
#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_updatestep.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/local_dataset.h>
#include <ert/enkf/misfit_ensemble.h>
#include <ert/enkf/ert_template.h>
#include <ert/enkf/rng_config.h>
#include <ert/enkf/enkf_plot_data.h>
#include <ert/enkf/ranking_table.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/runpath_list.h>
#include <ert/enkf/pca_plot_data.h>
#include <ert/enkf/analysis_config.h>
#include <ert/enkf/analysis_iter_config.h>
#include <ert/enkf/field.h>
#include <ert/enkf/ert_log.h>
#include <ert/enkf/ert_init_context.h>
#include <ert/enkf/ert_run_context.h>
#include <ert/enkf/run_arg.h>

/**/

/**
   This object should contain **everything** needed to run a enkf
   simulation. A way to wrap up all available information/state and
   pass it around. An attempt has been made to collect various pieces
   of related information together in a couple of objects
   (model_config, ecl_config, site_config and ensemble_config). When
   it comes to these holding objects the following should be observed:

    1. It not always obvious where a piece of information should be
       stored, i.e. the grid is a property of the model, however it is
       an eclipse grid, and hence also belongs to eclipse
       configuration?? [In this case ecl_config wins out.]

    2. The information stored in these objects is typically passed on
       to the enkf_state object, where it is used.

    3. At enkf_state level it is not really consequent - in some cases
       the enkf_state object takes a scalar copy (i.e. keep_runpath),
       and in other cases only a pointer down to the underlying
       enkf_main object is taken. In the former case it is no way to
       change global behaviour by modifying the enkf_main objects.

       In the enkf_state object the fields of the member_config,
       ecl_config, site_config and ensemble_config objects are mixed
       and matched into other small holding objects defined in
       enkf_state.c.

*/

#define ENKF_MAIN_ID              8301

struct enkf_main_struct {
  UTIL_TYPE_ID_DECLARATION;
  enkf_fs_type         * dbase;              /* The internalized information. */

  ensemble_config_type * ensemble_config;    /* The config objects for the various enkf nodes.*/
  hook_manager_type    * hook_manager;
  model_config_type    * model_config;
  ecl_config_type      * ecl_config;
  site_config_type     * site_config;
  analysis_config_type * analysis_config;
  local_config_type    * local_config;       /* Holding all the information about local analysis. */
  ert_templates_type   * templates;          /* Run time templates */
  plot_config_type     * plot_config;        /* Information about plotting. */
  rng_config_type      * rng_config;
  rng_type             * rng;
  ert_workflow_list_type * workflow_list;
  ranking_table_type   * ranking_table;

  /*---------------------------*/            /* Variables related to substitution. */
  subst_func_pool_type * subst_func_pool;
  subst_list_type      * subst_list;         /* A parent subst_list instance - common to all ensemble members. */
  /*-------------------------*/

  int_vector_type      * keep_runpath;       /* HACK: This is only used in the initialization period - afterwards the data is held by the enkf_state object. */
  bool                   pre_clear_runpath;  /* HACK: This is only used in the initialization period - afterwards the data is held by the enkf_state object. */

  char                 * site_config_file;
  char                 * user_config_file;
  char                 * rft_config_file;       /* File giving the configuration to the RFTwells*/
  enkf_obs_type        * obs;
  enkf_state_type     ** ensemble;         /* The ensemble ... */
  int                    ens_size;         /* The size of the ensemble */
  bool                   verbose;
};




/*****************************************************************/

void enkf_main_init_internalization( enkf_main_type *  , run_mode_type  );
void enkf_main_update_local_updates( enkf_main_type * enkf_main);
static void enkf_main_close_fs( enkf_main_type * enkf_main );
static void enkf_main_init_fs( enkf_main_type * enkf_main );
static void enkf_main_user_select_initial_fs(enkf_main_type * enkf_main );

/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION(enkf_main , ENKF_MAIN_ID)
UTIL_IS_INSTANCE_FUNCTION(enkf_main , ENKF_MAIN_ID)

analysis_config_type * enkf_main_get_analysis_config(const enkf_main_type * enkf_main) {
  return enkf_main->analysis_config;
}

bool enkf_main_get_pre_clear_runpath( const enkf_main_type * enkf_main ) {
  return enkf_state_get_pre_clear_runpath( enkf_main->ensemble[0] );
}

void enkf_main_set_pre_clear_runpath( enkf_main_type * enkf_main , bool pre_clear_runpath) {
  const int ens_size = enkf_main_get_ensemble_size( enkf_main );
  int iens;
  for (iens = 0; iens < ens_size; iens++)
    enkf_state_set_pre_clear_runpath( enkf_main->ensemble[iens] , pre_clear_runpath );
}


bool enkf_main_set_refcase( enkf_main_type * enkf_main , const char * refcase_path) {
  bool set_refcase = ecl_config_load_refcase( enkf_main->ecl_config , refcase_path );

  model_config_set_refcase( enkf_main->model_config , ecl_config_get_refcase( enkf_main->ecl_config ));
  ensemble_config_set_refcase( enkf_main->ensemble_config , ecl_config_get_refcase( enkf_main->ecl_config ));

  return set_refcase;
}


ui_return_type * enkf_main_validata_refcase( const enkf_main_type * enkf_main , const char * refcase_path) {
  return ecl_config_validate_refcase( enkf_main->ecl_config , refcase_path );
}


ui_return_type * enkf_main_set_eclbase( enkf_main_type * enkf_main , const char * eclbase_fmt) {
  ui_return_type * ui_return = ecl_config_validate_eclbase( enkf_main->ecl_config , eclbase_fmt);
  if (ui_return_get_status(ui_return) == UI_RETURN_OK) {
    ecl_config_set_eclbase( enkf_main->ecl_config , eclbase_fmt );
    for (int iens = 0; iens < enkf_main->ens_size; iens++)
      enkf_state_update_eclbase(enkf_main->ensemble[iens]);
  }
  return ui_return;
}

void enkf_main_init_jobname( enkf_main_type * enkf_main) {
  for (int iens = 0; iens < enkf_main->ens_size; iens++)
    enkf_state_update_jobname( enkf_main->ensemble[iens] );
}


void enkf_main_set_jobname( enkf_main_type * enkf_main , const char * jobname_fmt) {
  model_config_set_jobname_fmt( enkf_main->model_config , jobname_fmt );
  enkf_main_init_jobname( enkf_main );
}

void enkf_main_set_user_config_file( enkf_main_type * enkf_main , const char * user_config_file ) {
  enkf_main->user_config_file = util_realloc_string_copy( enkf_main->user_config_file , user_config_file );
}

void enkf_main_set_rft_config_file( enkf_main_type * enkf_main , const char * rft_config_file ) {
  enkf_main->rft_config_file = util_realloc_string_copy( enkf_main->rft_config_file , rft_config_file );
}

void enkf_main_set_site_config_file( enkf_main_type * enkf_main , const char * site_config_file ) {
  enkf_main->site_config_file = util_realloc_string_copy( enkf_main->site_config_file , site_config_file );
}

const char * enkf_main_get_user_config_file( const enkf_main_type * enkf_main ) {
  return enkf_main->user_config_file;
}

const char * enkf_main_get_site_config_file( const enkf_main_type * enkf_main ) {
  return enkf_main->site_config_file;
}

const char * enkf_main_get_rft_config_file( const enkf_main_type * enkf_main ) {
  return enkf_main->rft_config_file;
}

ensemble_config_type * enkf_main_get_ensemble_config(const enkf_main_type * enkf_main) {
  return enkf_main->ensemble_config;
}

site_config_type * enkf_main_get_site_config( const enkf_main_type * enkf_main ) {
  return enkf_main->site_config;
}


subst_list_type * enkf_main_get_data_kw( const enkf_main_type * enkf_main ) {
  return enkf_main->subst_list;
}


local_config_type * enkf_main_get_local_config( const enkf_main_type * enkf_main ) {
  return enkf_main->local_config;
}

model_config_type * enkf_main_get_model_config( const enkf_main_type * enkf_main ) {
  return enkf_main->model_config;
}

plot_config_type * enkf_main_get_plot_config( const enkf_main_type * enkf_main ) {
  return enkf_main->plot_config;
}

ranking_table_type * enkf_main_get_ranking_table( const enkf_main_type * enkf_main ) {
  return enkf_main->ranking_table;
}

ecl_config_type *enkf_main_get_ecl_config(const enkf_main_type * enkf_main) {
        return enkf_main->ecl_config;
}

int enkf_main_get_history_length( const enkf_main_type * enkf_main) {
  return model_config_get_last_history_restart( enkf_main->model_config);
}

bool enkf_main_has_prediction( const enkf_main_type * enkf_main ) {
  return model_config_has_prediction( enkf_main->model_config );
}



enkf_obs_type * enkf_main_get_obs(const enkf_main_type * enkf_main) {
  return enkf_main->obs;
}


bool enkf_main_have_obs( const enkf_main_type * enkf_main ) {
  return enkf_obs_have_obs( enkf_main->obs );
}



hook_manager_type * enkf_main_get_hook_manager( const enkf_main_type * enkf_main ) {
  return enkf_main->hook_manager;
}



void enkf_main_alloc_obs( enkf_main_type * enkf_main ) {
  enkf_main->obs = enkf_obs_alloc( model_config_get_history(enkf_main->model_config),
                                   model_config_get_external_time_map(enkf_main->model_config),
                                   ecl_config_get_grid( enkf_main->ecl_config ),
                                   ecl_config_get_refcase( enkf_main->ecl_config ) ,
                                   enkf_main->ensemble_config );
}

void enkf_main_load_obs( enkf_main_type * enkf_main , const char * obs_config_file , bool clear_existing) {
  if (clear_existing)
    enkf_obs_clear( enkf_main->obs );

  if (enkf_obs_load(enkf_main->obs ,
                    obs_config_file ,
                    analysis_config_get_std_cutoff(enkf_main->analysis_config))) {
    enkf_main_update_local_updates( enkf_main );
  } else
      fprintf(stderr,"** Warning: failed to load observation data from: %s \n",obs_config_file);
}


/**
   This function should be called when a new data_file has been set.
*/

static void enkf_main_update_num_cpu( enkf_main_type * enkf_main ) {
  /**
     This is how the number of CPU's are passed on to the forward models:
  */
  {
    char * num_cpu_key     = enkf_util_alloc_tagged_string( "NUM_CPU" );
    char * num_cpu_string  = util_alloc_sprintf( "%d" , ecl_config_get_num_cpu( enkf_main->ecl_config ));

    subst_list_append_owned_ref( enkf_main->subst_list , num_cpu_key , num_cpu_string , NULL );
    free( num_cpu_key );
  }
}


ui_return_type * enkf_main_set_data_file( enkf_main_type * enkf_main , const char * data_file ) {
  ui_return_type * ui_return = ecl_config_validate_data_file( enkf_main->ecl_config , data_file );
  if (ui_return_get_status(ui_return) == UI_RETURN_OK) {
    ecl_config_set_data_file( enkf_main->ecl_config , data_file );
    enkf_main_update_num_cpu( enkf_main );
  }
  return ui_return;
}



static void enkf_main_free_ensemble( enkf_main_type * enkf_main ) {
  if (enkf_main->ensemble != NULL) {
    const int ens_size = enkf_main->ens_size;
    int i;
    for (i=0; i < ens_size; i++)
      enkf_state_free( enkf_main->ensemble[i] );
    free(enkf_main->ensemble);
    enkf_main->ensemble = NULL;
  }
}


void enkf_main_free(enkf_main_type * enkf_main){
  if (enkf_main->rng != NULL)
    rng_free( enkf_main->rng );
  rng_config_free( enkf_main->rng_config );

  if (enkf_main->obs)
    enkf_obs_free(enkf_main->obs);

  ranking_table_free( enkf_main->ranking_table );
  enkf_main_free_ensemble( enkf_main );
  enkf_main_close_fs( enkf_main );
  ert_log_close();

  analysis_config_free(enkf_main->analysis_config);
  ecl_config_free(enkf_main->ecl_config);
  model_config_free( enkf_main->model_config);


  hook_manager_free( enkf_main->hook_manager );
  site_config_free( enkf_main->site_config);
  ensemble_config_free( enkf_main->ensemble_config );

  local_config_free( enkf_main->local_config );

  ert_workflow_list_free( enkf_main->workflow_list );


  int_vector_free( enkf_main->keep_runpath );
  plot_config_free( enkf_main->plot_config );
  ert_templates_free( enkf_main->templates );

  subst_func_pool_free( enkf_main->subst_func_pool );
  subst_list_free( enkf_main->subst_list );
  util_safe_free( enkf_main->user_config_file );
  util_safe_free( enkf_main->site_config_file );
  util_safe_free( enkf_main->rft_config_file );
  free(enkf_main);
}



void enkf_main_exit(enkf_main_type * enkf_main) {
  enkf_main_free( enkf_main );
  exit(0);
}


/*****************************************************************/



/**
   This function returns a (enkf_node_type ** ) pointer, which points
   to all the instances with the same keyword, i.e.

   enkf_main_get_node_ensemble(enkf_main , "PRESSURE");

   Will return an ensemble of pressure nodes. Observe that apart from
   the list of pointers, *now new storage* is allocated, all the
   pointers point in to the underlying enkf_node instances under the
   enkf_main / enkf_state objects. Consequently there is no designated
   free() function to match this, just free() the result.

   Example:

   enkf_node_type ** pressure_nodes = enkf_main_get_node_ensemble(enkf_main , "PRESSURE");

   Do something with the pressure nodes ...

   free(pressure_nodes);

*/

enkf_node_type ** enkf_main_get_node_ensemble(const enkf_main_type * enkf_main , enkf_fs_type * src_fs, const char * key , int report_step) {
  const int ens_size              = enkf_main_get_ensemble_size( enkf_main );
  enkf_node_type ** node_ensemble = util_calloc(ens_size , sizeof * node_ensemble );
  node_id_type node_id = {.report_step = report_step ,
                          .iens        = -1 };
  int iens;


  for (iens = 0; iens < ens_size; iens++) {
    node_ensemble[iens] = enkf_state_get_node(enkf_main->ensemble[iens] , key);
    node_id.iens = iens;
    enkf_node_load( node_ensemble[iens] , src_fs , node_id);
  }
  return node_ensemble;
}

/*****************************************************************/




enkf_state_type * enkf_main_iget_state(const enkf_main_type * enkf_main , int iens) {
  return enkf_main->ensemble[iens];
}


member_config_type * enkf_main_iget_member_config(const enkf_main_type * enkf_main , int iens) {
  return enkf_state_get_member_config( enkf_main->ensemble[iens] );
}



void enkf_main_node_mean( const enkf_node_type ** ensemble , int ens_size , enkf_node_type * mean ) {
  int iens;
  enkf_node_clear( mean );
  for (iens = 0; iens < ens_size; iens++)
    enkf_node_iadd( mean , ensemble[iens] );

  enkf_node_scale( mean , 1.0 / ens_size );
}


/**
   This function calculates the node standard deviation from the
   ensemble. The mean can be NULL, in which case it is assumed that
   the mean has already been shifted away from the ensemble.
*/


void enkf_main_node_std( const enkf_node_type ** ensemble , int ens_size , const enkf_node_type * mean , enkf_node_type * std) {
  int iens;
  enkf_node_clear( std );
  for (iens = 0; iens < ens_size; iens++)
    enkf_node_iaddsqr( std , ensemble[iens] );
  enkf_node_scale(std , 1.0 / ens_size );

  if (mean != NULL) {
    enkf_node_scale( std , -1 );
    enkf_node_iaddsqr( std , mean );
    enkf_node_scale( std , -1 );
  }

  enkf_node_sqrt( std );
}


void enkf_main_inflate_node(enkf_main_type * enkf_main , enkf_fs_type * src_fs , enkf_fs_type * target_fs , int report_step , const char * key , const enkf_node_type * min_std) {
  int ens_size                              = enkf_main_get_ensemble_size(enkf_main);
  enkf_node_type ** ensemble                = enkf_main_get_node_ensemble( enkf_main , src_fs , key , report_step );  // Was ANALYZED
  enkf_node_type * mean                     = enkf_node_copyc( ensemble[0] );
  enkf_node_type * std                      = enkf_node_copyc( ensemble[0] );
  int iens;

  /* Shifting away the mean */
  enkf_main_node_mean( (const enkf_node_type **) ensemble , ens_size , mean );
  enkf_node_scale( mean , -1 );
  for (iens = 0; iens < ens_size; iens++)
    enkf_node_iadd( ensemble[iens] , mean );
  enkf_node_scale( mean , -1 );

  /*****************************************************************/
  /*
    Now we have the ensemble represented as a mean and an ensemble of
    deviations from the mean. This is the form suitable for actually
    doing the inflation.
  */
  {
    enkf_node_type * inflation = enkf_node_copyc( ensemble[0] );
    enkf_node_set_inflation( inflation , std , min_std  );

    for (iens = 0; iens < ens_size; iens++)
      enkf_node_imul( ensemble[iens] , inflation );

    enkf_node_free( inflation );
  }

  /* Add the mean back in - and store the updated node to disk.*/
  for (iens = 0; iens < ens_size; iens++) {
    node_id_type node_id = {.report_step = report_step , .iens = iens };
    enkf_node_iadd( ensemble[iens] , mean );
    enkf_node_store( ensemble[iens] , target_fs , true , node_id);
  }

  enkf_node_free( mean );
  enkf_node_free( std );
  free( ensemble );
}



/**
    Denne burde istedet loope gjennom noklene fra use_count
    direkte.
*/

void enkf_main_inflate(enkf_main_type * enkf_main , enkf_fs_type * src_fs , enkf_fs_type * target_fs , int report_step , hash_type * use_count) {
  stringlist_type * keys = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER + DYNAMIC_STATE);

  for (int ikey = 0; ikey < stringlist_get_size( keys ); ikey++) {
    const char * key = stringlist_iget( keys  , ikey );
    if (hash_get_counter(use_count , key) > 0) {
      const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , key );
      const enkf_node_type * min_std            = enkf_config_node_get_min_std( config_node );

      if (min_std != NULL)
        enkf_main_inflate_node(enkf_main , src_fs , target_fs , report_step , key , min_std );

    }
  }
  stringlist_free( keys );
}




static int __get_active_size(const ensemble_config_type * ensemble_config , enkf_fs_type * fs , const char * key, int report_step , const active_list_type * active_list) {
  const enkf_config_node_type * config_node = ensemble_config_get_node( ensemble_config , key );
  /**
     This is very awkward; the problem is that for the GEN_DATA
     type the config object does not really own the size. Instead
     the size is pushed (on load time) from gen_data instances to
     the gen_data_config instance. Therefor we have to assert
     that at least one gen_data instance has been loaded (and
     consequently updated the gen_data_config instance) before we
     query for the size.
  */
  {
    if (enkf_config_node_get_impl_type( config_node ) == GEN_DATA) {
      enkf_node_type * node = enkf_node_alloc( config_node );
      node_id_type node_id = {.report_step = report_step ,
                              .iens        = 0 };

      enkf_node_load( node , fs  , node_id );
      enkf_node_free( node );
    }
  }

  {
    active_mode_type active_mode = active_list_get_mode( active_list );
    int active_size;
    if (active_mode == INACTIVE)
      active_size = 0;
    else if (active_mode == ALL_ACTIVE)
      active_size = enkf_config_node_get_data_size( config_node , report_step );
    else if (active_mode == PARTLY_ACTIVE)
      active_size = active_list_get_active_size( active_list , -1 );
    else {
      util_abort("%s: internal error .. \n",__func__);
      active_size = -1; /* Compiler shut up */
    }
    return active_size;
  }
}


/*****************************************************************/
/**
   Helper struct used to pass information to the multithreaded
   serialize / deserialize functions.
*/

typedef struct {
  enkf_fs_type            * src_fs;
  enkf_fs_type            * target_fs;
  enkf_state_type        ** ensemble;
  int                       iens1;    /* Inclusive lower limit. */
  int                       iens2;    /* NOT inclusive upper limit. */
  const char              * key;
  int                       report_step;
  int                       target_step;
  run_mode_type             run_mode;
  int                       row_offset;
  const active_list_type  * active_list;
  matrix_type             * A;
  const int_vector_type   * iens_active_index;
} serialize_info_type;


static void serialize_node( enkf_fs_type * fs ,
                            enkf_state_type ** ensemble ,
                            const char * key ,
                            int iens ,
                            int report_step ,
                            int row_offset ,
                            int column,
                            const active_list_type * active_list,
                            matrix_type * A) {

  enkf_node_type * node = enkf_state_get_node( ensemble[iens] , key);
  node_id_type node_id = {.report_step = report_step, .iens = iens  };
  enkf_node_serialize( node , fs , node_id , active_list , A , row_offset , column);
}


static void * serialize_nodes_mt( void * arg ) {
  serialize_info_type * info = (serialize_info_type *) arg;
  int iens;
  for (iens = info->iens1; iens < info->iens2; iens++) {
    int column = int_vector_iget( info->iens_active_index , iens);
    if (column >= 0)
      serialize_node( info->src_fs ,
                      info->ensemble ,
                      info->key ,
                      iens ,
                      info->report_step ,
                      info->row_offset ,
                      column,
                      info->active_list ,
                      info->A );
  }
  return NULL;
}


static void enkf_main_serialize_node( const char * node_key ,
                                      const active_list_type * active_list ,
                                      int row_offset ,
                                      thread_pool_type * work_pool ,
                                      serialize_info_type * serialize_info) {

  /* Multithreaded serializing*/
  const int num_cpu_threads = thread_pool_get_max_running( work_pool );
  int icpu;

  thread_pool_restart( work_pool );
  for (icpu = 0; icpu < num_cpu_threads; icpu++) {
    serialize_info[icpu].key         = node_key;
    serialize_info[icpu].active_list = active_list;
    serialize_info[icpu].row_offset  = row_offset;

    thread_pool_add_job( work_pool , serialize_nodes_mt , &serialize_info[icpu]);
  }
  thread_pool_join( work_pool );
}



/**
   The return value is the number of rows in the serialized
   A matrix.
*/

static int enkf_main_serialize_dataset( const ensemble_config_type * ens_config ,
                                        const local_dataset_type * dataset ,
                                        int report_step,
                                        hash_type * use_count ,
                                        int * active_size ,
                                        int * row_offset,
                                        thread_pool_type * work_pool,
                                        serialize_info_type * serialize_info) {

  matrix_type * A   = serialize_info->A;
  stringlist_type * update_keys = local_dataset_alloc_keys( dataset );
  const int num_kw  = stringlist_get_size( update_keys );
  int ens_size      = matrix_get_columns( A );
  int current_row   = 0;

  for (int ikw=0; ikw < num_kw; ikw++) {
    const char             * key         = stringlist_iget(update_keys , ikw);
    enkf_config_node_type * config_node  = ensemble_config_get_node( ens_config , key );
    if ((serialize_info[0].run_mode == SMOOTHER_UPDATE) && (enkf_config_node_get_var_type( config_node ) != PARAMETER)) {
      /* We have tried to serialize a dynamic node when we are
         smoother update mode; that does not make sense and we just
         continue. */
      active_size[ikw] = 0;
      continue;
    } else {
      const active_list_type * active_list      = local_dataset_get_node_active_list( dataset , key );
      enkf_fs_type * src_fs = serialize_info->src_fs;
      active_size[ikw] = __get_active_size( ens_config , src_fs , key , report_step , active_list );
      row_offset[ikw]  = current_row;

      {
        int matrix_rows = matrix_get_rows( A );
        if ((active_size[ikw] + current_row) > matrix_rows)
          matrix_resize( A , matrix_rows + 2 * active_size[ikw] , ens_size , true );
      }

      if (active_size[ikw] > 0) {
        enkf_main_serialize_node( key , active_list , row_offset[ikw] , work_pool , serialize_info );
        current_row += active_size[ikw];
      }
    }
  }
  matrix_shrink_header( A , current_row , ens_size );
  stringlist_free( update_keys );
  return matrix_get_rows( A );
}

static void deserialize_node( enkf_fs_type            * fs,
                              enkf_state_type ** ensemble ,
                              const char * key ,
                              int iens,
                              int target_step ,
                              int row_offset ,
                              int column,
                              const active_list_type * active_list,
                              matrix_type * A) {

  enkf_node_type * node = enkf_state_get_node( ensemble[iens] , key);
  node_id_type node_id = { .report_step = target_step , .iens = iens };
  enkf_node_deserialize(node , fs , node_id , active_list , A , row_offset , column);
  state_map_update_undefined(enkf_fs_get_state_map(fs) , iens , STATE_INITIALIZED);
}



static void * deserialize_nodes_mt( void * arg ) {
  serialize_info_type * info = (serialize_info_type *) arg;
  int iens;
  for (iens = info->iens1; iens < info->iens2; iens++) {
    int column = int_vector_iget( info->iens_active_index , iens );
    if (column >= 0)
      deserialize_node( info->target_fs , info->ensemble , info->key , iens , info->target_step , info->row_offset , column, info->active_list , info->A );
  }
  return NULL;
}


static void enkf_main_deserialize_dataset( ensemble_config_type * ensemble_config ,
                                           const local_dataset_type * dataset ,
                                           const int * active_size ,
                                           const int * row_offset ,
                                           serialize_info_type * serialize_info ,
                                           thread_pool_type * work_pool ) {

  int num_cpu_threads = thread_pool_get_max_running( work_pool );
  stringlist_type * update_keys = local_dataset_alloc_keys( dataset );
  for (int i = 0; i < stringlist_get_size( update_keys ); i++) {
    const char             * key         = stringlist_iget(update_keys , i);
    enkf_config_node_type * config_node  = ensemble_config_get_node( ensemble_config , key );
    if ((serialize_info[0].run_mode == SMOOTHER_UPDATE) && (enkf_config_node_get_var_type( config_node ) != PARAMETER))
      /*
         We have tried to serialize a dynamic node when we are in
         smoother update mode; that does not make sense and we just
         continue.
      */
      continue;
    else {
      if (active_size[i] > 0) {
        const active_list_type * active_list      = local_dataset_get_node_active_list( dataset , key );

        {
          /* Multithreaded */
          int icpu;
          thread_pool_restart( work_pool );
          for (icpu = 0; icpu < num_cpu_threads; icpu++) {
            serialize_info[icpu].key         = key;
            serialize_info[icpu].active_list = active_list;
            serialize_info[icpu].row_offset  = row_offset[i];

            thread_pool_add_job( work_pool , deserialize_nodes_mt , &serialize_info[icpu]);
          }
          thread_pool_join( work_pool );
        }
      }
    }
  }
  stringlist_free( update_keys );
}


static void serialize_info_free( serialize_info_type * serialize_info ) {
  free( serialize_info );
}

static serialize_info_type * serialize_info_alloc( enkf_fs_type * src_fs,
                                                   enkf_fs_type * target_fs ,
                                                   const int_vector_type * iens_active_index ,
                                                   int target_step ,
                                                   enkf_state_type ** ensemble ,
                                                   run_mode_type run_mode ,
                                                   int report_step ,
                                                   matrix_type * A ,
                                                   int num_cpu_threads ) {

  serialize_info_type * serialize_info = util_calloc( num_cpu_threads , sizeof * serialize_info );
  int ens_size = int_vector_size(iens_active_index);
  int icpu;
  int iens_offset = 0;
  for (icpu = 0; icpu < num_cpu_threads; icpu++) {
    serialize_info[icpu].iens_active_index = iens_active_index;
    serialize_info[icpu].run_mode    = run_mode;
    serialize_info[icpu].src_fs      = src_fs;
    serialize_info[icpu].target_fs   = target_fs;
    serialize_info[icpu].target_step = target_step;
    serialize_info[icpu].ensemble    = ensemble;
    serialize_info[icpu].report_step = report_step;
    serialize_info[icpu].A           = A;
    serialize_info[icpu].iens1       = iens_offset;
    serialize_info[icpu].iens2       = iens_offset + (ens_size - iens_offset) / (num_cpu_threads - icpu);
    iens_offset = serialize_info[icpu].iens2;
  }
  serialize_info[num_cpu_threads - 1].iens2 = ens_size;
  return serialize_info;
}

static module_info_type * enkf_main_module_info_alloc( const local_ministep_type* ministep,
                                                       const obs_data_type * obs_data,
                                                       const local_dataset_type * dataset ,
                                                       const local_obsdata_type   * local_obsdata ,
                                                       int * active_size ,
                                                       int * row_offset)
{
  // Create and initialize the module_info instance.
  module_info_type * module_info = module_info_alloc(local_ministep_get_name(ministep));

  { /* Init data blocks in module_info */
    stringlist_type * update_keys = local_dataset_alloc_keys( dataset );
    const int num_kw  = stringlist_get_size( update_keys );
    module_data_block_vector_type * module_data_block_vector = module_info_get_data_block_vector(module_info);

    for (int ikw=0; ikw < num_kw; ikw++) {
      const char             * key         = stringlist_iget(update_keys , ikw);
      const active_list_type * active_list      = local_dataset_get_node_active_list( dataset , key );
      const module_data_block_type * data_block = module_data_block_alloc( key, active_list_get_active(active_list),  row_offset[ikw], active_size[ikw] );
      module_data_block_vector_add_data_block(module_data_block_vector, data_block);
    }
    stringlist_free( update_keys );
  }


  { /* Init obs blocks in module_info */
    module_obs_block_vector_type  * module_obs_block_vector =  module_info_get_obs_block_vector ( module_info );
    int current_row = 0;
    for (int block_nr = 0; block_nr < local_obsdata_get_size( local_obsdata ); block_nr++) {
      const obs_block_type  * obs_block  = obs_data_iget_block_const( obs_data , block_nr);
      int total_size =  obs_block_get_size(obs_block);
      local_obsdata_node_type * node   = local_obsdata_iget ( local_obsdata, block_nr );
      const char * key  = local_obsdata_node_get_key ( node );
      const active_list_type * active_list      = local_obsdata_node_get_active_list( node );
      int n_active = active_list_get_active_size(active_list, total_size);
      module_obs_block_type * module_obs_block = module_obs_block_alloc(key, active_list_get_active(active_list), current_row, n_active);
      module_obs_block_vector_add_obs_block ( module_obs_block_vector, module_obs_block );
      current_row += n_active;
    }
  }

  return module_info;
}

static void enkf_main_module_info_free( module_info_type * module_info ) {
  free( module_info );
}

void enkf_main_fprintf_PC(const char * filename ,
                          matrix_type * PC ,
                          matrix_type * PC_obs) {

  FILE * stream   = util_mkdir_fopen(filename , "w");
  const int num_PC   = matrix_get_rows( PC );
  const int ens_size = matrix_get_columns( PC );
  int ipc,iens;

  for (ipc = 0; ipc < num_PC; ipc++)
    fprintf(stream , "%10.6f " , matrix_iget( PC_obs , ipc , 0));
  fprintf(stream , "\n");

  for (iens = 0; iens < ens_size; iens++) {
      for (ipc = 0; ipc < num_PC; ipc++)
        fprintf(stream ,"%10.6f " , matrix_iget( PC , ipc, iens ));
      fprintf(stream , "\n");
  }
  fclose( stream );
}


void enkf_main_get_PC( const matrix_type * S,
                       const matrix_type * dObs,
                       double truncation ,
                       int ncomp ,
                       matrix_type * PC ,
                       matrix_type * PC_obs ,
                       double_vector_type * singular_values) {

  enkf_linalg_get_PC( S , dObs , truncation , ncomp , PC , PC_obs , singular_values);
}







static void assert_matrix_size(const matrix_type * m , const char * name , int rows , int columns) {
  if (m) {
    if (!matrix_check_dims(m , rows , columns))
      util_abort("%s: matrix mismatch %s:[%d,%d]   - expected:[%d, %d]", __func__ , name , matrix_get_rows(m) , matrix_get_columns(m) , rows , columns);
  } else
    util_abort("%s: matrix:%s is NULL \n",__func__ , name);
}

static void assert_size_equal(int ens_size , const bool_vector_type * ens_mask) {
  if (bool_vector_size( ens_mask ) != ens_size)
    util_abort("%s: fundamental inconsisentcy detected. Total ens_size:%d  mask_size:%d \n",__func__ , ens_size , bool_vector_size( ens_mask ));
}


static void enkf_main_analysis_update( enkf_main_type * enkf_main ,
                                       enkf_fs_type * target_fs ,
                                       const bool_vector_type * ens_mask ,
                                       int target_step ,
                                       hash_type * use_count,
                                       run_mode_type run_mode ,
                                       int step1 ,
                                       int step2 ,
                                       const local_ministep_type * ministep ,
                                       const meas_data_type * forecast ,
                                       obs_data_type * obs_data) {

  const int cpu_threads       = 4;
  const int matrix_start_size = 250000;
  thread_pool_type * tp       = thread_pool_alloc( cpu_threads , false );
  int active_ens_size   = meas_data_get_active_ens_size( forecast );
  int active_size       = obs_data_get_active_size( obs_data );
  matrix_type * X       = matrix_alloc( active_ens_size , active_ens_size );
  matrix_type * S       = meas_data_allocS( forecast );
  matrix_type * R       = obs_data_allocR( obs_data );
  matrix_type * dObs    = obs_data_allocdObs( obs_data );
  matrix_type * A       = matrix_alloc( matrix_start_size , active_ens_size );
  matrix_type * E       = NULL;
  matrix_type * D       = NULL;
  matrix_type * localA  = NULL;
  int_vector_type * iens_active_index = bool_vector_alloc_active_index_list(ens_mask , -1);

  analysis_module_type * module = analysis_config_get_active_module( enkf_main->analysis_config );
  if ( local_ministep_has_analysis_module (ministep))
    module = local_ministep_get_analysis_module (ministep);

  assert_matrix_size(X , "X" , active_ens_size , active_ens_size);
  assert_matrix_size(S , "S" , active_size , active_ens_size);
  assert_matrix_size(R , "R" , active_size , active_size);
  assert_size_equal( enkf_main_get_ensemble_size( enkf_main ) , ens_mask );

  if (analysis_module_check_option( module , ANALYSIS_NEED_ED)) {
    E = obs_data_allocE( obs_data , enkf_main->rng , active_ens_size );
    D = obs_data_allocD( obs_data , E , S );

    assert_matrix_size( E , "E" , active_size , active_ens_size);
    assert_matrix_size( D , "D" , active_size , active_ens_size);
  }

  if (analysis_module_check_option( module , ANALYSIS_SCALE_DATA))
    obs_data_scale( obs_data , S , E , D , R , dObs );

  if (analysis_module_check_option( module , ANALYSIS_USE_A) || analysis_module_check_option(module , ANALYSIS_UPDATE_A))
    localA = A;

  /*****************************************************************/

  analysis_module_init_update( module , ens_mask , S , R , dObs , E , D );
  {
    hash_iter_type * dataset_iter = local_ministep_alloc_dataset_iter( ministep );
    serialize_info_type * serialize_info = serialize_info_alloc( target_fs, //src_fs - we have already copied the parameters from the src_fs to the target_fs
                                                                 target_fs ,
                                                                 iens_active_index,
                                                                 target_step ,
                                                                 enkf_main_get_ensemble( enkf_main ) ,
                                                                 run_mode ,
                                                                 step2 ,
                                                                 A ,
                                                                 cpu_threads);


    // Store PC:
    if (analysis_config_get_store_PC( enkf_main->analysis_config )) {
      double truncation    = -1;
      int ncomp            = active_ens_size - 1;
      matrix_type * PC     = matrix_alloc(1,1);
      matrix_type * PC_obs = matrix_alloc(1,1);
      double_vector_type   * singular_values = double_vector_alloc(0,0);
      local_obsdata_type   * obsdata = local_ministep_get_obsdata( ministep );
      const char * obsdata_name = local_obsdata_get_name( obsdata );

      enkf_main_get_PC( S , dObs , truncation , ncomp , PC , PC_obs , singular_values);
      {
        char * filename  = util_alloc_sprintf(analysis_config_get_PC_filename( enkf_main->analysis_config ) , step1 , step2 , obsdata_name);
        char * full_path = util_alloc_filename( analysis_config_get_PC_path( enkf_main->analysis_config) , filename , NULL );

        enkf_main_fprintf_PC( full_path , PC , PC_obs);

        free( full_path );
        free( filename );
      }
      matrix_free( PC );
      matrix_free( PC_obs );
      double_vector_free( singular_values );
    }

    if (localA == NULL)
      analysis_module_initX( module , X , NULL , S , R , dObs , E , D );


    while (!hash_iter_is_complete( dataset_iter )) {
      const char * dataset_name = hash_iter_get_next_key( dataset_iter );
      const local_dataset_type * dataset = local_ministep_get_dataset( ministep , dataset_name );
      if (local_dataset_get_size( dataset )) {
        int * active_size = util_calloc( local_dataset_get_size( dataset ) , sizeof * active_size );
        int * row_offset  = util_calloc( local_dataset_get_size( dataset ) , sizeof * row_offset  );
        local_obsdata_type   * local_obsdata = local_ministep_get_obsdata( ministep );

        enkf_main_serialize_dataset( enkf_main->ensemble_config , dataset , step2 ,  use_count , active_size , row_offset , tp , serialize_info);
        module_info_type * module_info = enkf_main_module_info_alloc(ministep, obs_data, dataset, local_obsdata, active_size , row_offset);

        if (analysis_module_check_option( module , ANALYSIS_UPDATE_A)){
          if (analysis_module_check_option( module , ANALYSIS_ITERABLE)){
            analysis_module_updateA( module , localA , S , R , dObs , E , D , module_info );
          }
          else
            analysis_module_updateA( module , localA , S , R , dObs , E , D , module_info );
        }
        else {
          if (analysis_module_check_option( module , ANALYSIS_USE_A)){
            analysis_module_initX( module , X , localA , S , R , dObs , E , D );
          }

          matrix_inplace_matmul_mt2( A , X , tp );
        }

        // The deserialize also calls enkf_node_store() functions.
        enkf_main_deserialize_dataset( enkf_main_get_ensemble_config( enkf_main ) , dataset , active_size , row_offset , serialize_info , tp);

        free( active_size );
        free( row_offset );
        enkf_main_module_info_free( module_info );
      }
    }
    hash_iter_free( dataset_iter );
    serialize_info_free( serialize_info );
  }
  analysis_module_complete_update( module );


  /*****************************************************************/

  int_vector_free(iens_active_index);
  matrix_safe_free( E );
  matrix_safe_free( D );
  matrix_free( S );
  matrix_free( R );
  matrix_free( dObs );
  matrix_free( X );
  matrix_free( A );
}




/**
   This is  T H E  EnKF update routine.
**/


bool enkf_main_UPDATE(enkf_main_type * enkf_main , const int_vector_type * step_list, enkf_fs_type * source_fs , enkf_fs_type * target_fs , int target_step , run_mode_type run_mode) {
  /*
     If merge_observations is true all observations in the time
     interval [step1+1,step2] will be used, otherwise only the last
     observation at step2 will be used.
  */
  state_map_type * source_state_map = enkf_fs_get_state_map( source_fs );
  const analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  const int active_ens_size = state_map_count_matching( source_state_map , STATE_HAS_DATA );

  const int total_ens_size = enkf_main_get_ensemble_size(enkf_main);
  if (analysis_config_have_enough_realisations(analysis_config , active_ens_size, total_ens_size)) {
    double alpha       = analysis_config_get_alpha( enkf_main->analysis_config );
    double std_cutoff  = analysis_config_get_std_cutoff( enkf_main->analysis_config );
    int current_step   = int_vector_get_last( step_list );
    state_map_type * target_state_map = enkf_fs_get_state_map( target_fs );
    bool_vector_type * ens_mask = bool_vector_alloc(total_ens_size , false);
    int_vector_type * ens_active_list = int_vector_alloc(0,0);


    state_map_select_matching( source_state_map , ens_mask , STATE_HAS_DATA );
    ens_active_list = bool_vector_alloc_active_list( ens_mask );
    {
      /*
        Observations and measurements are collected in these temporary
        structures. obs_data is a precursor for the 'd' vector, and
        meas_data is a precursor for the 'S' matrix'.

        The reason for going via these temporary structures is to support
        deactivating observations which should not be used in the update
        process.
      */
      double global_std_scaling = analysis_config_get_global_std_scaling(analysis_config);
      obs_data_type               * obs_data      = obs_data_alloc(global_std_scaling);
      meas_data_type              * meas_data     = meas_data_alloc( ens_mask );
      local_config_type           * local_config  = enkf_main->local_config;
      const local_updatestep_type * updatestep    = local_config_get_updatestep( local_config );
      hash_type                   * use_count     = hash_alloc();
      const char                  * log_path      = analysis_config_get_log_path( enkf_main->analysis_config );
      FILE                        * log_stream;



      /* Copy all the parameter nodes from source case to target case;
         nodes which are updated will be fetched from the new target
         case, and nodes which are not updated will be manually copied
         over there.
      */

      if (target_fs != source_fs) {
	stringlist_type * param_keys = ensemble_config_alloc_keylist_from_var_type(enkf_main->ensemble_config, PARAMETER );
	for (int i=0; i < stringlist_get_size( param_keys ); i++) {
	  const char * key = stringlist_iget( param_keys , i );
          enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , key );
          enkf_node_type * data_node = enkf_node_alloc( config_node );
          for (int j=0; j < int_vector_size(ens_active_list); j++) {
            node_id_type node_id = {.iens = int_vector_iget( ens_active_list , j ),
                                    .report_step = 0 };
            enkf_node_load( data_node , source_fs , node_id );
            enkf_node_store( data_node , target_fs , false , node_id );
          }
          enkf_node_free( data_node );
	}
	stringlist_free( param_keys );
      }


      if ((local_updatestep_get_num_ministep( updatestep ) > 1) &&
          (analysis_config_get_module_option( analysis_config , ANALYSIS_ITERABLE))) {
            util_exit("** ERROR: Can not combine iterable modules with multi step updates - sorry\n");
          }


      {
        char * log_file;
        if (int_vector_size( step_list ) == 1)
          log_file = util_alloc_sprintf("%s%c%04d" , log_path , UTIL_PATH_SEP_CHAR , int_vector_iget( step_list , 0));
        else
          log_file = util_alloc_sprintf("%s%c%04d-%04d" , log_path , UTIL_PATH_SEP_CHAR , int_vector_iget( step_list , 0) , int_vector_get_last( step_list ));
        log_stream = util_fopen( log_file , "w" );

        free( log_file );
      }

      for (int ministep_nr = 0; ministep_nr < local_updatestep_get_num_ministep( updatestep ); ministep_nr++) {   /* Looping over local analysis ministep */
        local_ministep_type * ministep = local_updatestep_iget_ministep( updatestep , ministep_nr );
        local_obsdata_type   * obsdata = local_ministep_get_obsdata( ministep );

        obs_data_reset( obs_data );
        meas_data_reset( meas_data );

        /*
           Temporarily we will just force the timestep from the input
           argument onto the obsdata instance; in the future the
           obsdata should hold it's own here.
        */
        local_obsdata_reset_tstep_list(obsdata, step_list);

        if (analysis_config_get_std_scale_correlated_obs(enkf_main->analysis_config)) {
          double scale_factor = enkf_obs_scale_correlated_std(enkf_main->obs, source_fs, ens_active_list, obsdata);
          ert_log_add_fmt_message(1, NULL, "Scaling standard deviation in obdsata set:%s with %g", local_obsdata_get_name(obsdata) , scale_factor);
        }

        enkf_obs_get_obs_and_measure_data( enkf_main->obs,
                                           source_fs ,
                                           obsdata,
                                           ens_active_list ,
                                           meas_data,
                                           obs_data);



        enkf_analysis_deactivate_outliers( obs_data , meas_data  , std_cutoff , alpha , enkf_main->verbose);

        if (enkf_main->verbose)
          enkf_analysis_fprintf_obs_summary( obs_data , meas_data  , step_list , local_ministep_get_name( ministep ) , stdout );
        enkf_analysis_fprintf_obs_summary( obs_data , meas_data  , step_list , local_ministep_get_name( ministep ) , log_stream );

        if ((obs_data_get_active_size(obs_data) > 0) && (meas_data_get_active_obs_size( meas_data ) > 0))
          enkf_main_analysis_update( enkf_main ,
                                     target_fs ,
                                     ens_mask ,
                                     target_step ,
                                     use_count ,
                                     run_mode ,
                                     int_vector_get_first( step_list ),
                                     current_step ,
                                     ministep ,
                                     meas_data ,
                                     obs_data );
        else if (target_fs != source_fs)
          ert_log_add_fmt_message( 1 , stderr , "No active observations/parameters for MINISTEP: %s." , local_ministep_get_name(ministep));
      }
      fclose( log_stream );

      obs_data_free( obs_data );
      meas_data_free( meas_data );

      enkf_main_inflate( enkf_main , source_fs , target_fs , current_step , use_count);
      hash_free( use_count );

      if (target_state_map != source_state_map) {
        state_map_set_from_inverted_mask( target_state_map , ens_mask , STATE_PARENT_FAILURE);
        state_map_set_from_mask( target_state_map , ens_mask , STATE_INITIALIZED );
        enkf_fs_fsync( target_fs );
      }
    }
    bool_vector_free( ens_mask );
    int_vector_free( ens_active_list );
    return true;
  } else {
    fprintf(stderr,"** ERROR ** There are %d active realisations left, which is less than the minimum specified - stopping assimilation.\n" ,
            active_ens_size );
    return false;
  }

}


static bool enkf_main_smoother_update__(enkf_main_type * enkf_main , const int_vector_type * step_list , enkf_fs_type * source_fs, enkf_fs_type * target_fs) {
  return enkf_main_UPDATE( enkf_main , step_list , source_fs , target_fs , 0 , SMOOTHER_UPDATE );
}


bool enkf_main_smoother_update(enkf_main_type * enkf_main , enkf_fs_type * source_fs, enkf_fs_type * target_fs) {
  int stride = 1;
  int step2;
  time_map_type * time_map = enkf_fs_get_time_map( source_fs );
  int_vector_type * step_list;
  bool update_done;

  step2 = time_map_get_last_step( time_map );
  if (step2 < 0)
    step2 = model_config_get_last_history_restart( enkf_main->model_config );

  step_list = enkf_main_update_alloc_step_list( enkf_main , 0 , step2 , stride);
  update_done = enkf_main_smoother_update__( enkf_main , step_list , source_fs, target_fs );
  int_vector_free( step_list );

  return update_done;
}


static void enkf_main_monitor_job_queue ( const enkf_main_type * enkf_main) {
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  if (analysis_config_get_stop_long_running(analysis_config)) {
    job_queue_type * job_queue = site_config_get_job_queue(enkf_main->site_config);

    bool cont = true;
    while (cont) {
      //Check if minimum number of realizations have run, and if so, kill the rest after a certain time
      if (analysis_config_have_enough_realisations(analysis_config, job_queue_get_num_complete(job_queue), enkf_main_get_ensemble_size(enkf_main))) {
        job_queue_set_auto_job_stop_time(job_queue);
        cont = false;
      }

      //Check if all possible successes satisfies the minimum number of realizations threshold. If not so, it is time to give up
      int possible_successes = job_queue_get_num_running(job_queue) +
        job_queue_get_num_waiting(job_queue) +
        job_queue_get_num_pending(job_queue) +
        job_queue_get_num_complete(job_queue);

      
      if (analysis_config_have_enough_realisations(analysis_config, possible_successes, enkf_main_get_ensemble_size(enkf_main))) {
        cont = false;
      }

      if (cont) {
        util_usleep(10000);
      }
    }
  }
}



void enkf_main_isubmit_job( enkf_main_type * enkf_main , run_arg_type * run_arg ) {
  const ecl_config_type * ecl_config = enkf_main_get_ecl_config( enkf_main );
  enkf_state_type * enkf_state = enkf_main->ensemble[ run_arg_get_iens(run_arg) ];
  const member_config_type  * member_config = enkf_state_get_member_config( enkf_state );
  const site_config_type    * site_config   = enkf_main_get_site_config( enkf_main );
  const char * job_script                   = site_config_get_job_script( site_config );
  job_queue_type * job_queue                = site_config_get_job_queue( site_config );
  const char * run_path                     = run_arg_get_runpath( run_arg );

  // The job_queue_node will take ownership of this arg_pack; and destroy it when
  // the job_queue_node is discarded.
  arg_pack_type             * callback_arg      = arg_pack_alloc();

  /*
    Prepare the job and submit it to the queue
  */
  arg_pack_append_ptr( callback_arg , enkf_state );
  arg_pack_append_ptr( callback_arg , run_arg );

  {
    int queue_index = job_queue_add_job( job_queue ,
                                         job_script ,
                                         enkf_state_complete_forward_modelOK__ ,
                                         enkf_state_complete_forward_modelRETRY__ ,
                                         enkf_state_complete_forward_modelEXIT__,
                                         callback_arg ,
                                         ecl_config_get_num_cpu( ecl_config ),
                                         run_path ,
                                         member_config_get_jobname( member_config ) ,
                                         1,
                                         (const char *[1]) { run_path } );

    run_arg_set_queue_index( run_arg , queue_index );
    run_arg_increase_submit_count( run_arg );
  }

}

void * enkf_main_icreate_run_path( enkf_main_type * enkf_main, run_arg_type * run_arg){
  enkf_state_type * enkf_state = enkf_main->ensemble[ run_arg_get_iens(run_arg) ];
  {
    runpath_list_type * runpath_list = hook_manager_get_runpath_list( enkf_main->hook_manager );
    runpath_list_add( runpath_list ,
                      run_arg_get_iens( run_arg ),
                      run_arg_get_iter( run_arg ),
                      run_arg_get_runpath( run_arg ),
                      enkf_state_get_eclbase( enkf_state ));
  }
  enkf_state_init_eclipse( enkf_state , run_arg );
  return NULL;
}


static void * enkf_main_create_run_path__( enkf_main_type * enkf_main,
                                           const ert_init_context_type * init_context) {

  const bool_vector_type * iactive = ert_init_context_get_iactive(init_context);
  const int active_ens_size = util_int_min( bool_vector_size( iactive ) , enkf_main_get_ensemble_size( enkf_main ));
  int iens;
  for (iens = 0; iens < active_ens_size; iens++) {
    if (bool_vector_iget(iactive , iens)) {
      run_arg_type * run_arg = ert_init_context_iens_get_arg( init_context , iens);
      enkf_main_icreate_run_path(enkf_main, run_arg);
    }
  }
  return NULL;
}

void enkf_main_create_run_path(enkf_main_type * enkf_main , const bool_vector_type * iactive , int iter) {
  init_mode_type init_mode = INIT_CONDITIONAL;

  enkf_main_init_internalization(enkf_main , init_mode);
  {
    stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
    enkf_main_initialize_from_scratch(enkf_main ,
				      enkf_main_get_fs( enkf_main ),
				      param_list ,
				      iactive,
				      init_mode);
    stringlist_free( param_list );
  }


  {
    ert_init_context_type * init_context = enkf_main_alloc_ert_init_context( enkf_main ,
                                                                             enkf_main_get_fs( enkf_main ),
                                                                             iactive ,
                                                                             init_mode ,
                                                                             iter );
    enkf_main_create_run_path__( enkf_main , init_context );
    ert_init_context_free( init_context );
  }


  /*
    The runpath_list is written to disk here, when all the simulation
    folders have been created and filled with content. The
    runpath_list instance is owned and managed by the hook manager;
    could say that the responsability for writing that data should be
    with the hook_manager?
  */

  {
    runpath_list_type * runpath_list = hook_manager_get_runpath_list(enkf_main->hook_manager );
    runpath_list_fprintf( runpath_list );
  }
}



void * enkf_main_isubmit_job__( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg_pack_iget_ptr( arg_pack , 0 ));
  run_arg_type * run_arg = run_arg_safe_cast( arg_pack_iget_ptr( arg_pack , 1));

  enkf_main_isubmit_job( enkf_main , run_arg );
  return NULL;
}





static void enkf_main_submit_jobs__( enkf_main_type * enkf_main ,
                                     const ert_run_context_type * run_context ,
                                     thread_pool_type * submit_threads,
                                     arg_pack_type ** arg_pack_list) {
  {
    int iens;
    const bool_vector_type * iactive = ert_run_context_get_iactive( run_context );
    const int active_ens_size = util_int_min( bool_vector_size( iactive ) , enkf_main_get_ensemble_size( enkf_main ));

    for (iens = 0; iens < active_ens_size; iens++) {
      if (bool_vector_iget(iactive , iens)) {
        run_arg_type * run_arg = ert_run_context_iens_get_arg( run_context , iens);
        arg_pack_type * arg_pack = arg_pack_list[iens];

        arg_pack_append_ptr( arg_pack , enkf_main );
        arg_pack_append_ptr( arg_pack , run_arg);

        run_arg_set_run_status( run_arg, JOB_SUBMITTED );
        thread_pool_add_job(submit_threads , enkf_main_isubmit_job__ , arg_pack);
      }
    }
  }
}


void enkf_main_submit_jobs( enkf_main_type * enkf_main ,
                            const ert_run_context_type * run_context) {

  int ens_size = enkf_main_get_ensemble_size( enkf_main );
  arg_pack_type ** arg_pack_list = util_malloc( ens_size * sizeof * arg_pack_list );
  thread_pool_type * submit_threads = thread_pool_alloc( 4 , true );
  runpath_list_type * runpath_list = hook_manager_get_runpath_list( enkf_main->hook_manager );
  int iens;
  for (iens = 0; iens < ens_size; iens++)
    arg_pack_list[iens] = arg_pack_alloc( );

  runpath_list_clear( runpath_list );
  enkf_main_submit_jobs__(enkf_main , run_context , submit_threads , arg_pack_list);

  /*
    After this join all directories/files for the simulations
    have been set up correctly, and all the jobs have been added
    to the job_queue manager.
  */

  thread_pool_join(submit_threads);
  thread_pool_free(submit_threads);

  for (iens = 0; iens < ens_size; iens++)
    arg_pack_free( arg_pack_list[iens] );
  free( arg_pack_list );
}



/**
  The function will return number of non-failing jobs.
*/


static int enkf_main_run_step(enkf_main_type * enkf_main       ,
                               ert_run_context_type * run_context) {

  if (ert_run_context_get_step1(run_context))
    ecl_config_assert_restart( enkf_main_get_ecl_config( enkf_main ) );

  {
    int job_size , iens;
    bool     verbose_queue    = enkf_main->verbose;
    const int active_ens_size = util_int_min( bool_vector_size( ert_run_context_get_iactive( run_context )) , enkf_main_get_ensemble_size( enkf_main ));

    state_map_deselect_matching( enkf_fs_get_state_map( ert_run_context_get_init_fs( run_context )) ,
                                 ert_run_context_get_iactive( run_context ), STATE_LOAD_FAILURE | STATE_PARENT_FAILURE);

    ert_log_add_fmt_message( 1 , NULL , "===================================================================", false);

    job_size = bool_vector_count_equal( ert_run_context_get_iactive(run_context) , true );
    {
      job_queue_type * job_queue = site_config_get_job_queue(enkf_main->site_config);
      job_queue_manager_type * queue_manager = job_queue_manager_alloc( job_queue );
      bool restart_queue = true;

      /* Start the queue */
      if (site_config_has_job_script( enkf_main->site_config ))
        job_queue_manager_start_queue( queue_manager , job_size , verbose_queue , restart_queue);
      else
        util_exit("No job script specified, can not start any jobs. Use the key JOB_SCRIPT in the config file\n");


      enkf_main_submit_jobs( enkf_main , run_context );


      job_queue_submit_complete( job_queue );
      ert_log_add_message( 1 , NULL , "All jobs submitted to internal queue - waiting for completion" ,  false);

      int max_runtime = analysis_config_get_max_runtime(enkf_main_get_analysis_config( enkf_main ));
      job_queue_set_max_job_duration(job_queue, max_runtime);
      enkf_main_monitor_job_queue( enkf_main );

      job_queue_manager_wait( queue_manager );
      job_queue_manager_free( queue_manager );
    }


    /* This should be carefully checked for the situation where only a
       subset (with offset > 0) of realisations are simulated. */

    int totalOK = 0;
    int totalFailed = 0;
    for (iens = 0; iens < active_ens_size; iens++) {
      if (bool_vector_iget(ert_run_context_get_iactive(run_context) , iens)) {
        run_arg_type * run_arg = ert_run_context_iens_get_arg( run_context , iens );
        run_status_type run_status = run_arg_get_run_status( run_arg );

        if ((run_status == JOB_LOAD_FAILURE) || (run_status == JOB_RUN_FAILURE)) {
          ert_run_context_deactivate_realization(run_context, iens);
          totalFailed++;
        }
        else {
          totalOK++;
        }
      }
    }
    
    enkf_fs_fsync( ert_run_context_get_result_fs( run_context ) );
    if (totalFailed == 0)
      ert_log_add_fmt_message( 1 , NULL , "All jobs complete and data loaded.");


    return totalOK;
  }
}

/**
   The special value stride == 0 means to just include step2.
*/
int_vector_type * enkf_main_update_alloc_step_list( const enkf_main_type * enkf_main , int load_start , int step2 , int stride) {
  int_vector_type * step_list = int_vector_alloc( 0 , 0 );

  if (step2 < load_start)
    util_abort("%s: fatal internal error: Tried to make step list %d ... %d \n",__func__ , load_start , step2);

  if (stride == 0)
    int_vector_append( step_list , step2 );
  else {
    int step = util_int_max( 1 , load_start );
    while (true) {
      int_vector_append( step_list , step );

      if (step == step2)
        break;
      else {
        step += stride;
        if (step >= step2) {
          int_vector_append( step_list , step2 );
          break;
        }
      }

    }
  }
  return step_list;
}




void * enkf_main_get_enkf_config_node_type(const ensemble_config_type * ensemble_config, const char * key){
  enkf_config_node_type * config_node_type = ensemble_config_get_node(ensemble_config, key);
  return enkf_config_node_get_ref(config_node_type);
}


/**
   This function will initialize the necessary enkf_main structures
   before a run. Currently this means:

     1. Set the enkf_sched instance - either by loading from file or
        by using the default.

     2. Set up the configuration of what should be internalized.

*/


void enkf_main_init_run( enkf_main_type * enkf_main, const ert_run_context_type * run_context, init_mode_type init_mode) {
  enkf_main_init_internalization(enkf_main , ert_run_context_get_mode( run_context ));
  {
    stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
    enkf_main_initialize_from_scratch(enkf_main ,
				      ert_run_context_get_init_fs( run_context ),
				      param_list ,
				      ert_run_context_get_iactive( run_context ),
				      init_mode);
    stringlist_free( param_list );
  }
}




void enkf_main_run_tui_exp(enkf_main_type * enkf_main ,
                           bool_vector_type * iactive) {

  int active_before = bool_vector_count_equal(iactive, true);
  hook_manager_type * hook_manager = enkf_main_get_hook_manager(enkf_main);
  ert_run_context_type * run_context;
  init_mode_type init_mode = INIT_CONDITIONAL;
  int iter = 0;

  run_context = enkf_main_alloc_ert_run_context_ENSEMBLE_EXPERIMENT(enkf_main ,
                                                                    enkf_main_get_fs( enkf_main ) ,
                                                                    iactive ,
                                                                    iter );
  enkf_main_init_run( enkf_main , run_context , init_mode);
  enkf_main_create_run_path( enkf_main , iactive , iter );
  hook_manager_run_workflows(hook_manager, PRE_SIMULATION, enkf_main);
  enkf_main_run_step(enkf_main , run_context);
  
  int active_after = bool_vector_count_equal(iactive, true);
  if (active_after == active_before)
    hook_manager_run_workflows(hook_manager, POST_SIMULATION, enkf_main);

  ert_run_context_free( run_context );
}




int enkf_main_run_simple_step(enkf_main_type * enkf_main , bool_vector_type * iactive , init_mode_type init_mode, int iter) {
  ert_run_context_type * run_context = enkf_main_alloc_ert_run_context_ENSEMBLE_EXPERIMENT( enkf_main ,
                                                                                            enkf_main_get_fs( enkf_main ) ,
                                                                                            iactive ,
                                                                                            iter );
  enkf_main_init_run( enkf_main , run_context , init_mode);
  int successful_realizations = enkf_main_run_step( enkf_main , run_context );
  ert_run_context_free( run_context );

  return successful_realizations;
}



void enkf_main_run_smoother(enkf_main_type * enkf_main , enkf_fs_type * source_fs, const char * target_fs_name , bool_vector_type * iactive , int iter , bool rerun) {
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  if (!analysis_config_get_module_option( analysis_config , ANALYSIS_ITERABLE)) {
    if (enkf_main_run_simple_step( enkf_main , iactive , INIT_CONDITIONAL, iter)) {
      hook_manager_type * hook_manager = enkf_main_get_hook_manager(enkf_main);
      hook_manager_run_workflows(hook_manager, POST_SIMULATION, enkf_main);
    }

    {
      enkf_fs_type * target_fs = enkf_main_mount_alt_fs( enkf_main , target_fs_name , true );
      bool update_done = enkf_main_smoother_update( enkf_main , source_fs , target_fs );

      if (rerun) {
        if (update_done) {
          enkf_main_set_fs( enkf_main , target_fs , target_fs_name);
          if (enkf_main_run_simple_step(enkf_main , iactive , INIT_NONE, iter + 1)) {
            hook_manager_type * hook_manager = enkf_main_get_hook_manager(enkf_main);
            hook_manager_run_workflows(hook_manager, POST_SIMULATION, enkf_main);
          }
        } else
          fprintf(stderr,"** Warning: the analysis update failed - no rerun started.\n");
      }
      enkf_fs_decref( target_fs );

    }
  } else
    fprintf(stderr,"** ERROR: The normal smoother should not be combined with an iterable analysis module\n");
}


static bool enkf_main_run_simulation_and_postworkflow(enkf_main_type * enkf_main, ert_run_context_type * run_context) {
  bool ret = true;
  analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);

  int active_after_step = enkf_main_run_step(enkf_main , run_context);
  if (analysis_config_have_enough_realisations(analysis_config, active_after_step, enkf_main_get_ensemble_size(enkf_main))) {
    hook_manager_type * hook_manager = enkf_main_get_hook_manager(enkf_main);
    hook_manager_run_workflows(hook_manager, POST_SIMULATION, enkf_main);
  }  else {
    fprintf(stderr,"Simulation in iteration %d failed, stopping Iterated Ensemble Smoother\n", ert_run_context_get_iter( run_context ));
    ret = false;
  }

  return ret;
}


static bool enkf_main_run_analysis(enkf_main_type * enkf_main, enkf_fs_type * source_fs ,const char * target_fs_name, int iteration_number) {
  bool updateOK                          = false;
  analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);
  analysis_module_type * analysis_module = analysis_config_get_active_module(analysis_config);
  int pre_iteration_number               = analysis_module_get_int(analysis_module, "ITER");

  if (target_fs_name == NULL){
    fprintf(stderr,"Sorry: the updated ensemble will overwrite the current case in the iterated ensemble smoother.");
    printf("Running analysis on case %s, target case is %s\n", enkf_main_get_current_fs(enkf_main), enkf_main_get_current_fs(enkf_main));
    updateOK = enkf_main_smoother_update(enkf_main, source_fs, enkf_main_get_fs(enkf_main));
  } else {
    enkf_fs_type * target_fs = enkf_main_mount_alt_fs(enkf_main , target_fs_name , true );
    updateOK = enkf_main_smoother_update(enkf_main, source_fs , target_fs);
    enkf_fs_decref( target_fs );
  }

  int post_iteration_number = analysis_module_get_int(analysis_module, "ITER");

  if (post_iteration_number <= pre_iteration_number)
    updateOK = false;

  if (updateOK) {
    enkf_fs_type * target_fs = enkf_main_mount_alt_fs(enkf_main , target_fs_name , true );
    cases_config_set_int(enkf_fs_get_cases_config(target_fs), "iteration_number", iteration_number+1);
    enkf_fs_decref( target_fs );
  }

  return updateOK;
}


void enkf_main_run_iterated_ES(enkf_main_type * enkf_main, int num_iterations_to_run) {
  const analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);

  if (analysis_config_get_module_option( analysis_config , ANALYSIS_ITERABLE)) {
    const int ens_size                      = enkf_main_get_ensemble_size(enkf_main);
    bool_vector_type * iactive              = bool_vector_alloc(ens_size , true);
    enkf_fs_type * current_case             = enkf_main_get_fs( enkf_main );
    analysis_iter_config_type * iter_config = analysis_config_get_iter_config(analysis_config);
    int current_iteration                   = 0;
    const char * initial_case_name          = analysis_iter_config_iget_case( iter_config , current_iteration );

    if (!util_string_equal( initial_case_name , enkf_fs_get_case_name( current_case ))) {
      enkf_fs_type * initial_case = enkf_main_mount_alt_fs( enkf_main , initial_case_name , true);
      enkf_main_init_case_from_existing(enkf_main, current_case, 0, initial_case); // ANALYZED argument removed.
      enkf_main_set_fs( enkf_main , initial_case , NULL );
      enkf_fs_decref( initial_case );
    }

    { //Iteration 0
      ert_run_context_type * run_context = NULL;
      enkf_main_init_run(enkf_main , run_context , INIT_CONDITIONAL );
      enkf_main_run_simulation_and_postworkflow(enkf_main, run_context);
      ert_run_context_free( run_context );
    }

    { // Iteration 1 - num_iterations [iteration 1, num iterations]
      int num_retries_per_iteration = analysis_iter_config_get_num_retries_per_iteration(iter_config);
      int num_tries     = 0;
      enkf_fs_type * source_fs = enkf_main_get_fs( enkf_main );
      current_iteration = 1;

      while ((current_iteration <= num_iterations_to_run) && (num_tries < num_retries_per_iteration)) {
        ert_run_context_type * run_context = NULL;

        const char * target_fs_name = analysis_iter_config_iget_case( iter_config , current_iteration );

        if (enkf_main_run_analysis(enkf_main, source_fs, target_fs_name, current_iteration)) {
          enkf_main_select_fs(enkf_main, target_fs_name);
          if (!enkf_main_run_simulation_and_postworkflow(enkf_main, run_context ))
            break;
          num_tries = 0;
          ++current_iteration;
        } else {
          fprintf(stderr, "\nAnalysis failed, rerunning simulation on changed initial parameters\n");
          enkf_fs_type * target_fs = enkf_main_mount_alt_fs( enkf_main , target_fs_name , false );
          enkf_main_init_current_case_from_existing(enkf_main, target_fs, 0); // ANALYZED argument removed
          enkf_fs_decref(target_fs);
          ++num_tries;

          if (!enkf_main_run_simulation_and_postworkflow(enkf_main, run_context ))
          //if (!enkf_main_run_simulation_and_postworkflow(enkf_main, current_iteration-1, iactive))
            break;
        }

        ert_run_context_free( run_context );
      }
    }

    bool_vector_free(iactive);
  } else
    fprintf(stderr,"** ERROR: The current analysis module:%s can not be used for iterations \n", analysis_config_get_active_module_name( analysis_config ));
}


ert_run_context_type * enkf_main_alloc_ert_run_context_ENSEMBLE_EXPERIMENT(const enkf_main_type * enkf_main , enkf_fs_type * fs , bool_vector_type * iactive , int iter) {
  return ert_run_context_alloc_ENSEMBLE_EXPERIMENT( fs , iactive , model_config_get_runpath_fmt( enkf_main->model_config ) , enkf_main->subst_list , iter );
}

ert_init_context_type * enkf_main_alloc_ert_init_context(const enkf_main_type * enkf_main , enkf_fs_type * fs, const bool_vector_type * iactive , init_mode_type init_mode , int iter) {
  return ert_init_context_alloc( fs, iactive , model_config_get_runpath_fmt( enkf_main->model_config ) , enkf_main->subst_list , init_mode , iter );
}




/**
   This function creates a local_config file corresponding to the
   default 'ALL_ACTIVE' configuration. We eat our own dogshit around
   here...
*/

void enkf_main_create_all_active_config( const enkf_main_type * enkf_main) {


  bool single_node_update = analysis_config_get_single_node_update( enkf_main->analysis_config );
  local_config_type * local_config = enkf_main->local_config;
  local_config_clear( local_config );
  {
    local_updatestep_type * default_step = local_config_get_updatestep(local_config);
    local_ministep_type * ministep = local_config_alloc_ministep( local_config , "ALL_ACTIVE", NULL);
    local_obsdata_type * obsdata = local_config_alloc_obsdata(local_config, "ALL_OBS");
    local_dataset_type * all_active_dataset = local_config_alloc_dataset(local_config, "ALL_DATA");

    local_updatestep_add_ministep( default_step , ministep );

    /* Adding all observation keys */
    {
      hash_iter_type * obs_iter = enkf_obs_alloc_iter( enkf_main->obs );
      while ( !hash_iter_is_complete(obs_iter) ) {
        const char * obs_key = hash_iter_get_next_key( obs_iter );
        local_obsdata_node_type * obsdata_node = local_obsdata_node_alloc( obs_key , true );
        local_obsdata_add_node(obsdata, obsdata_node );
      }
      local_ministep_add_obsdata(ministep, obsdata);
      hash_iter_free( obs_iter );
    }

    /* Adding all node which can be updated. */
    {
      stringlist_type * keylist = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER);
      int i;
      for (i = 0; i < stringlist_get_size( keylist ); i++) {
        const char * key = stringlist_iget( keylist , i);
        bool add_node = true;

        /*
          Make sure the funny GEN_KW instance masquerading as
          SCHEDULE_PREDICTION_FILE is not added to the soup.
        */
        if (util_string_equal(key , "PRED"))
          add_node = false;


        if (add_node) {
          if (single_node_update) {
            local_dataset_type * this_dataset = local_config_alloc_dataset(local_config, key);
            local_dataset_add_node(this_dataset, key);
            local_ministep_add_dataset(ministep, this_dataset);
          }
          local_dataset_add_node(all_active_dataset, key);
        }
      }
      stringlist_free( keylist);
    }
    if (!single_node_update)
      local_ministep_add_dataset(ministep, all_active_dataset);

  }
}




static void enkf_main_init_user_config( const enkf_main_type * enkf_main , config_parser_type * config ) {
  config_schema_item_type * item;

  /*****************************************************************/
  /* config_add_schema_item():                                     */
  /*                                                               */
  /*  1. boolean - required?                                       */
  /*****************************************************************/

  ert_workflow_list_add_config_items( config );
  plot_config_add_config_items( config );
  analysis_config_add_config_items( config );
  ensemble_config_add_config_items( config );
  ecl_config_add_config_items( config );
  rng_config_add_config_items( config );

  /*****************************************************************/
  /* Required keywords from the ordinary model_config file */

  item = config_add_schema_item(config , CASE_TABLE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1);
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  config_add_key_value( config , LOG_LEVEL_KEY , false , CONFIG_INT);
  config_add_key_value( config , LOG_FILE_KEY  , false , CONFIG_STRING);

  config_add_key_value(config , MAX_RESAMPLE_KEY , false , CONFIG_INT);


  item = config_add_schema_item(config , NUM_REALIZATIONS_KEY , true  );
  config_schema_item_set_argc_minmax(item , 1 , 1);
  config_schema_item_iset_type( item , 0 , CONFIG_INT );
  config_add_alias(config , NUM_REALIZATIONS_KEY , "SIZE");
  config_add_alias(config , NUM_REALIZATIONS_KEY , "NUM_REALISATIONS");
  config_install_message(config , "SIZE" , "** Warning: \'SIZE\' is depreceated - use \'NUM_REALIZATIONS\' instead.");


  /*****************************************************************/
  /* Optional keywords from the model config file */

  item = config_add_schema_item( config , RUN_TEMPLATE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 2 , CONFIG_DEFAULT_ARG_MAX );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  config_add_key_value(config , RUNPATH_KEY , false , CONFIG_STRING);

  item = config_add_schema_item(config , ENSPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item( config , JOBNAME_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item(config , DBASE_TYPE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1, 1 );
  config_schema_item_set_common_selection_set(item , 2 , (const char *[2]) {"PLAIN" , "BLOCK_FS"});

  item = config_add_schema_item(config , FORWARD_MODEL_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config , DATA_KW_KEY , false  );
  config_schema_item_set_argc_minmax(item , 2 , 2);

  config_add_key_value(config , PRE_CLEAR_RUNPATH_KEY , false , CONFIG_BOOL);

  item = config_add_schema_item(config , DELETE_RUNPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config , OBS_CONFIG_KEY  , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  config_add_key_value(config , TIME_MAP_KEY , false , CONFIG_EXISTING_PATH);

  item = config_add_schema_item(config , RFT_CONFIG_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  item = config_add_schema_item(config , RFTPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item(config, GEN_KW_EXPORT_FILE_KEY, false );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item(config , LOCAL_CONFIG_KEY  , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  {
    stringlist_type * refcase_dep = stringlist_alloc_argv_ref( (const char *[1]) { REFCASE_KEY } , 1);

    item = config_add_schema_item(config , HISTORY_SOURCE_KEY , false  );
    config_schema_item_set_argc_minmax(item , 1 , 1);
    config_schema_item_set_common_selection_set(item , 3 , (const char *[3]) {"SCHEDULE" , "REFCASE_SIMULATED" , "REFCASE_HISTORY"});
    config_schema_item_set_required_children_on_value(item , "REFCASE_SIMULATED" , refcase_dep);
    config_schema_item_set_required_children_on_value(item , "REFCASE_HISTORY"  , refcase_dep);

    stringlist_free(refcase_dep);
  }

  hook_manager_add_config_items( config );
}


keep_runpath_type  enkf_main_iget_keep_runpath( const enkf_main_type * enkf_main , int iens ) {
  return enkf_state_get_keep_runpath( enkf_main->ensemble[iens] );
}

void enkf_main_iset_keep_runpath( enkf_main_type * enkf_main , int iens , keep_runpath_type keep_runpath) {
  enkf_state_set_keep_runpath( enkf_main->ensemble[iens] , keep_runpath);
}

void enkf_main_set_verbose( enkf_main_type * enkf_main , bool verbose) {
  enkf_main->verbose = verbose;
}


bool enkf_main_get_verbose( const enkf_main_type * enkf_main ) {
  return enkf_main->verbose;
}

/**
   Observe that this function parses and TEMPORARILY stores the keep_runpath
   information ion the enkf_main object. This is subsequently passed on the
   enkf_state members, and the functions enkf_main_iget_keep_runpath() and
   enkf_main_iset_keep_runpath() act on the enkf_state objects, and not on the
   internal keep_runpath field of the enkf_main object (what a fxxxing mess).
*/


void enkf_main_parse_keep_runpath(enkf_main_type * enkf_main , const char * delete_runpath_string , int ens_size ) {

  int i;
  for (i = 0; i < ens_size; i++)
    int_vector_iset( enkf_main->keep_runpath , i , DEFAULT_KEEP);

  {
    int_vector_type * active_list = string_util_alloc_active_list(delete_runpath_string);

    for (i = 0; i < int_vector_size( active_list ); i++)
      int_vector_iset( enkf_main->keep_runpath , int_vector_iget( active_list , i ) , EXPLICIT_DELETE);

    int_vector_free( active_list );
  }
}



/**
   There is NO tagging anymore - if the user wants tags - the user
   supplies the key __WITH__ tags.
*/
void enkf_main_add_data_kw(enkf_main_type * enkf_main , const char * key , const char * value) {
  subst_list_append_copy( enkf_main->subst_list   , key , value , "Supplied by the user in the configuration file.");
}


void enkf_main_data_kw_fprintf_config( const enkf_main_type * enkf_main , FILE * stream ) {
  for (int i = 0; i < subst_list_get_size( enkf_main->subst_list ); i++) {
    fprintf(stream , CONFIG_KEY_FORMAT , DATA_KW_KEY );
    fprintf(stream , CONFIG_VALUE_FORMAT    , subst_list_iget_key( enkf_main->subst_list , i ));
    fprintf(stream , CONFIG_ENDVALUE_FORMAT , subst_list_iget_value( enkf_main->subst_list , i ));
  }
}


void enkf_main_clear_data_kw( enkf_main_type * enkf_main ) {
  subst_list_clear( enkf_main->subst_list );
}

static void enkf_main_add_subst_kw( enkf_main_type * enkf_main , const char * key , const char * value, const char * help_text , bool insert_copy) {
  char * tagged_key = util_alloc_sprintf( INTERNAL_DATA_KW_TAG_FORMAT , key );

  if (insert_copy)
    subst_list_append_owned_ref( enkf_main->subst_list , tagged_key , util_alloc_string_copy( value ), help_text);
  else
    subst_list_append_ref( enkf_main->subst_list , tagged_key , value , help_text);

  free(tagged_key);
}


static void enkf_main_init_hook_manager( enkf_main_type * enkf_main , config_content_type * config ) {
  hook_manager_init( enkf_main->hook_manager , config );
}

static void enkf_main_init_subst_list( enkf_main_type * enkf_main ) {
  /* Here we add the functions which should be available for string substitution operations. */
  subst_func_pool_add_func( enkf_main->subst_func_pool , "EXP"       , "exp"                               , subst_func_exp         , false , 1 , 1 , NULL);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "LOG"       , "log"                               , subst_func_log         , false , 1 , 1 , NULL);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "POW10"     , "Calculates 10^x"                   , subst_func_pow10       , false , 1 , 1 , NULL);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "ADD"       , "Adds arguments"                    , subst_func_add         , true  , 1 , 0 , NULL);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "MUL"       , "Multiplies arguments"              , subst_func_mul         , true  , 1 , 0 , NULL);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "RANDINT"   , "Returns a random integer - 32 bit" , subst_func_randint     , false , 0 , 0 , enkf_main->rng);
  subst_func_pool_add_func( enkf_main->subst_func_pool , "RANDFLOAT" , "Returns a random float 0-1."       , subst_func_randfloat   , false , 0 , 0 , enkf_main->rng);

  /**
     Allocating the parent subst_list instance. This will (should ...)
     be the top level subst instance for all substitions in the ert
     program.

     All the functions available or only installed in this
     subst_list.

     The key->value replacements installed in this instance are
     key,value pairs which are:

      o Common to all ensemble members.

      o Constant in time.
  */


  /* Installing the functions. */
  subst_list_insert_func( enkf_main->subst_list , "EXP"         , "__EXP__");
  subst_list_insert_func( enkf_main->subst_list , "LOG"         , "__LOG__");
  subst_list_insert_func( enkf_main->subst_list , "POW10"       , "__POW10__");
  subst_list_insert_func( enkf_main->subst_list , "ADD"         , "__ADD__");
  subst_list_insert_func( enkf_main->subst_list , "MUL"         , "__MUL__");
  subst_list_insert_func( enkf_main->subst_list , "RANDINT"     , "__RANDINT__");
  subst_list_insert_func( enkf_main->subst_list , "RANDFLOAT"   , "__RANDFLOAT__");
}



enkf_main_type * enkf_main_alloc_empty( ) {
  enkf_main_type * enkf_main = util_malloc(sizeof * enkf_main);
  UTIL_TYPE_ID_INIT(enkf_main , ENKF_MAIN_ID);
  ert_log_open_empty();
  enkf_main->ensemble           = NULL;
  enkf_main->user_config_file   = NULL;
  enkf_main->site_config_file   = NULL;
  enkf_main->rft_config_file    = NULL;
  enkf_main->local_config       = NULL;
  enkf_main->rng                = NULL;
  enkf_main->ens_size           = 0;
  enkf_main->keep_runpath       = int_vector_alloc( 0 , DEFAULT_KEEP );
  enkf_main->rng_config         = rng_config_alloc( );
  enkf_main->site_config        = site_config_alloc_empty();
  enkf_main->ensemble_config    = ensemble_config_alloc();
  enkf_main->ecl_config         = ecl_config_alloc();
  enkf_main->plot_config        = plot_config_alloc_default();
  enkf_main->ranking_table      = ranking_table_alloc( 0 );
  enkf_main->obs                = NULL;
  enkf_main->model_config       = model_config_alloc( );
  enkf_main->local_config       = local_config_alloc( );

  enkf_main_rng_init( enkf_main );
  enkf_main->subst_func_pool    = subst_func_pool_alloc(  );
  enkf_main->subst_list         = subst_list_alloc( enkf_main->subst_func_pool );
  enkf_main->templates          = ert_templates_alloc( enkf_main->subst_list );
  enkf_main->workflow_list      = ert_workflow_list_alloc( enkf_main->subst_list );
  enkf_main->hook_manager       = hook_manager_alloc( enkf_main->workflow_list );
  enkf_main->analysis_config    = analysis_config_alloc( enkf_main->rng );

  enkf_main_init_subst_list( enkf_main );
  enkf_main_set_verbose( enkf_main , true );
  enkf_main_init_fs( enkf_main );
  return enkf_main;
}





static void enkf_main_install_data_kw( enkf_main_type * enkf_main , hash_type * config_data_kw) {
  /*
    Installing the DATA_KW keywords supplied by the user - these are
    at the very top level, so they can reuse everything defined later.
  */
  if (config_data_kw) {
    hash_iter_type * iter = hash_iter_alloc(config_data_kw);
    const char * key = hash_iter_get_next_key(iter);
    while (key != NULL) {
      enkf_main_add_data_kw( enkf_main , key , hash_get( config_data_kw , key ));
      key = hash_iter_get_next_key(iter);
    }
    hash_iter_free(iter);
  }
}



static void enkf_main_install_common_data_kw( enkf_main_type * enkf_main ) {
  /*
     Installing the based (key,value) pairs which are common to all
     ensemble members, and independent of time.
  */
  char * cwd                    = util_alloc_cwd();
  char * date_string            = util_alloc_date_stamp_utc();
  const char * num_cpu_string   = "1";

  enkf_main_add_subst_kw( enkf_main , "CWD"          , cwd , "The current working directory we are running from - the location of the config file." , true);
  enkf_main_add_subst_kw( enkf_main , "CONFIG_PATH"  , cwd , "The current working directory we are running from - the location of the config file." , true);
  enkf_main_add_subst_kw( enkf_main , "DATE"         , date_string , "The current date." , true);
  enkf_main_add_subst_kw( enkf_main , "NUM_CPU"      , num_cpu_string , "The number of CPU used for one forward model." , true );
  enkf_main_add_subst_kw( enkf_main , "RUNPATH_FILE" , hook_manager_get_runpath_list_file( enkf_main->hook_manager ) , "The name of a file with a list of run directories." , true);

  free( cwd );
  free( date_string );
}



runpath_list_type * enkf_main_get_runpath_list( const enkf_main_type * enkf_main ) {
  return hook_manager_get_runpath_list( enkf_main->hook_manager );
}


/**
   This function will resize the enkf_main->ensemble vector,
   allocating or freeing enkf_state instances as needed.
*/


void enkf_main_resize_ensemble( enkf_main_type * enkf_main , int new_ens_size ) {
  int iens;

  /* No change */
  if (new_ens_size == enkf_main->ens_size)
    return ;

  ranking_table_set_ens_size( enkf_main->ranking_table , new_ens_size );
  /* Tell the site_config object (i.e. the queue drivers) about the new ensemble size: */
  site_config_set_ens_size( enkf_main->site_config , new_ens_size );


  /* The ensemble is shrinking. */
  if (new_ens_size < enkf_main->ens_size) {
    /*1: Free all ensemble members which go out of scope. */
    for (iens = new_ens_size; iens < enkf_main->ens_size; iens++)
      enkf_state_free( enkf_main->ensemble[iens] );

    /*2: Shrink the ensemble pointer. */
    enkf_main->ensemble = util_realloc(enkf_main->ensemble , new_ens_size * sizeof * enkf_main->ensemble );
    enkf_main->ens_size = new_ens_size;
    return;
  }


  /* The ensemble is expanding */
  if (new_ens_size > enkf_main->ens_size) {
    /*1: Grow the ensemble pointer. */
    enkf_main->ensemble = util_realloc(enkf_main->ensemble , new_ens_size * sizeof * enkf_main->ensemble );

    /*2: Allocate the new ensemble members. */
    for (iens = enkf_main->ens_size; iens < new_ens_size; iens++)

      /* Observe that due to the initialization of the rng - this function is currently NOT thread safe. */
      enkf_main->ensemble[iens] = enkf_state_alloc(iens,
                                                   enkf_main->rng ,
                                                   model_config_iget_casename( enkf_main->model_config , iens ) ,
                                                   enkf_main->pre_clear_runpath                                 ,
                                                   int_vector_safe_iget( enkf_main->keep_runpath , iens)        ,
                                                   enkf_main->model_config                                      ,
                                                   enkf_main->ensemble_config                                   ,
                                                   enkf_main->site_config                                       ,
                                                   enkf_main->ecl_config                                        ,
                                                   enkf_main->templates                                         ,
                                                   enkf_main->subst_list);
    enkf_main->ens_size = new_ens_size;
    return;
  }

  util_abort("%s: something is seriously broken - should NOT be here .. \n",__func__);
}


void enkf_main_add_node(enkf_main_type * enkf_main, enkf_config_node_type * enkf_config_node) {
    for (int iens = 0; iens < enkf_main_get_ensemble_size(enkf_main); iens++) {

      enkf_state_add_node(enkf_main->ensemble[iens], enkf_config_node_get_key(enkf_config_node), enkf_config_node);
    }
}



void enkf_main_update_node( enkf_main_type * enkf_main , const char * key ) {
  int iens;
  for (iens = 0; iens < enkf_main->ens_size; iens++)
    enkf_state_update_node( enkf_main->ensemble[iens] , key );
}














/******************************************************************/

/**
   SCHEDULE_PREDICTION_FILE.

   The SCHEDULE_PREDICTION_FILE is implemented as a GEN_KW instance,
   with some twists. Observe the following:

   1. The SCHEDULE_PREDICTION_FILE is added to the ensemble_config
      as a GEN_KW node with key 'PRED'.

   2. The target file is set equal to the initial prediction file
      (i.e. the template in this case), NOT including any path
      components.

*/


void enkf_main_set_schedule_prediction_file__( enkf_main_type * enkf_main , const char * template_file , const char * parameters , const char * min_std , const char * init_file_fmt) {
  const char * key = "PRED";
  /*
    First remove/delete existing PRED node if it is already installed.
  */
  if (ensemble_config_has_key( enkf_main->ensemble_config , key))
    enkf_main_del_node( enkf_main , key );

  if (template_file != NULL) {
    char * target_file;
    bool forward_init = false;
    enkf_config_node_type * config_node = ensemble_config_add_gen_kw( enkf_main->ensemble_config , key , forward_init);
    {
      char * base;
      char * ext;
      util_alloc_file_components( template_file , NULL , &base , &ext);
      target_file = util_alloc_filename(NULL , base , ext );
      util_safe_free( base );
      util_safe_free( ext );
    }
    enkf_config_node_update_gen_kw( config_node , target_file , template_file , parameters , min_std , init_file_fmt );
    free( target_file );
    ecl_config_set_schedule_prediction_file( enkf_main->ecl_config , template_file );
  }
}


void enkf_main_set_schedule_prediction_file( enkf_main_type * enkf_main , const char * schedule_prediction_file) {
  enkf_main_set_schedule_prediction_file__(enkf_main , schedule_prediction_file , NULL , NULL , NULL );
}


const char * enkf_main_get_schedule_prediction_file( const enkf_main_type * enkf_main ) {
  return ecl_config_get_schedule_prediction_file( enkf_main->ecl_config );
}



/*****************************************************************/

static void enkf_main_init_data_kw( enkf_main_type * enkf_main , config_content_type * config ) {
  {
    const subst_list_type * define_list = config_content_get_define_list( config );
    for (int i=0; i < subst_list_get_size( define_list ); i++) {
      const char * key = subst_list_iget_key( define_list , i );
      const char * value = subst_list_iget_value( define_list , i );
      enkf_main_add_data_kw( enkf_main , key , value );
    }
  }

  if (config_content_has_item( config , DATA_KW_KEY)) {
    config_content_item_type * data_item = config_content_get_item( config , DATA_KW_KEY );
    hash_type      * data_kw = config_content_item_alloc_hash(data_item , true);
    enkf_main_install_data_kw( enkf_main , data_kw );
    hash_free( data_kw );
  }

  enkf_main_install_common_data_kw( enkf_main );
}





/*****************************************************************/


rng_config_type * enkf_main_get_rng_config( const enkf_main_type * enkf_main ) {
  return enkf_main->rng_config;
}


void enkf_main_rng_init( enkf_main_type * enkf_main) {
  if (enkf_main->rng != NULL)
    rng_config_init_rng(enkf_main->rng_config, enkf_main->rng);
  else
    enkf_main->rng = rng_config_alloc_init_rng( enkf_main->rng_config );
}


void enkf_main_update_local_updates( enkf_main_type * enkf_main) {
  const enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );
  if (enkf_obs_have_obs( enkf_obs )) {
    /* First create the default ALL_ACTIVE configuration. */
    enkf_main_create_all_active_config( enkf_main );
  }
}

static char * __enkf_main_alloc_user_config_file(const enkf_main_type * enkf_main, bool base_only) {
    char * base_name;
    char * extension;
    util_alloc_file_components(enkf_main_get_user_config_file(enkf_main), NULL, &base_name, &extension);

    char * config_file;
    if (base_only) {
        config_file = util_alloc_filename(NULL, base_name, NULL);;
    } else {
        config_file = util_alloc_filename(NULL, base_name, extension);
    }

    free(base_name);
    free(extension);
    return config_file;
}

static hash_type *__enkf_main_alloc_predefined_kw_map(const enkf_main_type *enkf_main) {
    char * config_file_base       = __enkf_main_alloc_user_config_file(enkf_main, true);
    char * config_file            = __enkf_main_alloc_user_config_file(enkf_main, false);
    hash_type * pre_defined_kw_map = hash_alloc();

    hash_insert_string(pre_defined_kw_map, "<CONFIG_FILE>", config_file);
    hash_insert_string(pre_defined_kw_map, "<CONFIG_FILE_BASE>", config_file_base);

    free( config_file ) ;
    free( config_file_base );
    return pre_defined_kw_map;
}


/**
   Observe that the site-config initializations starts with chdir() to
   the location of the site_config_file; this ensures that the
   site_config can contain relative paths to job description files and
   scripts.
*/


static void enkf_main_bootstrap_site(enkf_main_type * enkf_main , const char * site_config_file) {
  if (site_config_file != NULL) {
    if (!util_file_exists(site_config_file))  util_exit("%s: can not locate site configuration file:%s \n",__func__ , site_config_file);
    config_parser_type * config = config_alloc();
    site_config_add_config_items( config , true );
    {
      config_content_type * content = config_parse(config , site_config_file  , "--" , INCLUDE_KEY , DEFINE_KEY , NULL, CONFIG_UNRECOGNIZED_WARN , false);
      if (config_content_is_valid( content )) {
        site_config_init( enkf_main->site_config , content );
        analysis_config_load_all_external_modules_from_config(enkf_main->analysis_config, content);
        ert_workflow_list_init( enkf_main->workflow_list , content );
      } else {
        config_error_type * errors = config_content_get_errors( content );
        fprintf(stderr , "** ERROR: Parsing site configuration file:%s failed \n\n" , site_config_file);
        config_error_fprintf( errors , true , stderr );
        exit(1);
      }
      config_content_free( content );
    }
    config_free( config );
  }
}


/**
   This function boots everything needed for running a EnKF
   application. Very briefly it can be summarized as follows:

    1. A large config object is initalized with all the possible
       keywords we are looking for.

    2. All the config files are parsed in one go.

    3. The various objects are build up by reading from the config
       object.

    4. The resulting enkf_main object contains *EVERYTHING*
       (whoaha...)


  Observe that the function will start with chdir() to the directory
  containing the configuration file, so that all subsequent file
  references are relative to the location of the configuration
  file. This also applies if the command_line argument given is a
  symlink.


  If the parameter @strict is set to false a configuration with some
  missing parameters will validate; this is to support bootstrapping
  from a minimal configuration created by the GUI. The parameters
  which become optional in a non-strict mode are:

    FORWARD_MODEL
    DATA_FILE
    SCHEDULE_FILE
    ECLBASE

*/


/**
   It is possible to pass NULL as the model_config argument, in that
   case only the site config file will be parsed. The purpose of this
   is mainly to be able to test that the site config file is valid.
*/


enkf_main_type * enkf_main_bootstrap(const char * _model_config, bool strict , bool verbose) {
  const char     * site_config  = site_config_get_location();
  char           * model_config = NULL;
  enkf_main_type * enkf_main;    /* The enkf_main object is allocated when the config parsing is completed. */

  if (_model_config) {
    {
      char * path;
      char * base;
      char * ext;
      if (util_is_link( _model_config )) {   /* The command line argument given is a symlink - we start by changing to */
	                                     /* the real location of the configuration file. */
	char  * realpath = util_alloc_link_target( _model_config );
	util_alloc_file_components(realpath , &path , &base , &ext);
	free( realpath );
      } else
	util_alloc_file_components(_model_config , &path , &base , &ext);

      if (path != NULL) {
	if (util_chdir(path) != 0)
	  util_abort("%s: failed to change directory to: %s : %s \n",__func__ , path , strerror(errno));

	if (verbose)
	  printf("Changing to directory ...................: %s \n",path);

	if (ext != NULL)
	  model_config = util_alloc_filename( NULL , base , ext );
	else
	  model_config = util_alloc_string_copy( base );
      } else
	model_config = util_alloc_string_copy(_model_config);

      util_safe_free( path );
      util_safe_free( base );
      util_safe_free( ext );
    }

    if (!util_file_exists(model_config))
      util_exit("%s: can not locate user configuration file:%s \n",__func__ , model_config);
  }


  {
    config_parser_type * config;
    config_content_type * content;
    enkf_main            = enkf_main_alloc_empty( );
    enkf_main_set_verbose( enkf_main , verbose );
    enkf_main_bootstrap_site( enkf_main , site_config);

    if (model_config) {
      enkf_main_set_site_config_file( enkf_main , site_config );
      enkf_main_set_user_config_file( enkf_main , model_config );

      config = config_alloc();
      enkf_main_init_user_config( enkf_main , config );
      site_config_add_config_items( config , false );
      site_config_init_user_mode( enkf_main->site_config );

      {
        hash_type *pre_defined_kw_map = __enkf_main_alloc_predefined_kw_map(enkf_main);
        content = config_parse(config , model_config , "--" , INCLUDE_KEY , DEFINE_KEY , pre_defined_kw_map, CONFIG_UNRECOGNIZED_WARN , true);
        hash_free(pre_defined_kw_map);
      }

      if (!config_content_is_valid( content )) {
	config_error_type * errors = config_content_get_errors( content );
	config_error_fprintf( errors , true , stderr );
	exit(1);
      }

      site_config_init( enkf_main->site_config , content );                                   /*  <---- model_config : second pass. */

      /*****************************************************************/
      /*
	OK - now we have parsed everything - and we are ready to start
	populating the enkf_main object.
      */




      {
	char * log_file;
	int log_level = DEFAULT_LOG_LEVEL;
	if(config_content_has_item( content , LOG_LEVEL_KEY))
	  log_level = config_content_get_value_as_int(content , LOG_LEVEL_KEY);

	if (config_content_has_item( content , LOG_FILE_KEY))
	  log_file = util_alloc_string_copy( config_content_get_value(content , LOG_FILE_KEY));
	else
	  log_file = util_alloc_filename( NULL , enkf_main->user_config_file , "log");

	ert_log_init_log(log_level, log_file ,  enkf_main->verbose);

	free( log_file );
      }

      /*
	Initializing the various 'large' sub config objects.
      */
      rng_config_init( enkf_main->rng_config , content );
      enkf_main_rng_init( enkf_main );  /* Must be called before the ensmeble is created. */

      enkf_main_init_subst_list( enkf_main );
      ert_workflow_list_init( enkf_main->workflow_list , content );

      analysis_config_load_internal_modules( enkf_main->analysis_config );
      analysis_config_init( enkf_main->analysis_config , content );
      ecl_config_init( enkf_main->ecl_config , content );
      plot_config_init( enkf_main->plot_config , content );

      ensemble_config_init( enkf_main->ensemble_config , content , ecl_config_get_grid( enkf_main->ecl_config ) , ecl_config_get_refcase( enkf_main->ecl_config) );

      model_config_init( enkf_main->model_config ,
			 content ,
			 enkf_main_get_ensemble_size( enkf_main ),
			 site_config_get_installed_jobs(enkf_main->site_config) ,
			 ecl_config_get_last_history_restart( enkf_main->ecl_config ),
			 ecl_config_get_sched_file(enkf_main->ecl_config) ,
			 ecl_config_get_refcase( enkf_main->ecl_config ));

      enkf_main_init_hook_manager( enkf_main , content );
      enkf_main_init_data_kw( enkf_main , content );
      enkf_main_update_num_cpu( enkf_main );
      {
	if (config_content_has_item( content , SCHEDULE_PREDICTION_FILE_KEY )) {
	  const config_content_item_type * pred_item = config_content_get_item( content , SCHEDULE_PREDICTION_FILE_KEY );
	  config_content_node_type * pred_node = config_content_item_get_last_node( pred_item );
	  const char * template_file = config_content_node_iget_as_path( pred_node , 0 );
	  {
	    hash_type * opt_hash = hash_alloc();
	    config_content_node_init_opt_hash( pred_node , opt_hash , 1 );

	    const char * parameters = hash_safe_get( opt_hash , "PARAMETERS" );
	    const char * min_std    = hash_safe_get( opt_hash , "MIN_STD"    );
	    const char * init_files = hash_safe_get( opt_hash , "INIT_FILES" );

	    enkf_main_set_schedule_prediction_file__( enkf_main , template_file , parameters , min_std , init_files );
	    hash_free( opt_hash );
	  }
	}
      }


      /*****************************************************************/
      /**
         By default the simulation directories are left intact when
         the simulations re complete, but using the keyword
         DELETE_RUNPATH you can request (some of) the directories to
         be wiped after the simulations are complete.
      */
      {
	{
	  char * delete_runpath_string = NULL;
	  int    ens_size              = config_content_get_value_as_int(content , NUM_REALIZATIONS_KEY);

	  if (config_content_has_item(content , DELETE_RUNPATH_KEY))
	    delete_runpath_string = config_content_alloc_joined_string(content , DELETE_RUNPATH_KEY , "");

	  enkf_main_parse_keep_runpath( enkf_main , delete_runpath_string , ens_size );

	  util_safe_free( delete_runpath_string );
	}

	/* This is really in the wrong place ... */
	{
	  enkf_main->pre_clear_runpath = DEFAULT_PRE_CLEAR_RUNPATH;
	  if (config_content_has_item(content , PRE_CLEAR_RUNPATH_KEY))
	    enkf_main->pre_clear_runpath = config_content_get_value_as_bool( content , PRE_CLEAR_RUNPATH_KEY);
	}

	ecl_config_static_kw_init( enkf_main->ecl_config , content );

	/* Installing templates */
	ert_templates_init( enkf_main->templates , content );


	{
	  const char * rft_config_file = NULL;
	  if (config_content_has_item(content , RFT_CONFIG_KEY))
	    rft_config_file = config_content_iget(content , RFT_CONFIG_KEY , 0,0);

	  enkf_main_set_rft_config_file( enkf_main , rft_config_file );
	}


	/*****************************************************************/
        enkf_main_user_select_initial_fs( enkf_main );

	/* Adding ensemble members */
	enkf_main_resize_ensemble( enkf_main  , config_content_iget_as_int(content , NUM_REALIZATIONS_KEY , 0 , 0) );

	/*****************************************************************/

        /* Loading observations */
        enkf_main_alloc_obs(enkf_main);
        if (config_content_has_item(content , OBS_CONFIG_KEY)) {
          const char * obs_config_file = config_content_iget(content  , OBS_CONFIG_KEY , 0,0);
          enkf_main_load_obs( enkf_main , obs_config_file , true );
        }

      }
      config_content_free( content );
      config_free(config);
    }
    enkf_main_init_jobname( enkf_main );
    free( model_config );
  }
  return enkf_main;
}




/**
   This function creates a minimal configuration file, with a few
   parameters (a bit arbitrary) parameters read from (typically) a GUI
   configuration dialog.

   The set of parameters written by this function is _NOT_ a minimum
   set to generate a valid configuration.
*/

void enkf_main_create_new_config( const char * config_file , const char * storage_path , const char * dbase_type , int num_realizations) {

  FILE * stream = util_mkdir_fopen( config_file , "w" );

  fprintf(stream , CONFIG_KEY_FORMAT      , ENSPATH_KEY);
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , storage_path );

  fprintf(stream , CONFIG_KEY_FORMAT      , DBASE_TYPE_KEY);
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , dbase_type);

  fprintf(stream , CONFIG_KEY_FORMAT      , NUM_REALIZATIONS_KEY);
  fprintf(stream , CONFIG_INT_FORMAT , num_realizations);
  fprintf(stream , "\n");

  fclose( stream );

  printf("Have created configuration file: %s \n",config_file );
}








/**
   First deleting all the nodes - then the configuration.
*/

void enkf_main_del_node(enkf_main_type * enkf_main , const char * key) {
  const int ens_size = enkf_main_get_ensemble_size( enkf_main );
  int iens;
  for (iens = 0; iens < ens_size; iens++)
    enkf_state_del_node(enkf_main->ensemble[iens] , key);
  ensemble_config_del_node(enkf_main->ensemble_config , key);
}



int enkf_main_get_ensemble_size( const enkf_main_type * enkf_main ) {
  return enkf_main->ens_size;
}


enkf_state_type ** enkf_main_get_ensemble( enkf_main_type * enkf_main) {
  return enkf_main->ensemble;
}


const enkf_state_type ** enkf_main_get_ensemble_const( const enkf_main_type * enkf_main) {
  return (const enkf_state_type **) enkf_main->ensemble;
}



/**
   In this function we initialize the variables which control
   which nodes are internalized (i.e. loaded from the forward
   simulation and stored in the enkf_fs 'database'). The system is
   based on two-levels:

   * Should we store the state? This is goverened by the variable
     model_config->internalize_state. If this is true we will
     internalize all nodes which have enkf_var_type = {dynamic_state ,
     static_state}. In the same way the variable
     model_config->internalize_results governs whether the dynamic
     results (i.e. summary variables in ECLIPSE speak) should be
     internalized.

   * In addition we have fine-grained control in the enkf_config_node
     objects where we can explicitly say that, altough we do not want
     to internalize the full state, we want to internalize e.g. the
     pressure field.

   * All decisions on internalization are based on a per report step
     basis.

   The user-space API for manipulating this is (extremely)
   limited. What is implemented here is the following:

     1. We internalize the initial dynamic state.

     2. For all the end-points in the current enkf_sched instance we
        internalize the state.

     3. store_results is set to true for all report steps irrespective
        of run_mode.

     4. We iterate over all the observations, and ensure that the
        observed nodes (i.e. the pressure for an RFT) are internalized
        (irrespective of whether they are of type dynamic_state or
        dynamic_result).

   Observe that this cascade can result in some nodes, i.e. a rate we
   are observing, to be marked for internalization several times -
   that is no problem.

   -----

   For performance reason model_config contains the bool vector
   __load_eclipse_restart; if it is true the ECLIPSE restart state is
   loaded from disk, otherwise no loading is performed. This implies
   that if we do not want to internalize the full state but for
   instance the pressure (i.e. for an RFT) we must set the
   __load_state variable for the actual report step to true. For this
   reason calls enkf_config_node_internalize() must be accompanied by
   calls to model_config_set_load_state|results() - this is ensured
   when using this function to manipulate the configuration of
   internalization.

*/


void enkf_main_init_internalization( enkf_main_type * enkf_main , run_mode_type run_mode ) {
  /* Clearing old internalize flags. */
  model_config_init_internalization( enkf_main->model_config );

  /* Internalizing the initial state. */
  model_config_set_internalize_state( enkf_main->model_config , 0);


  /* Make sure we internalize at all observation times.*/
  {
    hash_type      * map  = enkf_obs_alloc_data_map(enkf_main->obs);
    hash_iter_type * iter = hash_iter_alloc(map);
    const char * obs_key  = hash_iter_get_next_key(iter);

    while (obs_key != NULL) {
      obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_main->obs , obs_key );
      enkf_config_node_type * data_node = obs_vector_get_config_node( obs_vector );
      int active_step = -1;
      do {
        active_step = obs_vector_get_next_active_step( obs_vector , active_step );
        if (active_step >= 0) {
          enkf_config_node_set_internalize( data_node , active_step );
          {
            enkf_var_type var_type = enkf_config_node_get_var_type( data_node );
            if (var_type == DYNAMIC_STATE)
              model_config_set_load_state( enkf_main->model_config , active_step);
          }
        }
      } while (active_step >= 0);
      obs_key = hash_iter_get_next_key(iter);
    }
    hash_iter_free(iter);
    hash_free(map);
  }
}




/*****************************************************************/






const ext_joblist_type * enkf_main_get_installed_jobs( const enkf_main_type * enkf_main ) {
  return site_config_get_installed_jobs( enkf_main->site_config );
}



/*****************************************************************/

void enkf_main_get_observations( const enkf_main_type * enkf_main, const char * user_key , int obs_count , time_t * obs_time , double * y , double * std) {
  ensemble_config_get_observations( enkf_main->ensemble_config , enkf_main->obs , user_key , obs_count , obs_time , y , std);
}


int enkf_main_get_observation_count( const enkf_main_type * enkf_main, const char * user_key ) {
  return ensemble_config_get_observations( enkf_main->ensemble_config , enkf_main->obs , user_key , 0 , NULL , NULL , NULL);
}



void enkf_main_log_fprintf_config( const enkf_main_type * enkf_main , FILE * stream ) {
  fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
  fprintf( stream , CONFIG_COMMENT_FORMAT  , "Here comes configuration information about the ERT logging.");
  fprintf( stream , CONFIG_KEY_FORMAT      , LOG_FILE_KEY );
  fprintf( stream , CONFIG_ENDVALUE_FORMAT , ert_log_get_filename());

  if (ert_log_get_log_level() != DEFAULT_LOG_LEVEL) {
    fprintf(stream , CONFIG_KEY_FORMAT      , LOG_LEVEL_KEY );
    fprintf(stream , CONFIG_INT_FORMAT , ert_log_get_log_level());
    fprintf(stream , "\n");
  }

  fprintf(stream , "\n");
  fprintf(stream , "\n");
}


void enkf_main_install_SIGNALS(void) {
  util_install_signals();
}




ert_templates_type * enkf_main_get_templates( enkf_main_type * enkf_main ) {
  return enkf_main->templates;
}



/*****************************************************************/


void enkf_main_fprintf_runpath_config( const enkf_main_type * enkf_main , FILE * stream ) {
  fprintf(stream , CONFIG_KEY_FORMAT      , PRE_CLEAR_RUNPATH_KEY );
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , CONFIG_BOOL_STRING( enkf_state_get_pre_clear_runpath( enkf_main->ensemble[0] )));

  {
    bool del_comma  = false;


    for (int iens = 0; iens < enkf_main->ens_size; iens++) {
      keep_runpath_type keep_runpath = enkf_main_iget_keep_runpath( enkf_main , iens );
      if (keep_runpath == EXPLICIT_DELETE) {
        if (!del_comma) {
          fprintf(stream , CONFIG_KEY_FORMAT , DELETE_RUNPATH_KEY );
          fprintf(stream , CONFIG_INT_FORMAT , iens);
          del_comma = true;
        } else {
          fprintf(stream , ",");
          fprintf(stream , CONFIG_INT_FORMAT , iens);
        }
      }
    }
    fprintf(stream , "\n");
  }
}



/*****************************************************************/

ert_workflow_list_type * enkf_main_get_workflow_list( enkf_main_type * enkf_main ) {
  return enkf_main->workflow_list;
}

bool enkf_main_run_workflow( enkf_main_type * enkf_main , const char * workflow ) {
  ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
  if (ert_workflow_list_has_workflow( workflow_list , workflow)){
      return ert_workflow_list_run_workflow_blocking( workflow_list , workflow , enkf_main);
  }
  else{
    return false;
  }
}


void enkf_main_run_workflows( enkf_main_type * enkf_main , const stringlist_type * workflows) {
  int iw;
  for (iw = 0; iw < stringlist_get_size( workflows ); iw++)
    enkf_main_run_workflow( enkf_main , stringlist_iget( workflows , iw ));
}


void enkf_main_load_from_forward_model_from_gui(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, enkf_fs_type * fs){
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  stringlist_type ** realizations_msg_list = util_calloc( ens_size , sizeof * realizations_msg_list );
  for (int iens = 0; iens < ens_size; ++iens)
    realizations_msg_list[iens] = stringlist_alloc_new();

  enkf_main_load_from_forward_model_with_fs(enkf_main, iter , iactive, realizations_msg_list, fs);

  for (int iens = 0; iens < ens_size; ++iens)
    stringlist_free( realizations_msg_list[iens] );

  free(realizations_msg_list);
}

void enkf_main_load_from_forward_model(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, stringlist_type ** realizations_msg_list){
  enkf_fs_type * fs         = enkf_main_get_fs( enkf_main );
  enkf_main_load_from_forward_model_with_fs(enkf_main, iter, iactive, realizations_msg_list, fs);
}


void enkf_main_load_from_forward_model_with_fs(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, stringlist_type ** realizations_msg_list, enkf_fs_type * fs) {

  const int ens_size        = enkf_main_get_ensemble_size( enkf_main );
  int result[ens_size];
  model_config_type * model_config = enkf_main->model_config;

  ert_run_context_type * run_context = ert_run_context_alloc_ENSEMBLE_EXPERIMENT( fs , iactive , model_config_get_runpath_fmt( model_config ) , enkf_main->subst_list , iter );
  arg_pack_type ** arg_list = util_calloc( ens_size , sizeof * arg_list );
  thread_pool_type * tp     = thread_pool_alloc( 4 , true );  /* num_cpu - HARD coded. */

  int iens = 0;
  for (; iens < ens_size; ++iens) {
    result[iens] = 0;
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_list[iens] = arg_pack;

    if (bool_vector_iget(iactive, iens)) {
      enkf_state_type * enkf_state = enkf_main_iget_state( enkf_main , iens );
      arg_pack_append_ptr( arg_pack , enkf_state);                                         /* 0: enkf_state*/
      arg_pack_append_ptr( arg_pack , ert_run_context_iens_get_arg( run_context , iens )); /* 1: run_arg */
      arg_pack_append_ptr(arg_pack, realizations_msg_list[iens]);                          /* 2: List of interactive mode messages. */
      arg_pack_append_bool( arg_pack, true );                                              /* 3: Manual load */
      arg_pack_append_ptr(arg_pack, &result[iens]);                                        /* 4: Result */
      thread_pool_add_job( tp , enkf_state_load_from_forward_model_mt , arg_pack);
    }
  }

  thread_pool_join( tp );
  thread_pool_free( tp );
  printf("\n");

  for (iens = 0; iens < ens_size; ++iens) {
    if (bool_vector_iget(iactive, iens)) {
      if (result[iens] & LOAD_FAILURE)
        fprintf(stderr, "** Warning: Function %s: Realization %d load failure\n", __func__, iens);
      else if (result[iens] & REPORT_STEP_INCOMPATIBLE)
        fprintf(stderr, "** Warning: Function %s: Reliazation %d report step incompatible\n", __func__, iens);
    }
    arg_pack_free(arg_list[iens]);
  }
  free( arg_list );
  ert_run_context_free( run_context );
}



char * enkf_main_alloc_abs_path_to_init_file(const enkf_main_type * enkf_main, const enkf_config_node_type * config_node) {
  model_config_type * model_config = enkf_main_get_model_config(enkf_main);
  char * runpath                   = NULL;
  char * path_to_init_file         = NULL;
  char * abs_path_to_init_file     = NULL;
  bool forward_init                = enkf_config_node_use_forward_init(config_node);

  if (forward_init) {
    path_fmt_type * runpath_fmt = model_config_get_runpath_fmt(model_config);
    runpath = path_fmt_alloc_path(runpath_fmt , false , 0, 0);  /* Replace first %d with iens, if a second %d replace with iter */
    path_to_init_file = enkf_config_node_alloc_initfile(config_node, runpath, 0);
  } else
    path_to_init_file = enkf_config_node_alloc_initfile(config_node, NULL, 0);

  if (path_to_init_file)
    abs_path_to_init_file = util_alloc_abs_path(path_to_init_file);

  if (abs_path_to_init_file && !util_file_exists(abs_path_to_init_file)) {
    free(abs_path_to_init_file);
    abs_path_to_init_file = NULL;
  }

  if (runpath)
    free(runpath);
  if (path_to_init_file)
    free(path_to_init_file);

  return abs_path_to_init_file;
}


bool enkf_main_export_field(const enkf_main_type * enkf_main,
                            const char * kw,
                            const char * path,
                            bool_vector_type * iactive,
                            field_file_format_type file_type,
                            int report_step)
{
    enkf_fs_type * fs = enkf_main_get_fs(enkf_main);
    bool result = enkf_main_export_field_with_fs(enkf_main, kw, path, iactive, file_type, report_step, fs);
    return result;
}




bool enkf_main_export_field_with_fs(const enkf_main_type * enkf_main,
                            const char * kw,
                            const char * path,
                            bool_vector_type * iactive,
                            field_file_format_type file_type,
                            int report_step,
                            enkf_fs_type * fs) {

  bool ret = false;
  if (util_int_format_count(path) < 1) {
    printf("EXPORT FIELD: There must be a %%d in the file name\n");
    return ret;
  }

  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config(enkf_main);
  const enkf_config_node_type * config_node = NULL;
  bool node_found = false;

  if (ensemble_config_has_key(ensemble_config, kw)) {
    config_node = ensemble_config_get_node(ensemble_config, kw);
    if (config_node && enkf_config_node_get_impl_type(config_node) == FIELD) {
      node_found = true;
    } else
      printf("Did not find a FIELD %s node\n", kw);
  } else
      printf("Ensemble config does not have key %s\n", kw);

  if (node_found) {
    enkf_node_type * node = NULL;

    char * init_file = enkf_main_alloc_abs_path_to_init_file(enkf_main, config_node);
    if (init_file)
      printf("init_file found: \"%s\", exporting initial value for inactive cells\n", init_file);
    else
      printf("no init_file found, exporting 0 or fill value for inactive cells\n");

    int iens;
    for (iens = 0; iens < bool_vector_size(iactive); ++iens) {
      if (bool_vector_iget(iactive, iens)) {
        node_id_type node_id = {.report_step = report_step , .iens = iens };
        node = enkf_state_get_node(enkf_main->ensemble[iens] , kw);
        if (node) {
          if (enkf_node_try_load(node , fs , node_id)) {
            path_fmt_type * export_path = path_fmt_alloc_path_fmt( path );
            char * filename = path_fmt_alloc_path( export_path , false , iens);
            path_fmt_free(export_path);

            {
              char * path;
              util_alloc_file_components(filename , &path , NULL , NULL);
              if (path != NULL) {
                util_make_path( path );
                free( path );
              }
            }

            {
              const field_type * field = enkf_node_value_ptr(node);
              const bool output_transform = true;
              field_export(field , filename , NULL , file_type , output_transform, init_file);
              ret = true;
            }
            free(filename);
          } else
            printf("%s : enkf_node_try_load returned returned false \n", __func__);
        } else
            printf("%s : enkf_state_get_node returned NULL for parameters  %d, %s \n", __func__, iens, kw);
       }
    }
    if (init_file)
      free(init_file);
  }


  if (ret)
    printf("Successful export of FIELD %s\n", kw);
  else
    printf("Errors during export of FIELD %s\n", kw);

  return ret;
}


void enkf_main_rank_on_observations(enkf_main_type * enkf_main,
                                    const char * ranking_key,
                                    const stringlist_type * obs_ranking_keys,
                                    const int_vector_type * steps) {

  enkf_fs_type               * fs              = enkf_main_get_fs(enkf_main);
  const enkf_obs_type        * enkf_obs        = enkf_main_get_obs( enkf_main );
  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config(enkf_main);
  const int history_length                     = enkf_main_get_history_length( enkf_main );
  const int ens_size                           = enkf_main_get_ensemble_size( enkf_main );

  misfit_ensemble_type * misfit_ensemble = enkf_fs_get_misfit_ensemble( fs );
  misfit_ensemble_initialize( misfit_ensemble , ensemble_config , enkf_obs , fs , ens_size , history_length, false);

  ranking_table_type * ranking_table = enkf_main_get_ranking_table( enkf_main );

  ranking_table_add_misfit_ranking( ranking_table , misfit_ensemble , obs_ranking_keys , steps , ranking_key );
  ranking_table_display_ranking( ranking_table , ranking_key);
}



void enkf_main_rank_on_data(enkf_main_type * enkf_main,
                            const char * ranking_key,
                            const char * data_key,
                            bool sort_increasing,
                            int step) {

  ranking_table_type * ranking_table     = enkf_main_get_ranking_table( enkf_main );
  ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config( enkf_main );
  enkf_fs_type * fs                      = enkf_main_get_fs(enkf_main);
  char * key_index;

  const enkf_config_node_type * config_node = ensemble_config_user_get_node( ensemble_config , data_key , &key_index);
  if (config_node) {
    ranking_table_add_data_ranking( ranking_table , sort_increasing , ranking_key , data_key , key_index , fs , config_node, step );
    ranking_table_display_ranking( ranking_table , ranking_key );
  } else {
    fprintf(stderr,"** No data found for key %s\n", data_key);
  }
}


void enkf_main_export_ranking(enkf_main_type * enkf_main, const char * ranking_key, const char * ranking_file) {
  ranking_table_type * ranking_table = enkf_main_get_ranking_table( enkf_main );
  ranking_table_fwrite_ranking(ranking_table, ranking_key, ranking_file);
}


#include "enkf_main_manage_fs.c"
