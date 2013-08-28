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
#include <ert/util/log.h>
#include <ert/util/node_ctype.h>
#include <ert/util/string_util.h>

#include <ert/config/config.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_io_config.h>

#include <ert/job_queue/job_queue.h>
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

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/enkf_sched.h>
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
#include <ert/enkf/qc_module.h>
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
#include <ert/enkf/ert_report_list.h>
#include <ert/enkf/ranking_table.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/runpath_list.h>

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
  char                 * current_fs_case;
  enkf_fs_type         * dbase;              /* The internalized information. */
  ensemble_config_type * ensemble_config;    /* The config objects for the various enkf nodes.*/
  qc_module_type       * qc_module;
  model_config_type    * model_config;
  ecl_config_type      * ecl_config;
  site_config_type     * site_config;
  analysis_config_type * analysis_config;
  local_config_type    * local_config;       /* Holding all the information about local analysis. */
  ert_templates_type   * templates;          /* Run time templates */
  log_type             * logh;               /* Handle to an open log file. */
  plot_config_type     * plot_config;        /* Information about plotting. */
  rng_config_type      * rng_config;
  rng_type             * rng;
  ert_report_list_type * report_list;
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




/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION(enkf_main , ENKF_MAIN_ID)

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


void enkf_main_set_eclbase( enkf_main_type * enkf_main , const char * eclbase_fmt) {
  ecl_config_set_eclbase( enkf_main->ecl_config , eclbase_fmt);
  for (int iens = 0; iens < enkf_main->ens_size; iens++) 
    enkf_state_update_eclbase( enkf_main->ensemble[iens] );
}

void enkf_main_init_jobname( enkf_main_type * enkf_main) {
  for (int iens = 0; iens < enkf_main->ens_size; iens++) 
    enkf_state_update_jobname( enkf_main->ensemble[iens] );
}


void enkf_main_set_jobname( enkf_main_type * enkf_main , const char * jobname_fmt) {
  model_config_set_jobname_fmt( enkf_main->model_config , jobname_fmt );
  enkf_main_init_jobname( enkf_main );
}


bool enkf_main_set_refcase( enkf_main_type * enkf_main , const char * refcase_path) {
  bool set_refcase = ecl_config_load_refcase( enkf_main->ecl_config , refcase_path );
  model_config_set_refcase( enkf_main->model_config , ecl_config_get_refcase( enkf_main->ecl_config ));
  ensemble_config_set_refcase( enkf_main->ensemble_config , ecl_config_get_refcase( enkf_main->ecl_config ));
  
  return set_refcase;
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

log_type * enkf_main_get_logh( const enkf_main_type * enkf_main ) {
  return enkf_main->logh;
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


bool enkf_main_has_QC_workflow( const enkf_main_type * enkf_main ) {
  return qc_module_has_workflow( enkf_main->qc_module );
}

qc_module_type * enkf_main_get_qc_module( const enkf_main_type * enkf_main ) {
  return enkf_main->qc_module;
}


/**
   Will do a forced reload of the observtaions; if the user has edited
   the content of the observation file while the ERT instance is
   running.
*/

void enkf_main_reload_obs( enkf_main_type * enkf_main) {
  enkf_obs_reload(enkf_main->obs , 
                  model_config_get_history(enkf_main->model_config), 
                  ecl_config_get_grid( enkf_main->ecl_config ),
                  ecl_config_get_refcase( enkf_main->ecl_config ) , 
                  analysis_config_get_std_cutoff(enkf_main->analysis_config),
                  enkf_main->ensemble_config );
}


/**
   Will not reload the observations if the input config file
   @obs_config_file is equal to the currently set config_file. If you
   want to force a reload of the observations use the function
   enkf_main_reload_obs().
*/

void enkf_main_load_obs( enkf_main_type * enkf_main , const char * obs_config_file ) {
  if (!util_string_equal( obs_config_file , enkf_obs_get_config_file( enkf_main->obs ))) {
    enkf_obs_load(enkf_main->obs , 
                  model_config_get_history(enkf_main->model_config), 
                  obs_config_file , 
                  ecl_config_get_grid( enkf_main->ecl_config ),
                  ecl_config_get_refcase( enkf_main->ecl_config ) , 
                  analysis_config_get_std_cutoff(enkf_main->analysis_config),
                  enkf_main->ensemble_config );
  }
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


void enkf_main_set_data_file( enkf_main_type * enkf_main , const char * data_file ) {
  ecl_config_set_data_file( enkf_main->ecl_config , data_file );
  enkf_main_update_num_cpu( enkf_main );
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
  
  enkf_obs_free(enkf_main->obs);
  ranking_table_free( enkf_main->ranking_table );
  enkf_main_free_ensemble( enkf_main );
  if (enkf_main->dbase != NULL) enkf_fs_close( enkf_main->dbase );
  util_safe_free( enkf_main->current_fs_case );

  if (log_is_open( enkf_main->logh ))
    log_add_message( enkf_main->logh , false , NULL , "Exiting ert application normally - all is fine(?)" , false);
  log_close( enkf_main->logh );

  analysis_config_free(enkf_main->analysis_config);
  ecl_config_free(enkf_main->ecl_config);
  model_config_free( enkf_main->model_config);
  
  
  qc_module_free( enkf_main->qc_module );
  site_config_free( enkf_main->site_config);
  ensemble_config_free( enkf_main->ensemble_config );
  
  if (enkf_main->local_config != NULL)
    local_config_free( enkf_main->local_config );

  ert_report_list_free( enkf_main->report_list );
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



static void enkf_main_load_sub_ensemble(enkf_main_type * enkf_main , int mask , int report_step , state_enum state, int iens1 , int iens2) {
  int iens;
  for (iens = iens1; iens < iens2; iens++)
    enkf_state_fread(enkf_main->ensemble[iens] , enkf_main_get_fs( enkf_main ) , mask , report_step , state );
}


static void * enkf_main_load_sub_ensemble__(void * __arg) {
  arg_pack_type * arg_pack   = arg_pack_safe_cast(__arg);
  enkf_main_type * enkf_main = arg_pack_iget_ptr(arg_pack , 0);
  int mask                   = arg_pack_iget_int(arg_pack , 1);
  int report_step            = arg_pack_iget_int(arg_pack , 2);
  state_enum state           = arg_pack_iget_int(arg_pack , 3);
  int iens1                  = arg_pack_iget_int(arg_pack , 4);
  int iens2                  = arg_pack_iget_int(arg_pack , 5);

  enkf_main_load_sub_ensemble(enkf_main , mask , report_step , state , iens1 , iens2);
  return NULL;
}



void enkf_main_load_ensemble(enkf_main_type * enkf_main , int mask , int report_step , state_enum state) {
  const   int cpu_threads = 4;
  int     sub_ens_size    = enkf_main_get_ensemble_size(enkf_main) / cpu_threads;
  int     icpu;
  thread_pool_type * tp          = thread_pool_alloc( cpu_threads , true );
  arg_pack_type ** arg_pack_list = util_calloc( cpu_threads , sizeof * arg_pack_list );

  for (icpu = 0; icpu < cpu_threads; icpu++) {
    arg_pack_type * arg = arg_pack_alloc();
    arg_pack_append_ptr(arg , enkf_main);
    arg_pack_append_int(arg , mask);
    arg_pack_append_int(arg , report_step);
    arg_pack_append_int(arg , state);

    {
      int iens1 =  icpu * sub_ens_size;
      int iens2 = iens1 + sub_ens_size;

      if (icpu == (cpu_threads - 1))
        iens2 = enkf_main_get_ensemble_size(enkf_main);

      arg_pack_append_int(arg ,  iens1);
      arg_pack_append_int(arg ,  iens2);
    }
    arg_pack_list[icpu] = arg;
    arg_pack_lock( arg );
    thread_pool_add_job( tp , enkf_main_load_sub_ensemble__ , arg);
  }
  thread_pool_join( tp );
  thread_pool_free( tp );

  for (icpu = 0; icpu < cpu_threads; icpu++)
    arg_pack_free( arg_pack_list[icpu] );
  free(arg_pack_list);
}







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

enkf_node_type ** enkf_main_get_node_ensemble(const enkf_main_type * enkf_main , const char * key , int report_step , state_enum load_state) {
  enkf_fs_type * fs               = enkf_main_get_fs( enkf_main );
  const int ens_size              = enkf_main_get_ensemble_size( enkf_main );
  enkf_node_type ** node_ensemble = util_calloc(ens_size , sizeof * node_ensemble );
  node_id_type node_id = {.report_step = report_step , 
                          .state       = load_state , 
                          .iens        = -1 };
  int iens;
  

  for (iens = 0; iens < ens_size; iens++) {
    node_ensemble[iens] = enkf_state_get_node(enkf_main->ensemble[iens] , key);
    node_id.iens = iens;
    enkf_node_load( node_ensemble[iens] , fs , node_id);
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


void enkf_main_inflate_node(enkf_main_type * enkf_main , enkf_fs_type * target_fs , int report_step , const char * key , const enkf_node_type * min_std) {
  int ens_size                              = enkf_main_get_ensemble_size(enkf_main);
  enkf_node_type ** ensemble                = enkf_main_get_node_ensemble( enkf_main , key , report_step , ANALYZED );
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
    node_id_type node_id = {.report_step = report_step , .iens = iens , .state = ANALYZED };
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

void enkf_main_inflate(enkf_main_type * enkf_main , enkf_fs_type * target_fs , int report_step , hash_type * use_count) {
  stringlist_type * keys = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER + DYNAMIC_STATE);
  msg_type * msg = msg_alloc("Inflating:" , false);

  msg_show( msg );
  for (int ikey = 0; ikey < stringlist_get_size( keys ); ikey++) {
    const char * key = stringlist_iget( keys  , ikey );
    if (hash_get_counter(use_count , key) > 0) {
      const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , key );
      const enkf_node_type * min_std            = enkf_config_node_get_min_std( config_node );
      
      if (min_std != NULL) {
        msg_update( msg , key );
        enkf_main_inflate_node(enkf_main , target_fs , report_step , key , min_std );
      }
    }
  }
  stringlist_free( keys );
  msg_free( msg , true );
}




static int __get_active_size(enkf_main_type * enkf_main , const char * key, int report_step , const active_list_type * active_list) {
  const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , key );
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
      enkf_node_type * node = enkf_state_get_node( enkf_main->ensemble[0] , key);
      node_id_type node_id = {.report_step = report_step , 
                              .iens        = 0,
                              .state       = FORECAST };

      enkf_node_load( node , enkf_main->dbase , node_id );
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
  state_enum                load_state;
  run_mode_type             run_mode;  
  int                       row_offset;
  const active_list_type  * active_list;
  matrix_type             * A;
} serialize_info_type;



static void serialize_node( enkf_fs_type * fs , 
                            enkf_state_type ** ensemble , 
                            const char * key , 
                            int iens , 
                            int report_step , 
                            state_enum load_state , 
                            int row_offset , 
                            const active_list_type * active_list,
                            matrix_type * A) {

  enkf_node_type * node = enkf_state_get_node( ensemble[iens] , key);
  node_id_type node_id = {.report_step = report_step, .iens = iens , .state = load_state };
  enkf_node_serialize( node , fs , node_id , active_list , A , row_offset , iens);
}


static void * serialize_nodes_mt( void * arg ) {
  serialize_info_type * info = (serialize_info_type *) arg;
  int iens;
  for (iens = info->iens1; iens < info->iens2; iens++) 
    serialize_node( info->src_fs , info->ensemble , info->key , iens , info->report_step , info->load_state , info->row_offset , info->active_list , info->A );
  
  return NULL;
}


static void enkf_main_serialize_node( const char * node_key , 
                                      state_enum load_state , 
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
    serialize_info[icpu].load_state  = load_state;
    serialize_info[icpu].row_offset  = row_offset;
    
    thread_pool_add_job( work_pool , serialize_nodes_mt , &serialize_info[icpu]);
  }
  thread_pool_join( work_pool );
}



/**
   The return value is the number of rows in the serialized
   A matrix. 
*/

static int enkf_main_serialize_dataset( enkf_main_type * enkf_main, 
                                        const local_dataset_type * dataset ,
                                        int report_step,
                                        hash_type * use_count ,  
                                        int * active_size , 
                                        int * row_offset,
                                        thread_pool_type * work_pool,
                                        serialize_info_type * serialize_info ) {

  matrix_type * A   = serialize_info->A;
  stringlist_type * update_keys = local_dataset_alloc_keys( dataset );
  const int num_kw  = stringlist_get_size( update_keys );
  int ens_size      = matrix_get_columns( A );
  int current_row   = 0;
  
  for (int ikw=0; ikw < num_kw; ikw++) {
    const char             * key         = stringlist_iget(update_keys , ikw);
    enkf_config_node_type * config_node  = ensemble_config_get_node( enkf_main->ensemble_config , key );
    if ((serialize_info[0].run_mode == SMOOTHER_UPDATE) && (enkf_config_node_get_var_type( config_node ) != PARAMETER)) {
      /* We have tried to serialize a dynamic node when we are
         smoother update mode; that does not make sense and we just
         continue. */
      active_size[ikw] = 0;
      continue;
    } else {
      const active_list_type * active_list      = local_dataset_get_node_active_list( dataset , key );
      
      active_size[ikw] = __get_active_size( enkf_main , key , report_step , active_list );
      row_offset[ikw]  = current_row;
      {
        int matrix_rows = matrix_get_rows( A );
        if ((active_size[ikw] + current_row) > matrix_rows) 
          matrix_resize( A , matrix_rows + 2 * active_size[ikw] , ens_size , true );
      }
      
      if (active_size[ikw] > 0) {
        state_enum load_state;
        
        if (hash_inc_counter( use_count , key) == 0)
          load_state = FORECAST;           /* This is the first time this keyword is updated for this reportstep */
        else
          load_state = ANALYZED;
        
        enkf_main_serialize_node( key , load_state , active_list , row_offset[ikw] , work_pool , serialize_info );
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
                              const active_list_type * active_list,
                              matrix_type * A) {
  
  enkf_node_type * node = enkf_state_get_node( ensemble[iens] , key);
  node_id_type node_id = { .report_step = target_step , .iens = iens , .state = ANALYZED };
  enkf_node_deserialize(node , fs , node_id , active_list , A , row_offset , iens);
}



static void * deserialize_nodes_mt( void * arg ) {
  serialize_info_type * info = (serialize_info_type *) arg;
  int iens;
  for (iens = info->iens1; iens < info->iens2; iens++) 
    deserialize_node( info->target_fs , info->ensemble , info->key , iens , info->target_step , info->row_offset , info->active_list , info->A );

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

static serialize_info_type * serialize_info_alloc( enkf_fs_type * src_fs, enkf_fs_type * target_fs , int target_step , enkf_state_type ** ensemble , run_mode_type run_mode , int report_step , matrix_type * A , int num_cpu_threads ) {
  serialize_info_type * serialize_info = util_calloc( num_cpu_threads , sizeof * serialize_info );
  int ens_size = matrix_get_columns( A );
  int icpu;
  int iens_offset = 0;
  for (icpu = 0; icpu < num_cpu_threads; icpu++) {
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


void enkf_main_get_PC( const enkf_main_type * enkf_main , 
                       const matrix_type * S, 
                       const matrix_type * dObs,
                       const char * obsset_name , 
                       int step1 , int step2 , 
                       double truncation , 
                       int ncomp , 
                       matrix_type * PC , 
                       matrix_type * PC_obs) {

  enkf_linalg_get_PC( S , dObs , truncation , ncomp , PC , PC_obs);
  {
    char * filename  = util_alloc_sprintf(analysis_config_get_PC_filename( enkf_main->analysis_config ) , step1 , step2 , obsset_name);
    char * full_path = util_alloc_filename( analysis_config_get_PC_path( enkf_main->analysis_config) , filename , NULL );
    FILE * stream   = util_mkdir_fopen(full_path , "w");
    {
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
    }
    fclose( stream );
    free( filename );
    free( full_path );
  }

}




static void enkf_main_analysis_update( enkf_main_type * enkf_main , 
                                       enkf_fs_type * target_fs ,
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
  analysis_module_type * module = analysis_config_get_active_module( enkf_main->analysis_config );
  int ens_size          = meas_data_get_ens_size( forecast );
  int active_size       = obs_data_get_active_size( obs_data );
  matrix_type * X       = matrix_alloc( ens_size , ens_size );
  matrix_type * S       = meas_data_allocS( forecast , active_size );
  matrix_type * R       = obs_data_allocR( obs_data , active_size );
  matrix_type * dObs    = obs_data_allocdObs( obs_data , active_size );
  matrix_type * A       = matrix_alloc( matrix_start_size , ens_size );
  matrix_type * E       = NULL;
  matrix_type * D       = NULL;
  matrix_type * localA  = NULL;

  


  if (analysis_module_get_option( module , ANALYSIS_NEED_ED)) {
    E = obs_data_allocE( obs_data , enkf_main->rng , ens_size , active_size );
    D = obs_data_allocD( obs_data , E , S );
  }

  if (analysis_module_get_option( module , ANALYSIS_SCALE_DATA))
    obs_data_scale( obs_data , S , E , D , R , dObs );
  
  if (analysis_module_get_option( module , ANALYSIS_USE_A | ANALYSIS_UPDATE_A)) 
    localA = A;

  /*****************************************************************/
  
  analysis_module_init_update( module , S , R , dObs , E , D );
  {
    hash_iter_type * dataset_iter = local_ministep_alloc_dataset_iter( ministep );
    enkf_fs_type * src_fs = enkf_main_get_fs( enkf_main );
    serialize_info_type * serialize_info = serialize_info_alloc( src_fs , target_fs ,  target_step , enkf_main_get_ensemble( enkf_main ) , run_mode , step2 , A , cpu_threads);
    
    // Store PC:
    if (analysis_config_get_store_PC( enkf_main->analysis_config )) {
      double truncation    = -1;
      int ncomp            = ens_size - 1;
      matrix_type * PC     = matrix_alloc(1,1);
      matrix_type * PC_obs = matrix_alloc(1,1);
      
      enkf_main_get_PC( enkf_main , S , dObs , local_ministep_get_name( ministep ) , step1 , step2 , truncation , ncomp , PC , PC_obs );
      
      matrix_free( PC );
      matrix_free( PC_obs );
    }
    
    if (localA == NULL) 
      analysis_module_initX( module , X , NULL , S , R , dObs , E , D );

    while (!hash_iter_is_complete( dataset_iter )) {
      const char * dataset_name = hash_iter_get_next_key( dataset_iter );
      const local_dataset_type * dataset = local_ministep_get_dataset( ministep , dataset_name );
      if (local_dataset_get_size( dataset )) {
        int * active_size = util_calloc( local_dataset_get_size( dataset ) , sizeof * active_size );
        int * row_offset  = util_calloc( local_dataset_get_size( dataset ) , sizeof * row_offset  );
        
        enkf_main_serialize_dataset( enkf_main , dataset , step2 ,  use_count , active_size , row_offset , tp , serialize_info);

        if (analysis_module_get_option( module , ANALYSIS_UPDATE_A))
          analysis_module_updateA( module , localA , S , R , dObs , E , D );
        else {
          if (analysis_module_get_option( module , ANALYSIS_USE_A)) 
            analysis_module_initX( module , X , localA , S , R , dObs , E , D );

          matrix_inplace_matmul_mt2( A , X , tp );
        }
       
        // The deserialize also calls enkf_node_store() functions.
        enkf_main_deserialize_dataset( enkf_main_get_ensemble_config( enkf_main ) , dataset , active_size , row_offset , serialize_info , tp);
        
        free( active_size );
        free( row_offset );
      }
    }

    hash_iter_free( dataset_iter );
    serialize_info_free( serialize_info );
  }
  analysis_module_complete_update( module );
    

  /*****************************************************************/

  matrix_safe_free( E );
  matrix_safe_free( D );
  matrix_free( S );
  matrix_free( R );
  matrix_free( dObs );
  matrix_free( X );
}




/**
   This is  T H E  EnKF update routine.
**/


static void enkf_main_UPDATE(enkf_main_type * enkf_main , const int_vector_type * step_list, enkf_fs_type * target_fs , int target_step , run_mode_type run_mode) {
  /*
     If merge_observations is true all observations in the time
     interval [step1+1,step2] will be used, otherwise only the last
     observation at step2 will be used.
  */
  const int ens_size = enkf_main_get_ensemble_size(enkf_main);
  double alpha       = analysis_config_get_alpha( enkf_main->analysis_config );
  double std_cutoff  = analysis_config_get_std_cutoff( enkf_main->analysis_config );
  int current_step   = int_vector_get_last( step_list );
  {
    /*
      Observations and measurements are collected in these temporary
      structures. obs_data is a precursor for the 'd' vector, and
      meas_forecast is a precursor for the 'S' matrix'.

      The reason for going via these temporary structures is to support
      deactivating observations which should not be used in the update
      process.
    */
    obs_data_type               * obs_data      = obs_data_alloc();
    meas_data_type              * meas_forecast = meas_data_alloc( ens_size );
    meas_data_type              * meas_analyzed = meas_data_alloc( ens_size );
    local_config_type           * local_config  = enkf_main->local_config;
    const local_updatestep_type * updatestep    = local_config_iget_updatestep( local_config , current_step );  /* Only last step considered when forming local update */
    hash_type                   * use_count     = hash_alloc();
    const char                  * log_path      = analysis_config_get_log_path( enkf_main->analysis_config );
    FILE                        * log_stream;

    
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
      local_obsset_type   * obsset   = local_ministep_get_obsset( ministep );

      obs_data_reset( obs_data );
      meas_data_reset( meas_forecast );
      
      enkf_obs_get_obs_and_measure( enkf_main->obs, 
                                    enkf_main_get_fs(enkf_main), 
                                    step_list , 
                                    FORECAST, 
                                    ens_size,
                                    (const enkf_state_type **) enkf_main->ensemble, 
                                    meas_forecast, 
                                    obs_data , 
                                    obsset );
      

      enkf_analysis_deactivate_outliers( obs_data , meas_forecast  , std_cutoff , alpha);
      
      if (enkf_main->verbose)
        enkf_analysis_fprintf_obs_summary( obs_data , meas_forecast  , step_list , local_ministep_get_name( ministep ) , stdout );
      enkf_analysis_fprintf_obs_summary( obs_data , meas_forecast  , step_list , local_ministep_get_name( ministep ) , log_stream );

      if (obs_data_get_active_size(obs_data) > 0)
        enkf_main_analysis_update( enkf_main , 
                                   target_fs , 
                                   target_step , 
                                   use_count , 
                                   run_mode , 
                                   int_vector_get_first( step_list ), 
                                   current_step , 
                                   ministep , 
                                   meas_forecast , 
                                   obs_data );
    }
    fclose( log_stream );

    obs_data_free( obs_data );
    meas_data_free( meas_forecast );
    meas_data_free( meas_analyzed );
    
    enkf_main_inflate( enkf_main , target_fs , current_step , use_count);
    hash_free( use_count );
  }
}

void enkf_main_assimilation_update(enkf_main_type * enkf_main , const int_vector_type * step_list) {
  enkf_main_UPDATE( enkf_main , step_list , enkf_main_get_fs( enkf_main ) , int_vector_get_last( step_list ) , ENKF_ASSIMILATION );
}


void enkf_main_smoother_update(enkf_main_type * enkf_main , const int_vector_type * step_list , enkf_fs_type * target_fs) {
  enkf_main_UPDATE( enkf_main , step_list , target_fs , 0 , SMOOTHER_UPDATE );
}




static void enkf_main_report_run_failure( const enkf_main_type * enkf_main , int iens) {
  job_queue_type * job_queue = site_config_get_job_queue(enkf_main->site_config);
  const enkf_state_type * enkf_state = enkf_main_iget_state( enkf_main , iens );
  int queue_index = enkf_state_get_queue_index( enkf_state );

  const char * stderr_file = job_queue_iget_stderr_file( job_queue , queue_index );
  if (stderr_file == NULL) 
    log_add_fmt_message( enkf_main->logh , 1 , stderr , "** ERROR ** path:%s  job:%s  reason:%s" , 
                         job_queue_iget_run_path( job_queue , queue_index), 
                         job_queue_iget_failed_job( job_queue , queue_index),
                         job_queue_iget_error_reason( job_queue , queue_index ));
  else
    log_add_fmt_message( enkf_main->logh , 1 , stderr , "** ERROR ** path:%s  job:%s  reason:%s  Check file:%s" , 
                         job_queue_iget_run_path( job_queue , queue_index), 
                         job_queue_iget_failed_job( job_queue , queue_index),
                         job_queue_iget_error_reason( job_queue , queue_index ),
                         job_queue_iget_stderr_file( job_queue , queue_index ));
}



static void enkf_main_report_load_failure( const enkf_main_type * enkf_main , int iens) {
  job_queue_type * job_queue = site_config_get_job_queue(enkf_main->site_config);
  const enkf_state_type * enkf_state = enkf_main_iget_state( enkf_main , iens );
  int queue_index = enkf_state_get_queue_index( enkf_state );

  log_add_fmt_message( enkf_main->logh , 1 , stderr , "** ERROR ** path:%s - Could not load all required data",
                       job_queue_iget_run_path( job_queue , queue_index));
}



/**
  If all simulations have completed successfully the function will
  return true, otherwise it will return false.  
*/


static bool enkf_main_run_step(enkf_main_type * enkf_main       ,
                               run_mode_type    run_mode        ,
                               const bool_vector_type * iactive ,
                               int load_start                   ,      /* For internalizing results, and the first step in the update when merging. */
                               int init_step_parameter          ,
                               state_enum init_state_parameter  ,
                               state_enum init_state_dynamic    ,
                               int step1                        ,
                               int step2) {



  
  if (step1 > 0)
    ecl_config_assert_restart( enkf_main_get_ecl_config( enkf_main ) );

  {
    bool     verbose_queue   = enkf_main->verbose;
    int  max_internal_submit = model_config_get_max_internal_submit(enkf_main->model_config);
    const int ens_size       = enkf_main_get_ensemble_size( enkf_main );
    int   job_size;
    int iens;

    if (enkf_main->verbose) {
      if (run_mode == ENKF_ASSIMILATION)
        printf("Starting forward step: %d -> %d\n",step1 , step2);
    }

    log_add_message(enkf_main->logh , 1 , NULL , "===================================================================", false);
    if (run_mode == ENKF_ASSIMILATION)
      log_add_fmt_message(enkf_main->logh , 1 , NULL , "Forward model: %d -> %d ",step1,step2);
    else
      log_add_fmt_message(enkf_main->logh , 1 , NULL , "Forward model: %d -> ??? ",step1);

    job_size = bool_vector_count_equal( iactive , true );
    {
      pthread_t        queue_thread;
      job_queue_type * job_queue = site_config_get_job_queue(enkf_main->site_config);
      
      /* Start the queue */
      if (run_mode != INIT_ONLY) {
        arg_pack_type  * queue_args = arg_pack_alloc();    /* This arg_pack will be freed() in the job_que_run_jobs__() */
        arg_pack_append_ptr(queue_args  , job_queue);
        arg_pack_append_int(queue_args  , job_size);
        arg_pack_append_bool(queue_args , verbose_queue);
        pthread_create( &queue_thread , NULL , job_queue_run_jobs__ , queue_args);
      }

      
      {
        thread_pool_type * submit_threads = thread_pool_alloc( 4 , true );
        enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
        runpath_list_type * runpath_list = qc_module_get_runpath_list( enkf_main->qc_module );
        runpath_list_clear( runpath_list );
        
        for (iens = 0; iens < ens_size; iens++) {
          enkf_state_type * enkf_state = enkf_main->ensemble[iens];
          if (bool_vector_safe_iget(iactive , iens)) {
            int load_start = step1;
            if (step1 > 0)
              load_start++;
            
            enkf_state_init_run(enkf_state , 
                                run_mode ,
                                true , 
                                max_internal_submit ,
                                init_step_parameter ,
                                init_state_parameter,
                                init_state_dynamic  ,
                                load_start ,
                                step1 ,
                                step2 );

            runpath_list_add( runpath_list , 
                              iens , 
                              enkf_state_get_run_path( enkf_state ) , 
                              enkf_state_get_eclbase( enkf_state ));
            {
              arg_pack_type * arg_pack = arg_pack_alloc( );   // This is discarded by the enkf_state_start_forward_model__() function. */
              
              arg_pack_append_ptr( arg_pack , enkf_state );
              arg_pack_append_ptr( arg_pack , fs );
              
              thread_pool_add_job(submit_threads , enkf_state_start_forward_model__ , arg_pack);
            }
          } else
            enkf_state_set_inactive( enkf_state );
        }
        /*
          After this join all directories/files for the simulations
          have been set up correctly, and all the jobs have been added
          to the job_queue manager.
        */
        qc_module_export_runpath_list( enkf_main->qc_module );
        thread_pool_join(submit_threads);        
        thread_pool_free(submit_threads);        
      }
      if (run_mode != INIT_ONLY) {
        job_queue_submit_complete( job_queue );
        log_add_message(enkf_main->logh , 1 , NULL , "All jobs submitted to internal queue - waiting for completion" ,  false);
        pthread_join( queue_thread , NULL );   /* Wait for the job_queue_run_jobs() function to complete. */
      }
    }

    
    /* This should be carefully checked for the situation where only a
       subset (with offset > 0) of realisations are simulated. */
    if (run_mode != INIT_ONLY) {
      bool totalOK = true;
      for (iens = 0; iens < ens_size; iens++) {    
        if (bool_vector_iget(iactive , iens)) {
          run_status_type run_status = enkf_state_get_simple_run_status( enkf_main->ensemble[iens] );

          switch (run_status) {
          case JOB_RUN_FAILURE:
            enkf_main_report_run_failure( enkf_main , iens );
            break;
          case JOB_LOAD_FAILURE:
            enkf_main_report_load_failure( enkf_main , iens );
            break;
          case JOB_RUN_OK:
            break;
          default:
            util_abort("%s: invalid job status:%s \n",__func__ , run_status );
          }
          totalOK = totalOK && ( run_status == JOB_RUN_OK );

        }
      }
      enkf_fs_fsync( enkf_main->dbase );
      if (totalOK) {
        log_add_fmt_message(enkf_main->logh , 1 , NULL , "All jobs complete and data loaded.");
        if (run_mode != ENKF_ASSIMILATION)
          qc_module_run_workflow( enkf_main->qc_module , enkf_main );
      }
      return totalOK;
    } else
      return true;
  }
}

/**
   The special value stride == 0 means to just include step2.
*/
int_vector_type * enkf_main_update_alloc_step_list( const enkf_main_type * enkf_main , int load_start , int step2 , int stride) {
  int_vector_type * step_list = int_vector_alloc( 0 , 0 );
  
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


void enkf_main_init_run( enkf_main_type * enkf_main, run_mode_type run_mode) {
  const ext_joblist_type * joblist = site_config_get_installed_jobs( enkf_main->site_config);

  model_config_set_enkf_sched( enkf_main->model_config , joblist , run_mode );
  enkf_main_init_internalization(enkf_main , run_mode);
}




void enkf_main_run_exp(enkf_main_type * enkf_main            ,
                       const bool_vector_type * iactive      , 
                       bool             simulate , 
                       int              init_step_parameters ,
                       int              start_report         ,
                       state_enum       start_state) {

  bool force_init = false;
  int ens_size    = enkf_main_get_ensemble_size( enkf_main );
  run_mode_type run_mode = simulate ? ENSEMBLE_EXPERIMENT : INIT_ONLY;
  {
    stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
    enkf_main_initialize_from_scratch( enkf_main , param_list , 0 , ens_size - 1, force_init);
    stringlist_free( param_list );
  }  
  enkf_main_init_run( enkf_main , run_mode );
  {
    int load_start                  = start_report;
    state_enum init_state_parameter = start_state;
    state_enum init_state_dynamic   = start_state;
    enkf_main_run_step(enkf_main , run_mode , iactive , load_start , init_step_parameters , init_state_parameter , init_state_dynamic , start_report , -1);
  }
}



void enkf_main_run_assimilation(enkf_main_type * enkf_main            ,
                                const bool_vector_type * iactive      , 
                                int              init_step_parameters ,
                                int              start_report         ,
                                state_enum       start_state) {
  bool force_init = false;
  int ens_size = enkf_main_get_ensemble_size( enkf_main );
  {
    stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
    enkf_main_initialize_from_scratch( enkf_main , param_list , 0 , ens_size - 1 , force_init );
    stringlist_free( param_list );
  }  
  bool rerun       = analysis_config_get_rerun( enkf_main->analysis_config );
  int  rerun_start = analysis_config_get_rerun_start( enkf_main->analysis_config );
  enkf_main_init_run( enkf_main , ENKF_ASSIMILATION);
  {
    bool analyzed_start = false;
    bool prev_enkf_on;
    const enkf_sched_type * enkf_sched = model_config_get_enkf_sched(enkf_main->model_config);
    const int num_nodes                = enkf_sched_get_num_nodes(enkf_sched);
    const int start_inode              = enkf_sched_get_node_index(enkf_sched , start_report);
    int inode;
    
    if (start_state == ANALYZED)
      analyzed_start = true;
    else if (start_state == FORECAST)
      analyzed_start = false;
    else
      util_abort("%s: internal error - start_state must be analyzed | forecast \n",__func__);
    
    prev_enkf_on = analyzed_start;
    for (inode = start_inode; inode < num_nodes; inode++) {
      const enkf_sched_node_type * node = enkf_sched_iget_node(enkf_sched , inode);
      state_enum init_state_parameter;
      state_enum init_state_dynamic;
      int      init_step_parameter;
      int      load_start;
      int      report_step1;
      int      report_step2;
      bool     enkf_on;


      enkf_sched_node_get_data(node , &report_step1 , &report_step2 , &enkf_on );
      if (inode == start_inode)
        report_step1 = start_report;  /* If we are restarting from somewhere. */
      
      if (rerun) {
        /* rerun ... */
        load_start           = report_step1;    /* +1 below. Observe that report_step is set to rerun_start below. */
        init_step_parameter  = report_step1;
        init_state_dynamic   = FORECAST;
        init_state_parameter = ANALYZED;
        report_step1         = rerun_start;
      } else {
        if (prev_enkf_on)
          init_state_dynamic = ANALYZED;
        else
          init_state_dynamic = FORECAST;
        /*
          This is not a rerun - and then parameters and dynamic
          data should be initialized from the same report step.
        */
        init_step_parameter  = report_step1;
        init_state_parameter = init_state_dynamic;
        load_start = report_step1;
      }
      
      if (load_start > 0)
        load_start++;
      
      {
        bool runOK = enkf_main_run_step(enkf_main , ENKF_ASSIMILATION , iactive , load_start , init_step_parameter , 
                                        init_state_parameter , init_state_dynamic , report_step1 , report_step2);
        
        if (runOK) {
          if (enkf_on) {
            bool merge_observations = analysis_config_get_merge_observations( enkf_main->analysis_config );
            int_vector_type * step_list;
            int stride;

            if (merge_observations)
              stride = 1;
            else
              stride = 0;
            
            step_list = enkf_main_update_alloc_step_list( enkf_main , load_start , report_step2 , stride );

            enkf_main_assimilation_update(enkf_main , step_list);
            int_vector_free( step_list );
            enkf_fs_fsync( enkf_main->dbase );
          }
        } else 
          util_exit("Problems with the forward model - exiting.\n");
        
        prev_enkf_on = enkf_on;
      }
    }
  }
}


void enkf_main_run_smoother(enkf_main_type * enkf_main , const char * target_fs_name , bool rerun) {
  bool force_init = false;
  int ens_size = enkf_main_get_ensemble_size( enkf_main );
  {
    stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
    enkf_main_initialize_from_scratch( enkf_main , param_list , 0 , ens_size - 1 , force_init);
    stringlist_free( param_list );
  }
  
  {
    bool_vector_type * iactive = bool_vector_alloc( 0 , true );
    bool_vector_iset( iactive , ens_size - 1 , true );

    enkf_main_init_run( enkf_main , ENSEMBLE_EXPERIMENT );
    if (enkf_main_run_step(enkf_main , ENSEMBLE_EXPERIMENT , iactive , 0 , 0 , ANALYZED , UNDEFINED , 0 , 0)) {
      time_map_type * time_map = enkf_fs_get_time_map( enkf_main_get_fs( enkf_main ));
      enkf_fs_type * target_fs = enkf_main_get_alt_fs( enkf_main , target_fs_name , false , true );
      {
        int stride = 1;
        int_vector_type * step_list = enkf_main_update_alloc_step_list( enkf_main , 0 , time_map_get_last_step( time_map ) , stride);
        enkf_main_smoother_update( enkf_main , step_list , target_fs );
        int_vector_free( step_list );
      }
      enkf_main_set_fs( enkf_main , target_fs , target_fs_name);
      
      if (rerun) { 
        /* 
           IFF a rerun path has been added with the RERUN_PATH config
           key the model_config object will select that runpath as the
           currently active one. If no path has been created with the
           RERUN_PATH config option the model_config_select_runpath()
           call will fail silently.

           The runpath select with this call will remain the currently
           active runpath for the remaining part of this program
           invocation.
        */
        model_config_select_runpath( enkf_main_get_model_config( enkf_main ) , RERUN_PATH_KEY );  
        enkf_main_run_step(enkf_main , ENSEMBLE_EXPERIMENT , iactive , 0 , 0 , ANALYZED , UNDEFINED , 0 , 0 );
      }
    }

    bool_vector_free( iactive );
  }
}


/*****************************************************************/
/*  Filesystem copy functions                                    */


void enkf_main_copy_ensemble(enkf_main_type * enkf_main        , 
                             const char * source_case          , 
                             int          source_report_step   ,
                             state_enum   source_state         ,
                             const char * target_case          ,  
                             int          target_report_step   ,
                             state_enum   target_state         , 
                             const bool_vector_type * iens_mask,
                             const char * ranking_key ,    /* It is OK to supply NULL - but if != NULL it must exist */
                             const stringlist_type * node_list) {

  /**
     Must start by setting up the enkf_fs instance to read and write
     from the correct cases.
  */

  const int ens_size            = enkf_main_get_ensemble_size( enkf_main );
  
  {
    enkf_fs_type * source_fs = enkf_main_get_alt_fs( enkf_main , source_case , true , false );
    enkf_fs_type * target_fs = enkf_main_get_alt_fs( enkf_main , target_case , false , true );
    {
      int * ranking_permutation;
      int inode , src_iens;
      
      if (ranking_key != NULL) {
        ranking_table_type * ranking_table = enkf_main_get_ranking_table( enkf_main );
        ranking_permutation = (int *) ranking_table_get_permutation( ranking_table , ranking_key );
      } else {
        ranking_permutation = util_calloc( ens_size , sizeof * ranking_permutation );
        for (src_iens = 0; src_iens < ens_size; src_iens++)
          ranking_permutation[src_iens] = src_iens;
      }
      
      for (inode =0; inode < stringlist_get_size( node_list ); inode++) {
        enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , stringlist_iget( node_list , inode ));
        for (src_iens = 0; src_iens < enkf_main_get_ensemble_size( enkf_main ); src_iens++) {
          if (bool_vector_safe_iget(iens_mask , src_iens)) {
            int target_iens = ranking_permutation[src_iens];
            node_id_type src_id    = {.report_step = source_report_step , .iens = src_iens    , .state = source_state };
            node_id_type target_id = {.report_step = target_report_step , .iens = target_iens , .state = target_state };
            
            enkf_node_copy( config_node , 
                            source_fs , target_fs , 
                            src_id , target_id );
            
          }
        }
      }

      if (ranking_permutation == NULL) 
        free( ranking_permutation );
    }

    enkf_main_close_alt_fs( enkf_main , source_fs );
    enkf_main_close_alt_fs( enkf_main , target_fs );
  }
}






/**
   This is based on a general copy function, but a couple of variables
   have been set to default values because this is an initialization:

   target_step  = 0
   target_state = analyzed
   
*/

void enkf_main_initialize_from_existing__(enkf_main_type * enkf_main , 
                                          const char * source_case , 
                                          int          source_report_step,
                                          state_enum   source_state,
                                          const bool_vector_type * iens_mask,
                                          const char * ranking_key ,    /* It is OK to supply NULL - but if != NULL it must exist */
                                          const stringlist_type * node_list) {
  
  const int target_report_step  = 0;
  const state_enum target_state = ANALYZED;
  const char * target_case      = NULL;

  enkf_main_copy_ensemble(enkf_main , 
                          source_case , source_report_step , source_state , 
                          target_case , target_report_step , target_state , 
                          iens_mask , ranking_key , node_list);
  
}



/**
   This function will select all the parameter variables in the
   ensmeble, and then call enkf_main_initialize_from_existing__() with
   that list.
*/
void enkf_main_initialize_from_existing(enkf_main_type * enkf_main , 
                                        const char * source_case , 
                                        int          source_report_step,
                                        state_enum   source_state,
                                        const bool_vector_type * iens_mask,
                                        const char  * ranking_key) { 
  stringlist_type * param_list = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER ); /* Select only paramters - will fail for GEN_DATA of type DYNAMIC_STATE. */
  enkf_main_initialize_from_existing__(enkf_main , source_case , source_report_step , source_state , iens_mask , ranking_key , param_list );
  stringlist_free( param_list );
}



static void * enkf_main_initialize_from_scratch_mt(void * void_arg) {
  arg_pack_type * arg_pack           = arg_pack_safe_cast( void_arg );
  enkf_main_type * enkf_main         = arg_pack_iget_ptr( arg_pack , 0);
  const stringlist_type * param_list = arg_pack_iget_const_ptr( arg_pack , 1 );
  int iens1                          = arg_pack_iget_int( arg_pack , 2 );
  int iens2                          = arg_pack_iget_int( arg_pack , 3 );
  bool force_init                    = arg_pack_iget_bool( arg_pack , 4 );
  int iens;
  
  for (iens = iens1; iens < iens2; iens++) {
    enkf_state_type * state = enkf_main_iget_state( enkf_main , iens);
    enkf_state_initialize( state , enkf_main_get_fs( enkf_main ) , param_list , force_init);
  }

  return NULL;
}


void enkf_main_initialize_from_scratch(enkf_main_type * enkf_main , const stringlist_type * param_list , int iens1 , int iens2, bool force_init) {
  int num_cpu               = 4;
  thread_pool_type * tp     = thread_pool_alloc( num_cpu , true );
  int ens_sub_size          = (iens2 - iens1 + 1) / num_cpu;
  arg_pack_type ** arg_list = util_calloc( num_cpu , sizeof * arg_list );
  int i;
  
  printf("Initializing .... "); fflush( stdout );
  for (i = 0; i < num_cpu;  i++) {
    arg_list[i] = arg_pack_alloc();
    arg_pack_append_ptr( arg_list[i] , enkf_main );
    arg_pack_append_const_ptr( arg_list[i] , param_list );
    {
      int start_iens = i * ens_sub_size;
      int end_iens   = start_iens + ens_sub_size;
      
      if (i == (num_cpu - 1)){
        end_iens = iens2 + 1;  /* Input is upper limit inclusive. */
        if(ens_sub_size == 0)
          start_iens = iens1;  /* Don't necessarily want to start from zero when ens_sub_size = 0*/
      }
      arg_pack_append_int( arg_list[i] , start_iens );
      arg_pack_append_int( arg_list[i] , end_iens );
    }
    arg_pack_append_bool( arg_list[i] , force_init );
    thread_pool_add_job( tp , enkf_main_initialize_from_scratch_mt , arg_list[i]);
  }
  thread_pool_join( tp );
  for (i = 0; i < num_cpu; i++)
    arg_pack_free( arg_list[i] ); 
  free( arg_list );
  thread_pool_free( tp );
  printf("\n");
}





/**
   This function creates a local_config file corresponding to the
   default 'ALL_ACTIVE' configuration. We eat our own dogshit around
   here...
*/

void enkf_main_create_all_active_config( const enkf_main_type * enkf_main , 
                                         const char * local_config_file ) {


  bool single_node_update = analysis_config_get_single_node_update( enkf_main->analysis_config );
  bool update_results     = analysis_config_get_update_results( enkf_main->analysis_config );

  const char * update_step_name = "ALL_ACTIVE";
  const char * ministep_name    = "ALL_ACTIVE";
  const char * obsset_name      = "ALL_OBS";
  const char * dataset_name     = "ALL_DATA";   // <- This is is created for possible further use, even if 
                                                //    single_node_update is true. 
  
  FILE * stream = util_fopen( local_config_file , "w");

  fprintf(stream , "%-32s %s\n", local_config_get_cmd_string( CREATE_UPDATESTEP ) , update_step_name);
  fprintf(stream , "%-32s %s \n", local_config_get_cmd_string( CREATE_OBSSET ) , obsset_name);
  fprintf(stream , "%-32s %s %s \n", local_config_get_cmd_string( CREATE_MINISTEP ) , ministep_name , obsset_name);
  fprintf(stream , "%-32s %s %s \n" , local_config_get_cmd_string( ATTACH_MINISTEP ), update_step_name , ministep_name);

  fprintf(stream , "%-32s %s \n", local_config_get_cmd_string( CREATE_DATASET ) , dataset_name);
  if (!single_node_update) 
    fprintf(stream , "%-32s %s %s \n", local_config_get_cmd_string( ATTACH_DATASET ) , ministep_name , dataset_name);
  
  /* Adding all observation keys */
  {
    hash_iter_type * obs_iter = enkf_obs_alloc_iter( enkf_main->obs );
    while ( !hash_iter_is_complete(obs_iter) ) {
      const char * obs_key = hash_iter_get_next_key( obs_iter );
      fprintf(stream , "%-32s %s %s\n",local_config_get_cmd_string( ADD_OBS ) , obsset_name , obs_key);
    }
    hash_iter_free( obs_iter );
  }
  
  /* Adding all node which can be updated. */
  {
    stringlist_type * keylist = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER + DYNAMIC_STATE + DYNAMIC_RESULT);
    int i;
    for (i = 0; i < stringlist_get_size( keylist ); i++) {
      const char * key = stringlist_iget( keylist , i);
      const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , key );
      enkf_var_type var_type = enkf_config_node_get_var_type( config_node );
      bool add_node = true;

      if ((var_type == DYNAMIC_RESULT) && (!update_results))
        add_node = false;

      /*
        Make sure the funny GEN_KW instance masquerading as
        SCHEDULE_PREDICTION_FILE is not added to the soup.
      */
      if (util_string_equal(key , "PRED"))
        add_node = false;

      
      if (add_node) {
        if (single_node_update) {
          fprintf(stream , "%-32s %s \n"    , local_config_get_cmd_string( CREATE_DATASET ) , key);
          fprintf(stream , "%-32s %s %s \n" , local_config_get_cmd_string( ATTACH_DATASET ) , ministep_name , key);
          fprintf(stream , "%-32s %s %s\n"  , local_config_get_cmd_string( ADD_DATA ) , key , key);
        } 
        fprintf(stream , "%-32s %s %s\n",local_config_get_cmd_string( ADD_DATA ) , dataset_name , key);
      }
    }
    stringlist_free( keylist);
  }

  /* Install the ALL_ACTIVE step as the default. */
  fprintf(stream , "%-32s ALL_ACTIVE" , local_config_get_cmd_string( INSTALL_DEFAULT_UPDATESTEP ));
  fclose( stream );
}




static void enkf_main_init_user_config( const enkf_main_type * enkf_main , config_type * config ) {
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
  config_add_key_value(config , RERUN_PATH_KEY , false , CONFIG_STRING);
  
  item = config_add_schema_item(config , ENSPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item( config , JOBNAME_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  
  item = config_add_schema_item(config , SELECT_CASE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item(config , DBASE_TYPE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1, 1 );
  config_schema_item_set_common_selection_set(item , 2 , (const char *[2]) {"PLAIN" , "BLOCK_FS"});

  item = config_add_schema_item(config , FORWARD_MODEL_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config , DATA_KW_KEY , false  );
  config_schema_item_set_argc_minmax(item , 2 , 2);

  item = config_add_schema_item(config , KEEP_RUNPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  config_add_key_value(config , PRE_CLEAR_RUNPATH_KEY , false , CONFIG_BOOL);

  item = config_add_schema_item(config , DELETE_RUNPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config , OBS_CONFIG_KEY  , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  item = config_add_schema_item(config , RFT_CONFIG_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  item = config_add_schema_item(config , RFTPATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item(config , LOCAL_CONFIG_KEY  , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  item = config_add_schema_item(config , ENKF_SCHED_FILE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

  item = config_add_schema_item(config , HISTORY_SOURCE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1);
  {
    stringlist_type * refcase_dep = stringlist_alloc_argv_ref( (const char *[1]) { REFCASE_KEY } , 1);

    config_schema_item_set_common_selection_set(item , 3 , (const char *[3]) {"SCHEDULE" , "REFCASE_SIMULATED" , "REFCASE_HISTORY"});
    config_schema_item_set_required_children_on_value(item , "REFCASE_SIMULATED" , refcase_dep);
    config_schema_item_set_required_children_on_value(item , "REFCASE_HISTORY"  , refcase_dep);

    stringlist_free(refcase_dep);
  }
  
  ert_report_list_add_config_items( config);
  qc_module_add_config_items( config );
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


void enkf_main_parse_keep_runpath(enkf_main_type * enkf_main , const char * keep_runpath_string , const char * delete_runpath_string , int ens_size ) {

  int i;
  for (i = 0; i < ens_size; i++)
    int_vector_iset( enkf_main->keep_runpath , i , DEFAULT_KEEP);
  

  {
    int_vector_type * active_list = string_util_alloc_active_list(keep_runpath_string);

    for (i = 0; i < int_vector_size( active_list ); i++)
      int_vector_iset( enkf_main->keep_runpath , int_vector_iget( active_list , i ) , EXPLICIT_KEEP);
    
    int_vector_free( active_list );
  }
  
  
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


static void enkf_main_init_qc( enkf_main_type * enkf_main , config_type * config ) {
  qc_module_init( enkf_main->qc_module , config );
  enkf_main_add_subst_kw( enkf_main , "QC_PATH" , qc_module_get_path( enkf_main->qc_module ) , "QC Root path" , true);
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
  enkf_main->current_fs_case    = NULL;
  enkf_main->dbase              = NULL;
  enkf_main->ensemble           = NULL;
  enkf_main->user_config_file   = NULL;
  enkf_main->site_config_file   = NULL;
  enkf_main->rft_config_file    = NULL;
  enkf_main->local_config       = NULL;
  enkf_main->rng                = NULL; 
  enkf_main->ens_size           = 0;
  enkf_main->keep_runpath       = int_vector_alloc( 0 , DEFAULT_KEEP );
  enkf_main->logh               = log_open( NULL , DEFAULT_LOG_LEVEL );
  enkf_main->rng_config         = rng_config_alloc( );
  enkf_main->site_config        = site_config_alloc_empty();
  enkf_main->ensemble_config    = ensemble_config_alloc_empty();
  enkf_main->ecl_config         = ecl_config_alloc_empty();
  enkf_main->plot_config        = plot_config_alloc_default();                       
  enkf_main->ranking_table      = ranking_table_alloc( 0 );
  enkf_main->obs                = enkf_obs_alloc( );
  enkf_main->model_config       = model_config_alloc_empty( );

  enkf_main_rng_init( enkf_main );
  enkf_main->subst_func_pool    = subst_func_pool_alloc(  );
  enkf_main->subst_list         = subst_list_alloc( enkf_main->subst_func_pool );
  enkf_main->templates          = ert_templates_alloc( enkf_main->subst_list );
  enkf_main->workflow_list      = ert_workflow_list_alloc( enkf_main->subst_list );
  enkf_main->qc_module          = qc_module_alloc( enkf_main->workflow_list , DEFAULT_QC_PATH );
  enkf_main->analysis_config    = analysis_config_alloc_default( enkf_main->rng );   
  enkf_main->report_list        = ert_report_list_alloc( DEFAULT_REPORT_PATH , plot_config_get_path( enkf_main->plot_config ) );
  
  enkf_main_init_subst_list( enkf_main );
  enkf_main_set_verbose( enkf_main , true );
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



  /*
     Installing the based (key,value) pairs which are common to all
     ensemble members, and independent of time.
  */
  {
    char * cwd                    = util_alloc_cwd();
    char * date_string            = util_alloc_date_stamp();
    const char * num_cpu_string   = "1";
  
    enkf_main_add_subst_kw( enkf_main , "CWD"          , cwd , "The current working directory we are running from - the location of the config file." , true);
    enkf_main_add_subst_kw( enkf_main , "CONFIG_PATH"  , cwd , "The current working directory we are running from - the location of the config file." , false);
    enkf_main_add_subst_kw( enkf_main , "DATE"         , date_string , "The current date." , true);
    enkf_main_add_subst_kw( enkf_main , "NUM_CPU"      , num_cpu_string , "The number of CPU used for one forward model." , true );
    enkf_main_add_subst_kw( enkf_main , "RUNPATH_FILE" , qc_module_get_runpath_list_file( enkf_main->qc_module ) , "The name of a file with a list of run directories." , true);
    
    //free( cwd );
    //free( date_string );
  }
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
                                                   enkf_main->dbase ,
                                                   model_config_iget_casename( enkf_main->model_config , iens ) ,
                                                   enkf_main->pre_clear_runpath                                 ,
                                                   int_vector_safe_iget( enkf_main->keep_runpath , iens)        , 
                                                   enkf_main->model_config                                      ,
                                                   enkf_main->ensemble_config                                   ,
                                                   enkf_main->site_config                                       ,
                                                   enkf_main->ecl_config                                        ,
                                                   enkf_main->logh                                              ,
                                                   enkf_main->templates                                         ,
                                                   enkf_main->subst_list);
    enkf_main->ens_size = new_ens_size;
    return;
  }
  
  util_abort("%s: something is seriously broken - should NOT be here .. \n",__func__);
}





void enkf_main_update_node( enkf_main_type * enkf_main , const char * key ) {
  int iens;
  for (iens = 0; iens < enkf_main->ens_size; iens++) 
    enkf_state_update_node( enkf_main->ensemble[iens] , key );
}


// NOTE KF 20130628: This is commented out, because I don't think it is used, but we'll give it some time
//
///**
//   When the case has changed it is essential to invalidate the meta
//   information in the enkf_nodes, otherwise the nodes might reuse old
//   data (from a previous case).
//*/
//
//static void enkf_main_invalidate_cache( enkf_main_type * enkf_main ) {
//  int ens_size = enkf_main_get_ensemble_size( enkf_main );
//  int iens;
//  for (iens = 0; iens < ens_size; iens++)
//    enkf_state_invalidate_cache( enkf_main->ensemble[iens] );
//}


void enkf_main_create_fs( enkf_main_type * enkf_main , const char * fs_path) {
  fs_driver_impl driver_id = model_config_get_dbase_type( enkf_main->model_config );
  void * arg = NULL;
  
  enkf_fs_create_fs( fs_path , driver_id , arg );
}


static void enkf_main_link_current_fs__( enkf_main_type * enkf_main , const char * case_path) {
  const char * ens_path = model_config_get_enspath( enkf_main->model_config);
  
  /* 1: Create a symlink pointing to the currently open case. */
  {
    char * current_link = util_alloc_filename( ens_path , CURRENT_CASE , NULL );
    {
      if (util_entry_exists( current_link ))
        unlink( current_link );
      symlink( case_path , current_link );
    }
    free( current_link );
  }


  /* 2: Update a small text file with the name of the host currently
        running ert, the pid number of the process, the active case
        and when it started. 
        
        If the previous shutdown was unclean the file will be around,
        and we will need the info from the previous invocation which
        is in the file. For that reason we open with mode 'a' instead
        of 'w'.
  */
  {
    int buffer_size = 256;
    char * current_host = util_alloc_filename( ens_path , CASE_LOG , NULL );
    FILE * stream = util_fopen( current_host , "a");
    
    fprintf(stream , "CASE:%-16s  " , case_path ); 
    fprintf(stream , "PID:%-8d  " , getpid());
    {
      char hostname[buffer_size];
      gethostname( hostname , buffer_size );
      fprintf(stream , "HOST:%-16s  " , hostname );
    }

    
    {
      int year,month,day,hour,minute,second;
      time_t now = time( NULL );
      
      util_set_datetime_values( now , &second , &minute , &hour , &day , &month , &year );
      
      fprintf(stream , "TIME:%02d/%02d/%4d-%02d.%02d.%02d\n" , day , month ,  year , hour , minute , second);
    }
    fclose( stream );
    free( current_host );
  }
}




static void enkf_main_close_fs( enkf_main_type * enkf_main ) {
  enkf_fs_close( enkf_main->dbase );
  enkf_main->dbase = NULL;
}


enkf_fs_type * enkf_main_get_fs(const enkf_main_type * enkf_main) {
  return enkf_main->dbase;
}


char * enkf_main_alloc_mount_point( const enkf_main_type * enkf_main , const char * case_path) {
  char * mount_point;
  if (util_is_abs_path( case_path ))
    mount_point = util_alloc_string_copy( case_path );
  else
    mount_point = util_alloc_filename( model_config_get_enspath( enkf_main->model_config) , case_path , NULL);
  return mount_point;
}


void enkf_main_gen_data_special( enkf_main_type * enkf_main ) {
  stringlist_type * gen_data_keys = ensemble_config_alloc_keylist_from_impl_type( enkf_main->ensemble_config , GEN_DATA);
  for (int i=0; i < stringlist_get_size( gen_data_keys ); i++) {
    enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , stringlist_iget( gen_data_keys , i));
    enkf_var_type var_type = enkf_config_node_get_var_type(config_node);
    if ((var_type == DYNAMIC_STATE) || (var_type == DYNAMIC_RESULT)) {
      gen_data_config_type * gen_data_config = enkf_config_node_get_ref( config_node );
      gen_data_config_set_dynamic( gen_data_config , enkf_main->dbase );
      gen_data_config_set_ens_size( gen_data_config , enkf_main->ens_size );
    }
  }
  stringlist_free( gen_data_keys );
}


void enkf_main_set_fs( enkf_main_type * enkf_main , enkf_fs_type * fs , const char * case_path ) {
  if (enkf_main->dbase != fs) {
    if (enkf_main->dbase != NULL)
      enkf_main_close_fs( enkf_main );
    enkf_main->dbase = fs;

    enkf_main_link_current_fs__( enkf_main , case_path);
    enkf_main->current_fs_case = util_realloc_string_copy( enkf_main->current_fs_case , case_path);
    enkf_main_gen_data_special( enkf_main );
    enkf_main_add_subst_kw( enkf_main , "ERT-CASE" , enkf_main->current_fs_case , "Current case" , true );
    enkf_main_add_subst_kw( enkf_main , "ERTCASE"  , enkf_main->current_fs_case , "Current case" , true );
  }
}


stringlist_type * enkf_main_alloc_caselist( const enkf_main_type * enkf_main ) {
  stringlist_type * case_list = stringlist_alloc_new( );
  {
    const char * ens_path = model_config_get_enspath( enkf_main->model_config );
    DIR * ens_dir = opendir( ens_path );
    if (ens_dir != NULL) {
      int ens_fd = dirfd( ens_dir );
      if (ens_fd != -1) {
        struct dirent * dp;
        do {
          dp = readdir( ens_dir );
          if (dp != NULL) {
            if (!(util_string_equal( dp->d_name , ".") || util_string_equal(dp->d_name , ".."))) {
              if (!util_string_equal( dp->d_name , CURRENT_CASE)) {
                char * full_path = util_alloc_filename( ens_path , dp->d_name , NULL);
                if (util_is_directory( full_path ))
                  stringlist_append_copy( case_list , dp->d_name );
                free( full_path);
              }
            }
          }
        } while (dp != NULL);
      }
    }
    closedir( ens_dir );
  }
  return case_list;
}


void enkf_main_close_alt_fs(enkf_main_type * enkf_main , enkf_fs_type * fs) {
  if (fs != enkf_main->dbase) 
    enkf_fs_close( fs );
}


enkf_fs_type * enkf_main_get_alt_fs(enkf_main_type * enkf_main , const char * case_path , bool read_only , bool create) {
  enkf_fs_type * alt_fs = enkf_main->dbase;
  if (case_path != NULL) {
    char * new_mount_point    = enkf_main_alloc_mount_point( enkf_main , case_path );
    const char * current_mount_point = NULL;
    
    if (enkf_main->dbase != NULL)
      current_mount_point = enkf_fs_get_mount_point( enkf_main->dbase );

    if (!util_string_equal( new_mount_point , current_mount_point )) {
      if (!enkf_fs_exists( new_mount_point )) {
        if (create)
          enkf_fs_create_fs( new_mount_point,
                             model_config_get_dbase_type( enkf_main->model_config ) , 
                             model_config_get_dbase_args( enkf_main->model_config ));
      }
      alt_fs = enkf_fs_open( new_mount_point , read_only );
    }
    free( new_mount_point );
  }
  return alt_fs;
}



void enkf_main_select_fs( enkf_main_type * enkf_main , const char * case_path ) {
  enkf_fs_type * new_fs = enkf_main_get_alt_fs( enkf_main , case_path , false , true );
  
  if (new_fs != NULL) 
    enkf_main_set_fs( enkf_main , new_fs , case_path);
  else {
    const char * ens_path = model_config_get_enspath( enkf_main->model_config );
    util_exit("%s: select filesystem %s:%s failed \n",__func__ , ens_path , case_path );
  }
}


const char * enkf_main_get_current_fs( const enkf_main_type * enkf_main ) {
  return enkf_main->current_fs_case;
}

bool enkf_main_fs_exists(const enkf_main_type * enkf_main, const char * input_case){
  bool exists = false;
  char * new_mount_point = enkf_main_alloc_mount_point( enkf_main , input_case);
  if(enkf_fs_exists( new_mount_point )) 
    exists = true;

  free( new_mount_point );
  return exists;
}


void enkf_main_user_select_fs(enkf_main_type * enkf_main , const char * input_case ) {
  const char * ens_path = model_config_get_enspath( enkf_main->model_config);      
  int root_version = enkf_fs_get_version104( ens_path );
  if (root_version == -1 || root_version == 105) {
    if (input_case == NULL) {
      char * current_mount_point = util_alloc_filename( ens_path , CURRENT_CASE , NULL);
      
      if (enkf_fs_exists( current_mount_point )) {
        // We will mount the case under the 'current' link; we use
        // readlink to get hold of the actual target before calling the 
        // enkf_main_select_fs() function
        char * target_case = util_alloc_atlink_target( ens_path , CURRENT_CASE );
        enkf_main_select_fs( enkf_main , target_case );  // Selecting (a new) default case
        free( target_case );
      } else 
        enkf_main_select_fs( enkf_main , DEFAULT_CASE );  // Selecting (a new) default case

      free( current_mount_point );
    } else
      enkf_main_select_fs( enkf_main , input_case );
  }  else {
    fprintf(stderr,"Sorry: the filesystem located in %s must be upgraded before the current ERT version can read it.\n" , ens_path);
    exit(1);
  }
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


/*
   Adding inverse observation keys to the enkf_nodes; can be called
   several times.
*/


void enkf_main_update_obs_keys( enkf_main_type * enkf_main ) {
  /* First clear all existing observation keys. */
  ensemble_config_clear_obs_keys( enkf_main->ensemble_config );

  /* Add new observation keys. */
  {
    hash_type      * map  = enkf_obs_alloc_data_map(enkf_main->obs);
    hash_iter_type * iter = hash_iter_alloc(map);
    const char * obs_key  = hash_iter_get_next_key(iter);
    while (obs_key  != NULL) {
      const char * state_kw = hash_get(map , obs_key);
      ensemble_config_add_obs_key(enkf_main->ensemble_config , state_kw , obs_key);
      obs_key = hash_iter_get_next_key(iter);
    }
    hash_iter_free(iter);
    hash_free(map);
  }
}

/*****************************************************************/


void enkf_main_set_log_file( enkf_main_type * enkf_main , const char * log_file ) {
  log_reopen( enkf_main->logh , log_file);
}


const char * enkf_main_get_log_file( const enkf_main_type * enkf_main ) {
  return log_get_filename( enkf_main->logh );
}


void enkf_main_set_log_level( enkf_main_type * enkf_main , int log_level ) {
  log_set_level( enkf_main->logh , log_level);
}


int enkf_main_get_log_level( const enkf_main_type * enkf_main ) {
  return log_get_level( enkf_main->logh );
}


static void enkf_main_init_log( enkf_main_type * enkf_main , const config_type * config ) {
  if (config_item_set( config , LOG_LEVEL_KEY))
    enkf_main_set_log_level( enkf_main , config_get_value_as_int(config , LOG_LEVEL_KEY));
  
  if (config_item_set( config , LOG_FILE_KEY))
    enkf_main_set_log_file( enkf_main , config_get_value(config , LOG_FILE_KEY));
  else {
    char * log_file = util_alloc_filename(NULL , enkf_main->user_config_file , DEFAULT_LOG_FILE);
    enkf_main_set_log_file( enkf_main , log_file );
    free( log_file );
  }
  
  if (enkf_main->verbose)
    printf("Activity will be logged to ..............: %s \n",log_get_filename( enkf_main->logh ));
  log_add_message(enkf_main->logh , 1 , NULL , "ert configuration loaded" , false);
}

static void enkf_main_init_data_kw( enkf_main_type * enkf_main , config_type * config ) {
  config_content_item_type * data_item = config_get_content_item( config , DATA_KW_KEY );
  hash_type      * data_kw = NULL;
  if (data_item) 
    data_kw = config_content_item_alloc_hash(data_item , true);

  enkf_main_install_data_kw( enkf_main , data_kw );
  
  if (data_kw)
    hash_free( data_kw );
}

    



/*****************************************************************/


rng_config_type * enkf_main_get_rng_config( const enkf_main_type * enkf_main ) {
  return enkf_main->rng_config;
}

void enkf_main_rng_init( enkf_main_type * enkf_main) {
  if (enkf_main->rng != NULL) {
    rng_free( enkf_main->rng );
    enkf_main->rng = NULL;
  }
  enkf_main->rng = rng_config_alloc_rng( enkf_main->rng_config );
}


void enkf_main_init_local_updates( enkf_main_type * enkf_main , const config_type * config ) {
  if (model_config_has_history( enkf_main->model_config )) {
    enkf_main->local_config  = local_config_alloc( );
    
    /* First create the default ALL_ACTIVE configuration. */
    {
      char * all_active_config_file = util_alloc_tmp_file("/tmp" , "enkf_local_config" , true);
      enkf_main_create_all_active_config( enkf_main , 
                                          all_active_config_file );
      
      /* Install custom local_config - if present.*/
      {
        int i;
        for (i = 0; i < config_get_occurences( config , LOCAL_CONFIG_KEY); i++) {
          const stringlist_type * files = config_iget_stringlist_ref(config , LOCAL_CONFIG_KEY , i);
          for (int j=0; j < stringlist_get_size( files ); j++)
            local_config_add_config_file( enkf_main->local_config , stringlist_iget( files , j) );
        }
      }
      
      /**
         This is where the local configuration files are actually parsed. 
      */
      local_config_reload( enkf_main->local_config , 
                           ecl_config_get_grid( enkf_main->ecl_config ), 
                           enkf_main->ensemble_config , 
                           enkf_main->obs , 
                           all_active_config_file );
      
      unlink( all_active_config_file );
      free(all_active_config_file);
    }
  } else 
    if (config_get_occurences( config , LOCAL_CONFIG_KEY) > 0) 
      fprintf(stderr,"** Warning: Not possible to configure local analysis without SCHEDULE or REFCASE - %s keyword(s) ignored\n", LOCAL_CONFIG_KEY);
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
    config_type * config = config_alloc();
    {
      site_config_add_config_items( config , true );
      if (config_parse(config , site_config_file  , "--" , INCLUDE_KEY , DEFINE_KEY , CONFIG_UNRECOGNIZED_WARN , false)) {
        site_config_init( enkf_main->site_config , config );
        ert_report_list_site_init( enkf_main->report_list , config );
        ert_workflow_list_init( enkf_main->workflow_list , config , enkf_main->logh);
      } else {
        fprintf(stderr , "** ERROR: Parsing site configuration file:%s failed \n\n" , site_config_file);
        config_fprintf_errors( config , true , stderr );
        exit(1);
      }
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



enkf_main_type * enkf_main_bootstrap(const char * _site_config, const char * _model_config, bool strict , bool verbose) {
  const char     * site_config  = getenv("ERT_SITE_CONFIG");
  char           * model_config;
  enkf_main_type * enkf_main;    /* The enkf_main object is allocated when the config parsing is completed. */

  if (site_config == NULL)
    site_config = _site_config;

  if (site_config == NULL)
    fprintf(stderr,"**WARNING** main enkf_config file is not set. Use environment variable \"ERT_SITE_CONFIG\" - or recompile.\n");
  
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
      if (chdir(path) != 0)
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


  if (!util_file_exists(model_config)) util_exit("%s: can not locate user configuration file:%s \n",__func__ , model_config);
  {
    config_type * config;
    enkf_main            = enkf_main_alloc_empty( );
    enkf_main_set_verbose( enkf_main , verbose );
    enkf_main_bootstrap_site( enkf_main , site_config);
    
    config = config_alloc();
    enkf_main_init_user_config( enkf_main , config );
    site_config_add_config_items( config , false );
    site_config_init_user_mode( enkf_main->site_config );
    
    if (!config_parse(config , model_config , "--" , INCLUDE_KEY , DEFINE_KEY , CONFIG_UNRECOGNIZED_WARN , true)) {
      config_fprintf_errors( config , true , stderr );
      exit(1);
    }

    site_config_init( enkf_main->site_config , config );                                   /*  <---- model_config : second pass. */ 

    /*****************************************************************/
    /* 
       OK - now we have parsed everything - and we are ready to start
       populating the enkf_main object.
    */


    enkf_main_set_site_config_file( enkf_main , site_config );
    enkf_main_set_user_config_file( enkf_main , model_config );
    enkf_main_init_log( enkf_main , config );
    /*
      Initializing the various 'large' sub config objects. 
    */
    rng_config_init( enkf_main->rng_config , config );
    enkf_main_rng_init( enkf_main );  /* Must be called before the ensmeble is created. */
    enkf_main_init_subst_list( enkf_main );
    ert_workflow_list_init( enkf_main->workflow_list , config , enkf_main->logh );
    enkf_main_init_data_kw( enkf_main , config );
    
    analysis_config_load_internal_modules( enkf_main->analysis_config );
    analysis_config_init( enkf_main->analysis_config , config );
    ecl_config_init( enkf_main->ecl_config , config );
    plot_config_init( enkf_main->plot_config , config );
    ensemble_config_init( enkf_main->ensemble_config , config , ecl_config_get_grid( enkf_main->ecl_config ) , ecl_config_get_refcase( enkf_main->ecl_config) );

    model_config_init( enkf_main->model_config , 
                       config , 
                       enkf_main_get_ensemble_size( enkf_main ),
                       site_config_get_installed_jobs(enkf_main->site_config) ,
                       ecl_config_get_last_history_restart( enkf_main->ecl_config ),
                       ecl_config_get_sched_file(enkf_main->ecl_config) ,
                       ecl_config_get_refcase( enkf_main->ecl_config ));

    enkf_main_init_qc( enkf_main , config );
    enkf_main_update_num_cpu( enkf_main );
    {
      const config_content_item_type * pred_item = config_get_content_item( config , SCHEDULE_PREDICTION_FILE_KEY );
      if (pred_item != NULL) {
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
       To keep or not to keep the runpath directories? The problem is
       that the default behavior is different depending on the run_mode:

       enkf_mode: In this case the default behaviour is to delete the
       runpath directories. You can explicitly say that you want to
       keep runpath directories with the KEEP_RUNPATH
       directive.

       experiments: In this case the default is to keep the runpath
       directories around, but you can explicitly say that you
       want to remove the directories by using the DELETE_RUNPATH
       option.

       The final decision is performed in enkf_state().
    */
    {
      {
        char * keep_runpath_string   = NULL;
        char * delete_runpath_string = NULL;
        int    ens_size              = config_get_value_as_int(config , NUM_REALIZATIONS_KEY);
        
        if (config_item_set(config , KEEP_RUNPATH_KEY))
          keep_runpath_string = config_alloc_joined_string(config , KEEP_RUNPATH_KEY , "");

        if (config_item_set(config , DELETE_RUNPATH_KEY))
          delete_runpath_string = config_alloc_joined_string(config , DELETE_RUNPATH_KEY , "");

        enkf_main_parse_keep_runpath( enkf_main , keep_runpath_string , delete_runpath_string , ens_size );

        util_safe_free( keep_runpath_string   );
        util_safe_free( delete_runpath_string );
      }

      /* This is really in the wrong place ... */
      {
        enkf_main->pre_clear_runpath = DEFAULT_PRE_CLEAR_RUNPATH;
        if (config_item_set(config , PRE_CLEAR_RUNPATH_KEY))
          enkf_main->pre_clear_runpath = config_get_value_as_bool( config , PRE_CLEAR_RUNPATH_KEY);
      }


      ecl_config_static_kw_init( enkf_main->ecl_config , config );
      
      /* Installing templates */
      {
        ert_templates_init( enkf_main->templates , config );
      }

      /*****************************************************************/
      ert_report_list_init( enkf_main->report_list , config , ecl_config_get_refcase( enkf_main->ecl_config ));
      
      {
        const char * obs_config_file;
        if (config_item_set(config , OBS_CONFIG_KEY))
          obs_config_file = config_iget(config  , OBS_CONFIG_KEY , 0,0);
        else
          obs_config_file = NULL;

        enkf_main_load_obs( enkf_main , obs_config_file );
      }

      enkf_main_update_obs_keys(enkf_main);

      {
        const char * rft_config_file = NULL;
        if (config_item_set(config , RFT_CONFIG_KEY))
          rft_config_file = config_iget(config , RFT_CONFIG_KEY , 0,0);

        enkf_main_set_rft_config_file( enkf_main , rft_config_file ); 
      }
      

      /*****************************************************************/
      {
        const char * select_case = NULL;
        if (config_item_set( config , SELECT_CASE_KEY))
          select_case = config_get_value( config , SELECT_CASE_KEY );
        
        enkf_main_user_select_fs( enkf_main , select_case );
      }

      /* Adding ensemble members */
      enkf_main_resize_ensemble( enkf_main  , config_iget_as_int(config , NUM_REALIZATIONS_KEY , 0 , 0) );

      /*****************************************************************/
      /*
         Installing the local_config object. Observe that the
         ALL_ACTIVE local_config configuration is ALWAYS loaded. But
         if you have created a personal local config that will be
         loaded on top.
      */
      enkf_main_init_local_updates(enkf_main , config );
      
    }
    config_free(config);
  }
  enkf_main_init_jobname( enkf_main );
  enkf_main_gen_data_special( enkf_main );
  free( model_config );

  return enkf_main;
}




/**
   This function creates a minimal configuration file, with a few
   parameters (a bit arbitrary) parameters read from (typically) a GUI
   configuration dialog.

   The set of parameters written by this function is _NOT_ a minimum
   set to generate a valid configuration.
*/

void enkf_main_create_new_config( const char * config_file , const char * storage_path , const char * case_name , const char * dbase_type , int num_realizations) {
  
  FILE * stream = util_mkdir_fopen( config_file , "w" );
  
  fprintf(stream , CONFIG_KEY_FORMAT      , ENSPATH_KEY);
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , storage_path );

  fprintf(stream , CONFIG_KEY_FORMAT      , SELECT_CASE_KEY);
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , case_name);

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

   For performance reason model_config contains two bool vectors
   __load_state and __load_result; if they are true the state and
   summary are loaded from disk, otherwise no loading is
   performed. This implies that if we do not want to internalize the
   full state but for instance the pressure (i.e. for an RFT) we must
   set the __load_state variable for the actual report step to
   true. For this reason calls enkf_config_node_internalize() must be
   accompanied by calls to model_config_set_load_state|results() -
   this is ensured when using this function to manipulate the
   configuration of internalization.

*/


void enkf_main_init_internalization( enkf_main_type * enkf_main , run_mode_type run_mode ) {
  /* Clearing old internalize flags. */
  model_config_init_internalization( enkf_main->model_config );
  ensemble_config_init_internalization( enkf_main->ensemble_config );

  /* Internalizing the initial state. */
  model_config_set_internalize_state( enkf_main->model_config , 0);

  /* We internalize all the endpoints in the enkf_sched. */
  if (run_mode == ENKF_ASSIMILATION) {
    int inode;
    enkf_sched_type * enkf_sched = model_config_get_enkf_sched(enkf_main->model_config);
    for (inode = 0; inode < enkf_sched_get_num_nodes( enkf_sched ); inode++) {
      const enkf_sched_node_type * node = enkf_sched_iget_node(enkf_sched , inode);
      int report_step2            = enkf_sched_node_get_last_step( node );
      model_config_set_internalize_state( enkf_main->model_config , report_step2);
    }
  }


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




/* Used by external application - this is a library ... */
void  enkf_main_list_users(  set_type * users , const char * executable ) {
  DIR * dir = opendir( DEFAULT_VAR_DIR );
  if (dir != NULL) {
    struct dirent * dp;
    do {
      dp = readdir(dir);
      if (dp != NULL) {
        int pid;
        if (util_sscanf_int( dp->d_name , &pid )) {
          char * full_path = util_alloc_filename( DEFAULT_VAR_DIR , dp->d_name , NULL );
          bool add_user    = false;
          int  uid;

          {
            FILE * stream    = util_fopen( full_path , "r");
            char this_executable[512];

            if (fscanf( stream , "%s %d" , this_executable , &uid) == 2) {
              if (executable != NULL) {
                if (util_string_equal( this_executable , executable ))
                  add_user   = true;
              } else
                add_user = true;
            }
            fclose( stream );
          }


          /* Remove the pid files of dead processes. */
          if (!util_proc_alive( pid )) {
            unlink( full_path );
            add_user = false;
          }


          if (add_user) {
            struct passwd *pwd;
            pwd = getpwuid( uid );
            if (pwd != NULL)
              set_add_key( users , pwd->pw_name );
          }


          free( full_path );
        }
      }
    } while (dp != NULL );
    closedir( dir );
  }
}


const ext_joblist_type * enkf_main_get_installed_jobs( const enkf_main_type * enkf_main ) {
  return site_config_get_installed_jobs( enkf_main->site_config );
}


enkf_plot_data_type * enkf_main_alloc_plot_data( enkf_main_type * enkf_main ) {
  enkf_plot_data_type * plot_data = enkf_plot_data_alloc( ecl_config_get_start_date( enkf_main_get_ecl_config( enkf_main )));
  
  return plot_data;
}

/*****************************************************************/

void enkf_main_get_observations( const enkf_main_type * enkf_main, const char * user_key , int obs_count , time_t * obs_time , double * y , double * std) {
  ensemble_config_get_observations( enkf_main->ensemble_config , enkf_main->obs , user_key , obs_count , obs_time , y , std);
}


int enkf_main_get_observation_count( const enkf_main_type * enkf_main, const char * user_key ) {
  return ensemble_config_get_observations( enkf_main->ensemble_config , enkf_main->obs , user_key , 0 , NULL , NULL , NULL);
}



/**
   This function will go through the filesystem and check that we have
   initial data for all parameters and all realizations. If the second
   argument mask is different from NULL, the function will only
   consider the realizations for which mask is true (if mask == NULL
   all realizations will be checked).
*/

bool enkf_main_is_initialized( const enkf_main_type * enkf_main , bool_vector_type * __mask) {
  stringlist_type  * parameter_keys = ensemble_config_alloc_keylist_from_var_type( enkf_main->ensemble_config , PARAMETER );
  bool_vector_type * mask;
  bool initialized = true;
  int ikey = 0;
  if (__mask != NULL)
    mask = __mask;
  else
    mask = bool_vector_alloc(0 , true );
  
  do {
    const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_main->ensemble_config , stringlist_iget( parameter_keys , ikey) );
    int iens = 0;
    do {
      if (bool_vector_safe_iget( mask , iens)) {
        node_id_type node_id = {.report_step = 0 , .iens = iens , .state = ANALYZED };
        initialized = enkf_config_node_has_node( config_node , enkf_main->dbase , node_id);
      }
      iens++;
    } while ((iens < enkf_main->ens_size) && (initialized));
    ikey++;
  } while ((ikey < stringlist_get_size( parameter_keys )) && (initialized));
  
  stringlist_free( parameter_keys );
  if (__mask == NULL)
    bool_vector_free( mask );
  return initialized;
}




void enkf_main_log_fprintf_config( const enkf_main_type * enkf_main , FILE * stream ) {
  fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
  fprintf( stream , CONFIG_COMMENT_FORMAT  , "Here comes configuration information about the ERT logging.");
  fprintf( stream , CONFIG_KEY_FORMAT      , LOG_FILE_KEY );
  fprintf( stream , CONFIG_ENDVALUE_FORMAT , enkf_main_get_log_file( enkf_main ));

  if (enkf_main_get_log_level( enkf_main ) != DEFAULT_LOG_LEVEL) {
    fprintf(stream , CONFIG_KEY_FORMAT      , LOG_LEVEL_KEY );
    fprintf(stream , CONFIG_INT_FORMAT , enkf_main_get_log_level( enkf_main ));
    fprintf(stream , "\n");
  }
  
  fprintf(stream , "\n");
  fprintf(stream , "\n");
}


void enkf_main_install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. 
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
  signal(SIGABRT , util_abort_signal);    /* Signal abort. */ 
}




ert_templates_type * enkf_main_get_templates( enkf_main_type * enkf_main ) {
  return enkf_main->templates;
}

void enkf_main_set_case_table( enkf_main_type * enkf_main , const char * case_table_file ) {
  model_config_set_case_table( enkf_main->model_config , enkf_main->ens_size , case_table_file );
}


ert_report_list_type * enkf_main_get_report_list( const enkf_main_type * enkf_main ) {
  return enkf_main->report_list;
}


/*****************************************************************/


void enkf_main_fprintf_runpath_config( const enkf_main_type * enkf_main , FILE * stream ) {
  fprintf(stream , CONFIG_KEY_FORMAT      , PRE_CLEAR_RUNPATH_KEY );
  fprintf(stream , CONFIG_ENDVALUE_FORMAT , CONFIG_BOOL_STRING( enkf_state_get_pre_clear_runpath( enkf_main->ensemble[0] )));
  
  {
    bool keep_comma = false;
    bool del_comma  = false;
    
    
    for (int iens = 0; iens < enkf_main->ens_size; iens++) {
      keep_runpath_type keep_runpath = enkf_main_iget_keep_runpath( enkf_main , iens );
      if (keep_runpath == EXPLICIT_KEEP) {
        if (!keep_comma) {
          fprintf(stream , CONFIG_KEY_FORMAT , KEEP_RUNPATH_KEY );
          fprintf(stream , "%d" , iens);
          keep_comma = true;
        } else 
          fprintf(stream , ",%d" , iens);
      }
    }
    fprintf(stream , "\n");


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




void enkf_main_fprintf_config( const enkf_main_type * enkf_main ) {
  if (util_file_exists( enkf_main->user_config_file)) {
    /** 
        A version of the config file already exist, and we will take
        backup. 
    */
    char * backup_file = NULL;
    char * prev_backup = NULL;
    int backup_nr      = 1;
    do {
      backup_file = util_realloc_sprintf( backup_file , "%s.%d" , enkf_main->user_config_file , backup_nr);
      if (util_file_exists( backup_file )) {
        prev_backup = util_realloc_string_copy( prev_backup , backup_file );
        backup_nr++;
      }
    } while (util_file_exists( backup_file ));
    
    /**
       When leaving the do { } while loop backup_file will point to
       the first non-existing backup filename; and prev_backup will
       point to the last existing (or be NULL if there was no existing
       backup file).

       1. If prev_backup == NULL there was no previous backup file,
          and we just backup the current file to backup_file and be
          done with it.

       2. If prev_backup != NULL we do the following: The latest
          backup is compared to the current config file, if they are
          equal no new backup is taken; otherwise a new backup is
          stored.

    */
    if (prev_backup == NULL)
      util_copy_file( enkf_main->user_config_file , backup_file );
    else {
      if (!util_files_equal( enkf_main->user_config_file , prev_backup )) 
        util_copy_file( enkf_main->user_config_file , backup_file );
    }
    util_safe_free( prev_backup );
    util_safe_free( backup_file );
  }
  
  /* Start the proper saving */
  {
    FILE * stream = util_fopen( enkf_main->user_config_file , "w");
    
    ecl_config_fprintf_config( enkf_main->ecl_config , stream );
    model_config_fprintf_config( enkf_main->model_config , enkf_main->ens_size , stream );

    enkf_obs_fprintf_config( enkf_main->obs , stream );
    analysis_config_fprintf_config( enkf_main->analysis_config , stream );
    ensemble_config_fprintf_config( enkf_main->ensemble_config , stream );
    local_config_fprintf_config( enkf_main->local_config , stream );
    enkf_main_fprintf_runpath_config( enkf_main , stream );
    ert_templates_fprintf_config( enkf_main->templates , stream );
    enkf_main_log_fprintf_config( enkf_main , stream );
    site_config_fprintf_config( enkf_main->site_config , stream );    
    rng_config_fprintf_config( enkf_main->rng_config , stream );
    fclose( stream );
  }
}


/*****************************************************************/

ert_workflow_list_type * enkf_main_get_workflow_list( enkf_main_type * enkf_main ) {
  return enkf_main->workflow_list;
}

bool enkf_main_run_workflow( enkf_main_type * enkf_main , const char * workflow ) {
  ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
  if (ert_workflow_list_has_workflow( workflow_list , workflow)) 
    return ert_workflow_list_run_workflow( workflow_list , workflow , enkf_main);
  else
    return false;
}


void enkf_main_run_workflows( enkf_main_type * enkf_main , const stringlist_type * workflows) {
  int iw;
  for (iw = 0; iw < stringlist_get_size( workflows ); iw++) 
    enkf_main_run_workflow( enkf_main , stringlist_iget( workflows , iw ));
}


