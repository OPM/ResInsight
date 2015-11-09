/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'enkf_state.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

#include <ert/util/path_fmt.h>
#include <ert/util/thread_pool.h>
#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/arg_pack.h>
#include <ert/util/stringlist.h>
#include <ert/util/node_ctype.h>
#include <ert/util/subst_list.h>
#include <ert/util/timer.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/rng.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_io_config.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_endian_flip.h>

#include <ert/sched/sched_file.h>

#include <ert/job_queue/forward_model.h>
#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/ext_joblist.h>

#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/ecl_static_kw.h>
#include <ert/enkf/field.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/gen_kw.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/model_config.h>
#include <ert/enkf/site_config.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/ert_template.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/state_map.h>
#include <ert/enkf/ert_log.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/summary_key_matcher.h>

#define  ENKF_STATE_TYPE_ID 78132





/**
   This struct contains various objects which the enkf_state needs
   during operation, which the enkf_state_object *DOES NOT* own. The
   struct only contains pointers to objects owned by (typically) the
   enkf_main object.

   If the enkf_state object writes to any of the objects in this
   struct that can be considered a serious *BUG*.

   The elements in this struct should not change during the
   application lifetime?
*/

typedef struct shared_info_struct {
  model_config_type           * model_config;      /* .... */
  ext_joblist_type            * joblist;           /* The list of external jobs which are installed - and *how* they should be run (with Python code) */
  job_queue_type              * job_queue;         /* The queue handling external jobs. (i.e. LSF / TORQUE / rsh / local / ... )*/
  const site_config_type      * site_config;
  ert_templates_type          * templates;
  const ecl_config_type       * ecl_config;
} shared_info_type;






/*****************************************************************/

struct enkf_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  stringlist_type       * restart_kw_list;
  hash_type             * node_hash;
  subst_list_type       * subst_list;              /* This a list of key - value pairs which are used in a search-replace
                                                      operation on the ECLIPSE data file. Will at least contain the key INIT"
                                                      - which will describe initialization of ECLIPSE (EQUIL or RESTART).*/
  ensemble_config_type  * ensemble_config;         /* The config nodes for the enkf_node objects contained in node_hash. */

  shared_info_type      * shared_info;             /* Pointers to shared objects which is needed by the enkf_state object (read only). */
  member_config_type    * my_config;               /* Private config information for this member; not updated during a simulation. */
  rng_type              * rng;
};

/*****************************************************************/


static UTIL_SAFE_CAST_FUNCTION( enkf_state , ENKF_STATE_TYPE_ID )


static shared_info_type * shared_info_alloc(const site_config_type * site_config , model_config_type * model_config, const ecl_config_type * ecl_config , ert_templates_type * templates) {
  shared_info_type * shared_info = util_malloc(sizeof * shared_info );

  shared_info->joblist      = site_config_get_installed_jobs( site_config );
  shared_info->job_queue    = site_config_get_job_queue( site_config );
  shared_info->site_config  = site_config;
  shared_info->model_config = model_config;
  shared_info->templates    = templates;
  shared_info->ecl_config   = ecl_config;
  return shared_info;
}


static void shared_info_free(shared_info_type * shared_info) {
  /**
      Adding something here is a BUG - this object does
      not own anything.
  */
  free( shared_info );
}





/*****************************************************************/
/** Helper classes complete - starting on the enkf_state proper object. */
/*****************************************************************/

void enkf_state_initialize(enkf_state_type * enkf_state , enkf_fs_type * fs , const stringlist_type * param_list, init_mode_type init_mode) {
  if (init_mode != INIT_NONE) {
    int iens = enkf_state_get_iens( enkf_state );
    state_map_type * state_map = enkf_fs_get_state_map( fs );
    realisation_state_enum current_state = state_map_iget(state_map, iens);
    if ((current_state == STATE_PARENT_FAILURE) && (init_mode != INIT_FORCE))
      return;
    else {
      state_enum init_state = ANALYZED;

      for (int ip = 0; ip < stringlist_get_size(param_list); ip++) {
        enkf_node_type * param_node = enkf_state_get_node(enkf_state, stringlist_iget(param_list, ip));
        node_id_type node_id = { .report_step = 0, .iens = iens, .state = init_state };
        bool has_data = enkf_node_has_data(param_node, fs, node_id);

        if ((init_mode == INIT_FORCE) || (has_data == false) || (current_state == STATE_LOAD_FAILURE)) {
          if (enkf_node_initialize(param_node, iens, enkf_state->rng))
            enkf_node_store(param_node, fs, true, node_id);
        }
      }
      state_map_update_matching(state_map , iens , STATE_UNDEFINED | STATE_LOAD_FAILURE , STATE_INITIALIZED);
      enkf_fs_fsync(fs);
    }
  }
}







/*
  void enkf_state_set_iens(enkf_state_type * enkf_state , int iens) {
  enkf_state->my_iens = iens;
  }
*/

int enkf_state_get_iens(const enkf_state_type * enkf_state) {
  return member_config_get_iens( enkf_state->my_config );
}

member_config_type * enkf_state_get_member_config(const enkf_state_type * enkf_state) {
  return enkf_state->my_config;
}




void enkf_state_add_subst_kw(enkf_state_type * enkf_state , const char * kw , const char * value , const char * doc_string) {
  char * tagged_key = util_alloc_sprintf( INTERNAL_DATA_KW_TAG_FORMAT , kw );
  subst_list_append_owned_ref(enkf_state->subst_list , tagged_key , util_alloc_string_copy(value) , doc_string);
  free(tagged_key);
}





/**
   This function must be called each time the eclbase_fmt has been
   updated.
*/

void enkf_state_update_eclbase( enkf_state_type * enkf_state ) {
  const char * eclbase  = member_config_update_eclbase( enkf_state->my_config , enkf_state->shared_info->ecl_config , enkf_state->subst_list);
  const char * casename = member_config_get_casename( enkf_state->my_config ); /* Mostly NULL */
  {
    enkf_state_add_subst_kw(enkf_state , "ECL_BASE"    , eclbase , NULL);
    enkf_state_add_subst_kw(enkf_state , "ECLBASE"     , eclbase , NULL);

    if (casename == NULL)
      enkf_state_add_subst_kw( enkf_state , "CASE" , eclbase , NULL);      /* No CASE_TABLE loaded - using the eclbase as default. */
    else
      enkf_state_add_subst_kw( enkf_state , "CASE" , casename , NULL);
  }
}


void enkf_state_update_jobname( enkf_state_type * enkf_state ) {
  member_config_update_jobname( enkf_state->my_config ,
                                model_config_get_jobname_fmt( enkf_state->shared_info->model_config ) ,
                                enkf_state->subst_list);
}


/**
   Sets all the static subst keywords which will not change during the simulation.
*/
static void enkf_state_set_static_subst_kw(enkf_state_type * enkf_state) {

  {
    int    iens        = member_config_get_iens( enkf_state->my_config );
    char * iens_s      = util_alloc_sprintf("%d"   , iens);
    char * iens4_s     = util_alloc_sprintf("%04d" , iens);
    char * iensp1_s    = util_alloc_sprintf("%d"   , iens + 1);

    enkf_state_add_subst_kw(enkf_state , "IENS"        , iens_s      , NULL);
    enkf_state_add_subst_kw(enkf_state , "IENSP1"      , iensp1_s    , NULL);
    enkf_state_add_subst_kw(enkf_state , "IENS4"       , iens4_s     , NULL);

    free(iensp1_s);
    free(iens_s);
    free(iens4_s);
  }
  enkf_state_update_eclbase( enkf_state );
}


static void enkf_state_add_nodes( enkf_state_type * enkf_state, const ensemble_config_type * ensemble_config) {
  stringlist_type * container_keys = stringlist_alloc_new();
  stringlist_type * keylist  = ensemble_config_alloc_keylist(ensemble_config);
  int keys        = stringlist_get_size(keylist);

  // 1: Add all regular nodes
  for (int ik = 0; ik < keys; ik++) {
    const char * key = stringlist_iget(keylist, ik);
    const enkf_config_node_type * config_node = ensemble_config_get_node(ensemble_config , key);
    if (enkf_config_node_get_impl_type( config_node ) == CONTAINER) {
      stringlist_append_ref( container_keys , key );
    } else
      enkf_state_add_node(enkf_state , key , config_node);
  }

  // 2: Add container nodes - must ensure that all other nodes have
  //    been added already (this implies that containers of containers
  //    will be victim of hash retrieval order problems ....

  for (int ik = 0; ik < stringlist_get_size( container_keys ); ik++) {
    const char * key = stringlist_iget(container_keys, ik);
    const enkf_config_node_type * config_node = ensemble_config_get_node(ensemble_config , key);
    enkf_state_add_node( enkf_state , key , config_node );
  }

  stringlist_free(keylist);
  stringlist_free( container_keys );
}


/**
   This variable is on a per-instance basis, but that is not really
   supported. The exported functionality applies to all realizations.
*/

bool enkf_state_get_pre_clear_runpath( const enkf_state_type * enkf_state ) {
  return member_config_pre_clear_runpath( enkf_state->my_config );
}


void enkf_state_set_pre_clear_runpath( enkf_state_type * enkf_state , bool pre_clear_runpath ) {
  member_config_set_pre_clear_runpath( enkf_state->my_config , pre_clear_runpath );
}



enkf_state_type * enkf_state_alloc(int iens,
                                   rng_type                  * main_rng ,
                                   enkf_fs_type              * fs,
                                   const char                * casename ,
                                   bool                        pre_clear_runpath ,
                                   keep_runpath_type           keep_runpath ,
                                   model_config_type         * model_config,
                                   ensemble_config_type      * ensemble_config,
                                   const site_config_type    * site_config,
                                   const ecl_config_type     * ecl_config,

                                   ert_templates_type        * templates,
                                   subst_list_type           * subst_parent) {

  enkf_state_type * enkf_state  = util_malloc(sizeof *enkf_state );
  UTIL_TYPE_ID_INIT( enkf_state , ENKF_STATE_TYPE_ID );

  enkf_state->ensemble_config   = ensemble_config;
  enkf_state->shared_info       = shared_info_alloc(site_config , model_config , ecl_config , templates);

  enkf_state->node_hash         = hash_alloc();
  enkf_state->restart_kw_list   = stringlist_alloc_new();
  enkf_state->subst_list        = subst_list_alloc( subst_parent );
  enkf_state->rng               = rng_alloc( rng_get_type( main_rng ) , INIT_DEFAULT );
  rng_rng_init( enkf_state->rng , main_rng );  /* <- Not thread safe */
  /*
    The user MUST specify an INIT_FILE, and for the first timestep the
    <INIT> tag in the data file will be replaced by an

    INCLDUE
    EQUIL_INIT_FILE

    statement. When considering the possibility of estimating EQUIL this
    require a real understanding of the treatment of paths:

    * If not estimating the EQUIL contacts, all members should use the
    same init_file. To ensure this the user must specify the ABSOLUTE
    PATH to a file containing initialization information.

    * If the user is estimating initial contacts, the INIT_FILE must
    point to the ecl_file of the EQUIL keyword, this must be a pure
    filename without any path component (as it will be generated by
    the EnKF program, and placed in the run_path directory). We could
    let the EnKF program use the ecl_file of the EQUIL keyword if it
    is present.

    The <INIT> key is actually initialized in the
    enkf_state_set_dynamic_subst_kw() function.
  */

  /**
     Adding all the subst_kw keywords here, with description. Listing
     all of them here in one go guarantees that we have control over
     the ordering (which is interesting because the substititions are
     done in a cascade like fashion). The user defined keywords are
     added first, so that these can refer to the built in keywords.
  */

  enkf_state_add_subst_kw(enkf_state , "RUNPATH"       , "---" , "The absolute path of the current forward model instance. ");
  enkf_state_add_subst_kw(enkf_state , "IENS"          , "---" , "The realisation number for this realization.");
  enkf_state_add_subst_kw(enkf_state , "IENS4"         , "---" , "The realization number for this realization - formated with %04d.");
  enkf_state_add_subst_kw(enkf_state , "ECLBASE"       , "---" , "The ECLIPSE basename for this realization.");
  enkf_state_add_subst_kw(enkf_state , "ECL_BASE"      , "---" , "Depreceated - use ECLBASE instead.");
  enkf_state_add_subst_kw(enkf_state , "SMSPEC"        , "---" , "The ECLIPSE SMSPEC file for this realization.");
  enkf_state_add_subst_kw(enkf_state , "TSTEP1"        , "---" , "The initial report step for this simulation.");
  enkf_state_add_subst_kw(enkf_state , "TSTEP2"        , "---" , "The final report step for this simulation.");
  enkf_state_add_subst_kw(enkf_state , "TSTEP1_04"     , "---" , "The initial report step for this simulation - formated with %04d.");
  enkf_state_add_subst_kw(enkf_state , "TSTEP2_04"     , "---" , "The final report step for this simulation - formated withh %04d.");
  enkf_state_add_subst_kw(enkf_state , "RESTART_FILE1" , "---" , "The ECLIPSE restart file this simulation starts with.");
  enkf_state_add_subst_kw(enkf_state , "RESTART_FILE2" , "---" , "The ECLIPSE restart file this simulation should end with.");
  enkf_state_add_subst_kw(enkf_state , "RANDINT"       , "---" , "Random integer value (depreceated: use __RANDINT__() instead).");
  enkf_state_add_subst_kw(enkf_state , "RANDFLOAT"     , "---" , "Random float value (depreceated: use __RANDFLOAT__() instead).");
  enkf_state_add_subst_kw(enkf_state , "INIT"          , "---" , "The code which will be inserted at the <INIT> tag");
  if (casename != NULL)
    enkf_state_add_subst_kw(enkf_state , "CASE" , casename , "The casename for this realization - as loaded from the CASE_TABLE file.");
  else
    enkf_state_add_subst_kw(enkf_state , "CASE" , "---" , "The casename for this realization - similar to ECLBASE.");

  enkf_state->my_config = member_config_alloc( iens , casename , pre_clear_runpath , keep_runpath , ecl_config , ensemble_config , fs);
  enkf_state_set_static_subst_kw( enkf_state );
  enkf_state_add_nodes( enkf_state , ensemble_config );

  return enkf_state;
}




bool enkf_state_has_node(const enkf_state_type * enkf_state , const char * node_key) {
  bool has_node = hash_has_key(enkf_state->node_hash , node_key);
  return has_node;
}



/**
   The enkf_state inserts a reference to the node object. The
   enkf_state object takes ownership of the node object, i.e. it will
   free it when the game is over.

   Observe that if the node already exists the existing node will be
   removed (freed and so on ... ) from the enkf_state object before
   adding the new; this was previously considered a run-time error.
*/


void enkf_state_add_node(enkf_state_type * enkf_state , const char * node_key , const enkf_config_node_type * config) {
  if (enkf_state_has_node(enkf_state , node_key))
    enkf_state_del_node( enkf_state , node_key );   /* Deleting the old instance (if we had one). */
  {
    enkf_node_type *enkf_node;
    if (enkf_config_node_get_impl_type( config ) == CONTAINER)
      enkf_node = enkf_node_alloc_shared_container( config , enkf_state->node_hash );
    else
      enkf_node = enkf_node_alloc( config );

    hash_insert_hash_owned_ref(enkf_state->node_hash , node_key , enkf_node, enkf_node_free__);

    /* Setting the global subst list so that the GEN_KW templates can contain e.g. <IENS> and <CWD>. */
    if (enkf_node_get_impl_type( enkf_node ) == GEN_KW)
      gen_kw_set_subst_parent( enkf_node_value_ptr( enkf_node ) , enkf_state->subst_list );
  }
}


enkf_node_type * enkf_state_get_or_create_node(enkf_state_type * enkf_state, const enkf_config_node_type * config_node) {
    const char * key = enkf_config_node_get_key(config_node);
    if(!enkf_state_has_node(enkf_state, key)) {
        enkf_state_add_node(enkf_state, key, config_node);
    }
    return enkf_state_get_node(enkf_state, key);
}



void enkf_state_update_node( enkf_state_type * enkf_state , const char * node_key ) {
  const enkf_config_node_type * config_node = ensemble_config_get_node( enkf_state->ensemble_config , node_key );
  if (!enkf_state_has_node( enkf_state , node_key))
    enkf_state_add_node( enkf_state , node_key , config_node );  /* Add a new node */
  else {
    bool modified = true;   /* ehhhh? */

    if (modified)
      enkf_state_add_node( enkf_state , node_key , config_node );
  }
}


const char * enkf_state_get_eclbase( const enkf_state_type * enkf_state ) {
  return member_config_get_eclbase( enkf_state->my_config );
}


static ecl_sum_type * enkf_state_load_ecl_sum(const enkf_state_type * enkf_state , const run_arg_type * run_arg , stringlist_type * messages , int * result) {
  const ecl_config_type * ecl_config     = enkf_state->shared_info->ecl_config;
  if (ecl_config_active( ecl_config )) {
    const bool fmt_file                    = ecl_config_get_formatted(ecl_config);
    const char * eclbase                   = enkf_state_get_eclbase( enkf_state );


    stringlist_type * data_files           = stringlist_alloc_new();
    char * header_file                     = ecl_util_alloc_exfilename(run_arg_get_runpath(run_arg) , eclbase , ECL_SUMMARY_HEADER_FILE , fmt_file , -1);
    char * unified_file                    = ecl_util_alloc_exfilename(run_arg_get_runpath(run_arg) , eclbase , ECL_UNIFIED_SUMMARY_FILE , fmt_file ,  -1);
    ecl_sum_type * summary                 = NULL;

    /* Should we load from a unified summary file, or from several non-unified files? */
    if (unified_file != NULL)
      /* Use unified file: */
      stringlist_append_ref( data_files , unified_file);
    else {
      /* Use several non unified files. */
      /* Bypassing the query to model_config_load_results() */
      int report_step = run_arg_get_load_start( run_arg );
      if (report_step == 0)
        report_step++;     // Ignore looking for the .S0000 summary file (it does not exist).
      while (true) {
        char * summary_file = ecl_util_alloc_exfilename(run_arg_get_runpath( run_arg ) , eclbase , ECL_SUMMARY_FILE , fmt_file ,  report_step);

        if (summary_file != NULL)
          stringlist_append_owned_ref( data_files , summary_file);
        else
          /*
             We stop the loading at first 'hole' in the series of summary files;
             the internalize layer must report failure if we are missing data.
          */
          break;

        if ((run_arg_get_run_mode( run_arg ) == ENKF_ASSIMILATION) && (run_arg_get_step2( run_arg ) == report_step))
          break;

        report_step++;
      }
    }

    if ((header_file != NULL) && (stringlist_get_size(data_files) > 0)) {
      summary = ecl_sum_fread_alloc(header_file , data_files , SUMMARY_KEY_JOIN_STRING );
      {
        time_t end_time = ecl_config_get_end_date( ecl_config );
        if (end_time > 0) {
          if (ecl_sum_get_end_time( summary ) < end_time) {
            /* The summary vector was shorter than expected; we interpret this as
               a simulation failure and discard the current summary instance. */
            {
              int end_day,end_month,end_year;
              int sum_day,sum_month,sum_year;

              util_set_date_values( end_time , &end_day , &end_month , &end_year );
              util_set_date_values( ecl_sum_get_end_time( summary ) , &sum_day , &sum_month , &sum_year );
              stringlist_append_owned_ref( messages ,
                                           util_alloc_sprintf("Summary ended at %02d/%02d/%4d - expected at least END_DATE: %02d/%02d/%4d" ,
                                                              sum_day , sum_month , sum_year ,
                                                              end_day , end_month , end_year ));
            }
            ecl_sum_free( summary );
            summary = NULL;
            *result |= LOAD_FAILURE;
          }
        }
      }
    }
    stringlist_free( data_files );
    util_safe_free( header_file );
    util_safe_free( unified_file );
    return summary;
  } else
    return NULL;
}


static void enkf_state_log_GEN_DATA_load( const enkf_node_type * enkf_node , int report_step , stringlist_type * msg_list) {
  /* In interactive mode we explicitly report the loads of GEN_DATA instances. */
  char * load_file = enkf_config_node_alloc_infile(enkf_node_get_config( enkf_node ) , report_step);
  int data_size = gen_data_get_size( enkf_node_value_ptr( enkf_node ));
  stringlist_append_owned_ref( msg_list ,
                               util_alloc_sprintf("Loaded GEN_DATA:%s instance for step:%d from file:%s size:%d" ,
                                                  enkf_node_get_key( enkf_node ) ,
                                                  report_step ,
                                                  load_file ,
                                                  data_size));
  free( load_file );
}

static void enkf_state_log_custom_kw_load(const enkf_node_type * enkf_node, int report_step, stringlist_type * msg_list) {
  /* In interactive mode we explicitly report the loads of GEN_DATA instances. */
  char * load_file = enkf_config_node_alloc_infile(enkf_node_get_config(enkf_node), report_step);
  stringlist_append_owned_ref(msg_list,
                               util_alloc_sprintf("Loaded CUSTOM_KW: %s instance for step: %d from file: %s",
                                                  enkf_node_get_key(enkf_node),
                                                  report_step,
                                                  load_file));
  free(load_file);
}

static bool enkf_state_report_step_compatible(const enkf_state_type * enkf_state, const ecl_sum_type * ecl_sum_simulated) {
  bool ret = true;

  const model_config_type * model_config = enkf_state->shared_info->model_config;
  const ecl_sum_type * ecl_sum_reference = model_config_get_refcase(model_config);

  if (ecl_sum_reference) //Can be NULL
    ret = ecl_sum_report_step_compatible(ecl_sum_reference, ecl_sum_simulated);

  return ret;
}


static int_vector_type * __enkf_state_get_time_index(enkf_fs_type * result_fs, ecl_sum_type * summary) {
    time_map_type * time_map = enkf_fs_get_time_map( result_fs );
    time_map_summary_update( time_map , summary );
    return time_map_alloc_index_map( time_map , summary );
}


static bool enkf_state_internalize_dynamic_eclipse_results(enkf_state_type * enkf_state , run_arg_type * run_arg , const model_config_type * model_config , int * result, bool interactive , stringlist_type * msg_list) {
  bool load_summary = ensemble_config_has_impl_type(enkf_state->ensemble_config, SUMMARY);
  if (load_summary) {
    int load_start = run_arg_get_load_start( run_arg );

    if (load_start == 0) { /* Do not attempt to load the "S0000" summary results. */
      load_start++;
    }

    {
      /* Looking for summary files on disk, and loading them. */
      ecl_sum_type * summary = enkf_state_load_ecl_sum( enkf_state , run_arg , msg_list , result );
      enkf_fs_type * result_fs = run_arg_get_result_fs( run_arg );
      /** OK - now we have actually loaded the ecl_sum instance, or ecl_sum == NULL. */
      if (summary != NULL) {
        int_vector_type * time_index = __enkf_state_get_time_index(result_fs, summary);

        /*
           Now there are two related / conflicting(?) systems for
           checking summary time consistency, both internally in the
           time_map and also through the
           enkf_state_report_step_compatible() function.
        */

        /*Check the loaded summary against the reference ecl_sum_type */
        if (!enkf_state_report_step_compatible(enkf_state, summary)) {
          *result |= REPORT_STEP_INCOMPATIBLE;
        }


        /* The actual loading internalizing - from ecl_sum -> enkf_node. */
        const int iens   = member_config_get_iens( enkf_state->my_config );
        const int step2  = ecl_sum_get_last_report_step( summary );  /* Step2 is just taken from the number of steps found in the summary file. */

        int_vector_iset_block( time_index , 0 , load_start , -1 );
        int_vector_resize( time_index , step2 + 1);

        const summary_key_matcher_type * matcher = ensemble_config_get_summary_key_matcher(enkf_state->ensemble_config);
        const ecl_smspec_type * smspec = ecl_sum_get_smspec(summary);

        for(int i = 0; i < ecl_smspec_num_nodes(smspec); i++) {
            const smspec_node_type * smspec_node = ecl_smspec_iget_node(smspec, i);
            const char * key = smspec_node_get_gen_key1(smspec_node);

            if(summary_key_matcher_match_summary_key(matcher, key)) {
                summary_key_set_type * key_set = enkf_fs_get_summary_key_set(result_fs);
                summary_key_set_add_summary_key(key_set, key);

                enkf_config_node_type * config_node = ensemble_config_get_or_create_summary_node(enkf_state->ensemble_config, key);
                enkf_node_type * node = enkf_state_get_or_create_node(enkf_state, config_node);

                enkf_node_try_load_vector( node , result_fs , iens , FORECAST );  // Ensure that what is currently on file is loaded before we update.

                enkf_node_forward_load_vector( node , run_arg_get_runpath( run_arg ) , summary , NULL , time_index , iens);
                enkf_node_store_vector( node , result_fs , iens , FORECAST );
            }
        }

        /* */
        stringlist_type * keys = summary_key_matcher_get_keys(matcher);
        for(int i = 0; i < stringlist_get_size(keys); i++) {
            const char * key = stringlist_iget(keys, i);
            if(summary_key_matcher_summary_key_is_required(matcher, key)) {
                if(!ecl_smspec_has_general_var(smspec, key)) {
                    *result |= LOAD_FAILURE;
                    ert_log_add_fmt_message( 3 , NULL , "[%03d:----] Failed to load data for vector node: %s", iens , key);
                    if (interactive) {
                        stringlist_append_owned_ref( msg_list , util_alloc_sprintf("Failed to load vector: %s" , key));
                    }
                }
            }
        }

        stringlist_free(keys);
        ecl_sum_free( summary );
        int_vector_free( time_index );
        return true;
      } else {
        fprintf(stderr , "** Warning: could not load ECLIPSE summary data from %s - this will probably fail later ...\n" , run_arg_get_runpath( run_arg ));
        return false;
      }
    }
  } else {
    return true;
  }
}





/**
   The ECLIPSE restart files can contain several instances of the same
   keyword, e.g. AQUIFER info can come several times with identical
   headers, also when LGR is in use the same header for
   e.g. PRESSURE/SWAT/INTEHEAD/... will occur several times. The
   enkf_state/ensembl_config objects require unique keys.

   This function takes keyword string and an occurence number, and
   combine them to one new string like this:

   __realloc_static_kw("INTEHEAD" , 0) ==>  "INTEHEAD_0"

   In the enkf layer the key used will then be INTEHEAD_0.
*/


static char * __realloc_static_kw(char * kw , int occurence) {
  char * new_kw = util_alloc_sprintf("%s_%d" , kw , occurence);
  free(kw);
  ecl_util_escape_kw(new_kw);
  return new_kw;
}


static void enkf_state_internalize_custom_kw(enkf_state_type * enkf_state,
                                            run_arg_type * run_arg,
                                            const model_config_type * model_config,
                                            int * result,
                                            bool interactive,
                                            stringlist_type * msg_list) {

    member_config_type * my_config   = enkf_state->my_config;
    const int iens                   = member_config_get_iens( my_config );
    stringlist_type * custom_kw_keys = ensemble_config_alloc_keylist_from_impl_type(enkf_state->ensemble_config, CUSTOM_KW);
    enkf_fs_type * result_fs         = run_arg_get_result_fs(run_arg);
    const int report_step            = 0;

    custom_kw_config_set_type * config_set = enkf_fs_get_custom_kw_config_set(result_fs);
    custom_kw_config_set_reset(config_set);

    for (int ikey=0; ikey < stringlist_get_size(custom_kw_keys); ikey++) {
        const char* custom_kw_key = stringlist_iget(custom_kw_keys, ikey);
        enkf_node_type * node = enkf_state_get_node(enkf_state, custom_kw_key);

        if (enkf_node_vector_storage(node)) {
            util_abort("%s: Vector storage not correctly implemented for CUSTOM_KW\n", __func__);
        } else {
            if (enkf_node_internalize(node, report_step)) {
                if (enkf_node_has_func(node, forward_load_func)) {
                    if (enkf_node_forward_load(node, run_arg_get_runpath(run_arg), NULL, NULL, report_step, iens)) {
                        node_id_type node_id = {.report_step = report_step, .iens = iens, .state = FORECAST};

                        enkf_node_store(node, result_fs, false, node_id);

                        const enkf_config_node_type * config_node = enkf_node_get_config(node);
                        const custom_kw_config_type * custom_kw_config = (custom_kw_config_type*) enkf_config_node_get_ref(config_node);
                        custom_kw_config_set_add_config(config_set, custom_kw_config);

                        if (interactive) {
                            enkf_state_log_custom_kw_load(node, report_step, msg_list);
                        }

                    } else {
                        *result |= LOAD_FAILURE;
                        ert_log_add_fmt_message(1, stderr, "[%03d:%04d] Failed load data for node: %s.", iens , report_step, enkf_node_get_key(node));

                        if (interactive) {
                            stringlist_append_owned_ref(msg_list, util_alloc_sprintf("Failed to load: %s at step: %d", enkf_node_get_key(node), report_step));
                        }
                    }
                }
            }
        }
    }

    stringlist_free(custom_kw_keys);
}



static void enkf_state_internalize_GEN_DATA(enkf_state_type * enkf_state ,
                                            run_arg_type * run_arg ,
                                            const model_config_type * model_config ,
                                            int last_report ,
                                            int * result,
                                            bool interactive ,
                                            stringlist_type * msg_list) {
  {
    member_config_type * my_config     = enkf_state->my_config;
    const int  iens                    = member_config_get_iens( my_config );
    stringlist_type * keylist_GEN_DATA = ensemble_config_alloc_keylist_from_impl_type(enkf_state->ensemble_config , GEN_DATA );
    enkf_fs_type * result_fs           = run_arg_get_result_fs( run_arg );

    for (int ikey=0; ikey < stringlist_get_size( keylist_GEN_DATA ); ikey++) {
      enkf_node_type * node = enkf_state_get_node( enkf_state , stringlist_iget( keylist_GEN_DATA , ikey));

      if (enkf_node_vector_storage(node)) {

        util_abort("%s: holy shit - vector storage not correctly implemented for GEN_DATA\n",__func__);
        enkf_node_try_load_vector( node , result_fs , iens , FORECAST);
        if (enkf_node_forward_load_vector( node , run_arg_get_runpath( run_arg ) , NULL , NULL , NULL , iens)) {
          enkf_node_store_vector( node , result_fs , iens , FORECAST );
          if (interactive)
            enkf_state_log_GEN_DATA_load( node , 0 , msg_list );
        } else {
          *result |= LOAD_FAILURE;
          ert_log_add_fmt_message(3 , NULL , "[%03d:----] Failed to load data for vector node:%s.",iens , enkf_node_get_key( node ));
          if (interactive)
            stringlist_append_owned_ref( msg_list , util_alloc_sprintf("Failed to load vector:%s" , enkf_node_get_key( node )));
        }

      } else {

        for (int report_step = run_arg_get_load_start( run_arg ); report_step <= last_report; report_step++) {
          if (enkf_node_internalize(node , report_step)) {

            if (enkf_node_has_func(node , forward_load_func)) {
              if (enkf_node_forward_load(node , run_arg_get_runpath( run_arg ) , NULL , NULL  , report_step , iens )) {
                node_id_type node_id = {.report_step = report_step ,
                                        .iens = iens ,
                                        .state = FORECAST };

                enkf_node_store( node , result_fs, false , node_id );

                if (interactive)
                  enkf_state_log_GEN_DATA_load( node , report_step , msg_list );

              } else {
                *result |= LOAD_FAILURE;
                ert_log_add_fmt_message(1 , stderr , "[%03d:%04d] Failed load data for node:%s.",iens , report_step , enkf_node_get_key( node ));

                if (interactive)
                  stringlist_append_owned_ref(msg_list ,
                                              util_alloc_sprintf("Failed to load: %s at step:%d" , enkf_node_get_key( node ) , report_step));
              }
            }
          }
        }
      }
    }
  }
}


/**
   This function loads the STATE from a forward simulation. In ECLIPSE
   speak that means to load the solution vectors (PRESSURE/SWAT/..)
   and the necessary static keywords.

   When the state has been loaded it goes straight to disk.
*/

static void enkf_state_internalize_eclipse_state(enkf_state_type * enkf_state ,
                                                 run_arg_type * run_arg ,
                                                 const model_config_type * model_config ,
                                                 int report_step ,
                                                 bool store_vectors ,
                                                 int * result,
                                                 bool interactive ,
                                                 stringlist_type * msg_list) {
  shared_info_type   * shared_info   = enkf_state->shared_info;
  const ecl_config_type * ecl_config = shared_info->ecl_config;
  enkf_fs_type * result_fs = run_arg_get_result_fs( run_arg );
  if (ecl_config_active( ecl_config )) {
    member_config_type * my_config     = enkf_state->my_config;
    const int  iens                    = member_config_get_iens( my_config );
    const bool fmt_file                = ecl_config_get_formatted( ecl_config );
    const bool unified                 = ecl_config_get_unified_restart( ecl_config );
    const bool internalize_state       = model_config_internalize_state( model_config , report_step );
    ecl_file_type  * restart_file;


    /**
       Loading the restart block.
    */

    if (unified)
      util_abort("%s: sorry - unified restart files are not supported \n",__func__);
    {
      char * filename  = ecl_util_alloc_exfilename(run_arg_get_runpath(run_arg) , member_config_get_eclbase(enkf_state->my_config) , ECL_RESTART_FILE , fmt_file , report_step);
      if (filename) {
        restart_file = ecl_file_open( filename , 0 );
        free(filename);
      } else
        restart_file = NULL;  /* No restart information was found; if that is expected the program will fail hard in the enkf_node_forward_load() functions. */
    }

    /*****************************************************************/


    /**
       Iterating through the restart file:

       1. Build up enkf_state->restart_kw_list.
       2. Send static keywords straight out.
    */

    if (restart_file) {
      stringlist_clear( enkf_state->restart_kw_list );
      {
        int ikw;

        for (ikw =0; ikw < ecl_file_get_size( restart_file ); ikw++) {
          ert_impl_type impl_type;
          const ecl_kw_type * ecl_kw = ecl_file_iget_kw( restart_file , ikw);
          int occurence              = ecl_file_iget_occurence( restart_file , ikw ); /* This is essentially the static counter value. */
          char * kw                  = util_alloc_string_copy( ecl_kw_get_header( ecl_kw ) );
          /**
              Observe that this test will never succeed for static keywords,
              because the internalized key has appended a _<occurence>.
          */
          if (ensemble_config_has_key(enkf_state->ensemble_config , kw)) {
            /**
               This is poor-mans treatment of LGR. When LGR is used the restart file
               will contain repeated occurences of solution vectors, like
               PRESSURE. The first occurence of PRESSURE will be for the ordinary
               grid, and then there will be subsequent PRESSURE sections for each
               LGR section. The way this is implemented here is as follows:

               1. The first occurence of pressure is internalized as the enkf_node
               pressure (if we indeed have a pressure node).

               2. The consecutive pressure nodes are internalized as static
               parameters.

               The variable 'occurence' is the key here.
            */

            if (occurence == 0) {
              const enkf_config_node_type * config_node = ensemble_config_get_node(enkf_state->ensemble_config , kw);
              impl_type = enkf_config_node_get_impl_type(config_node);
            } else
              impl_type = STATIC;
          } else
            impl_type = STATIC;


          if (impl_type == FIELD)
            stringlist_append_copy(enkf_state->restart_kw_list , kw);
          else if (impl_type == STATIC) {
            if (ecl_config_include_static_kw(ecl_config , kw)) {
              /* It is a static kw like INTEHEAD or SCON */
              /*
                 Observe that for static keywords we do NOT ask the node 'privately' if
                 internalize_state is false: It is impossible to single out static keywords for
                 internalization.
              */

              /* Now we mangle the static keyword .... */
              kw = __realloc_static_kw(kw , occurence);

              if (internalize_state) {
                stringlist_append_copy( enkf_state->restart_kw_list , kw);

                ensemble_config_ensure_static_key(enkf_state->ensemble_config , kw );

                if (!enkf_state_has_node(enkf_state , kw)) {
                  const enkf_config_node_type * config_node = ensemble_config_get_node(enkf_state->ensemble_config , kw);
                  enkf_state_add_node(enkf_state , kw , config_node);
                }

                /*
                   The following thing can happen:

                   1. A static keyword appears at report step n, and is added to the enkf_state
                   object.

                   2. At report step n+k that static keyword is no longer active, and it is
                   consequently no longer part of restart_kw_list().

                   3. However it is still part of the enkf_state. Not loaded here, and subsequently
                   purged from enkf_main.

                   One keyword where this occurs is FIPOIL, which at least might appear only in the
                   first restart file. Unused static keywords of this type are purged from the
                   enkf_main object by a call to enkf_main_del_unused_static(). The purge is based on
                   looking at the internal __report_step state of the static kw.
                */

                {
                  enkf_node_type * enkf_node  = enkf_state_get_node(enkf_state , kw);
                  node_id_type node_id        = {.report_step = report_step , .iens = iens , .state = FORECAST };

                  enkf_node_ecl_load_static(enkf_node , ecl_kw , report_step , iens);
                  /*
                    Static kewyords go straight out ....
                  */
                  enkf_node_store(enkf_node , result_fs , true , node_id);
                  enkf_node_free_data(enkf_node);
                }
              }
            }
          } else
            util_abort("%s: hm - something wrong - can (currently) only load FIELD/STATIC implementations from restart files - aborting \n",__func__);
          free(kw);
        }
        enkf_fs_fwrite_restart_kw_list( result_fs , report_step , iens , enkf_state->restart_kw_list );
      }
    }

    /******************************************************************/
    /**
        Starting on the enkf_node_forward_load() function calls. This is where the
        actual loading (apart from static keywords) is done. Observe that this
        loading might involve other load functions than the ones used for
        loading PRESSURE++ from ECLIPSE restart files (e.g. for loading seismic
        results..)
    */

    {
      hash_iter_type * iter = hash_iter_alloc(enkf_state->node_hash);
      while ( !hash_iter_is_complete(iter) ) {
        enkf_node_type * enkf_node = hash_iter_get_next_value(iter);
        if (enkf_node_get_var_type(enkf_node) == DYNAMIC_STATE &&
            enkf_node_get_impl_type(enkf_node) == FIELD) {

          bool internalize_kw = internalize_state;
          if (!internalize_kw)
            internalize_kw = enkf_node_internalize(enkf_node , report_step);

          if (internalize_kw) {
            if (enkf_node_has_func(enkf_node , forward_load_func)) {
              if (enkf_node_forward_load(enkf_node , run_arg_get_runpath( run_arg ) , NULL , restart_file , report_step , iens )) {
                node_id_type node_id = {.report_step = report_step ,
                                        .iens = iens ,
                                        .state = FORECAST };

                enkf_node_store( enkf_node , result_fs, store_vectors , node_id );
              } else {
                *result |= LOAD_FAILURE;
                ert_log_add_fmt_message( 1 , NULL , "[%03d:%04d] Failed load data for node:%s.",iens , report_step , enkf_node_get_key( enkf_node ));

                if (interactive)
                  stringlist_append_owned_ref(msg_list , util_alloc_sprintf("Failed to load: %s at step:%d" , enkf_node_get_key( enkf_node ) , report_step));
              }
            }
          }
        }
      }
      hash_iter_free(iter);
    }

    /*****************************************************************/
    /* Cleaning up */
    if (restart_file != NULL) ecl_file_close( restart_file );
  }
}




/**
   This function loads the results from a forward simulations from report_step1
   to report_step2. The details of what to load are in model_config and the
   spesific nodes for special cases.

   Will mainly be called at the end of the forward model, but can also
   be called manually from external scope.
*/


static void enkf_state_internalize_results(enkf_state_type * enkf_state , run_arg_type * run_arg , int * result , bool interactive , stringlist_type * msg_list) {
  model_config_type * model_config = enkf_state->shared_info->model_config;
  int report_step;

  /*
    The timing information - i.e. mainly what is the last report step
    in these results are inferred from the loading of summary results,
    hence we must load the summary results first.
  */

  enkf_state_internalize_dynamic_eclipse_results(enkf_state , run_arg , model_config , result, interactive , msg_list);
  {
    enkf_fs_type * result_fs = run_arg_get_result_fs( run_arg );
    int last_report = time_map_get_last_step( enkf_fs_get_time_map( result_fs ));
    if (last_report < 0)
      last_report = model_config_get_last_history_restart( enkf_state->shared_info->model_config);

    /*
      If we are in true assimilation mode we use the step2 setting, otherwise we are
      just in plain gready-load-mode.
    */
    if (run_arg_get_run_mode(run_arg) == ENKF_ASSIMILATION)
      last_report = run_arg_get_step2( run_arg );

    /* Ensure that the last step is internalized? */
    model_config_set_internalize_state( model_config , last_report);

    for (report_step = run_arg_get_load_start( run_arg ); report_step <= last_report; report_step++) {
      bool store_vectors = (report_step == last_report) ? true : false;
      if (model_config_load_state( model_config , report_step))
        enkf_state_internalize_eclipse_state(enkf_state , run_arg , model_config , report_step , store_vectors , result , interactive , msg_list);
    }

    enkf_state_internalize_GEN_DATA(enkf_state , run_arg ,  model_config , last_report , result , interactive , msg_list);
    enkf_state_internalize_custom_kw(enkf_state, run_arg, model_config, result, interactive, msg_list);
  }
}


void enkf_state_forward_init(enkf_state_type * enkf_state ,
                             run_arg_type * run_arg ,
                             int * result ) {

  if (run_arg_get_step1(run_arg) == 0) {
    int iens = enkf_state_get_iens( enkf_state );
    hash_iter_type * iter = hash_iter_alloc( enkf_state->node_hash );
    while ( !hash_iter_is_complete(iter) ) {
      enkf_node_type * node = hash_iter_get_next_value(iter);
      if (enkf_node_use_forward_init(node)) {
        enkf_fs_type * result_fs = run_arg_get_result_fs( run_arg );
        node_id_type node_id = {.report_step = 0 ,
                                .iens = iens ,
                                .state = ANALYZED };


        /*
           Will not reinitialize; i.e. it is essential that the
           forward model uses the state given from the stored
           instance, and not from the current run of e.g. RMS.
        */

        if (!enkf_node_has_data( node , result_fs , node_id)) {
          if (enkf_node_forward_init(node , run_arg_get_runpath( run_arg ) , iens ))
            enkf_node_store( node , result_fs , false , node_id );
          else {
            char * init_file = enkf_config_node_alloc_initfile( enkf_node_get_config( node ) , run_arg_get_runpath(run_arg) , iens );

            if (init_file && !util_file_exists( init_file ))
              fprintf(stderr,"File not found: %s - failed to initialize node: %s\n", init_file , enkf_node_get_key( node ));
            else
              fprintf(stderr,"Failed to initialize node: %s\n", enkf_node_get_key( node ));

            util_safe_free( init_file );
            *result |= LOAD_FAILURE;
          }
        }

      }
    }
    hash_iter_free( iter );
  }

}



void enkf_state_load_from_forward_model(enkf_state_type * enkf_state ,
                                        run_arg_type * run_arg ,
                                        int * result,
                                        bool interactive ,
                                        stringlist_type * msg_list) {


  if (ensemble_config_have_forward_init( enkf_state->ensemble_config ))
    enkf_state_forward_init( enkf_state , run_arg , result );

  enkf_state_internalize_results( enkf_state , run_arg , result , interactive , msg_list );
  {
    state_map_type * state_map = enkf_fs_get_state_map( run_arg_get_result_fs( run_arg ) );
    int iens = member_config_get_iens( enkf_state->my_config );
    if (*result & LOAD_FAILURE)
      state_map_iset( state_map , iens , STATE_LOAD_FAILURE);
    else
      state_map_iset( state_map , iens , STATE_HAS_DATA);
  }
}


/**
   Observe that this does not return the loadOK flag; it will load as
   good as it can all the data it should, and be done with it.
*/

void * enkf_state_load_from_forward_model_mt( void * arg ) {
  arg_pack_type * arg_pack     = arg_pack_safe_cast( arg );
  enkf_state_type * enkf_state = enkf_state_safe_cast(arg_pack_iget_ptr( arg_pack  , 0 ));
  run_arg_type * run_arg       = arg_pack_iget_ptr( arg_pack  , 1 );
  bool interactive             = arg_pack_iget_bool( arg_pack , 2 );
  stringlist_type * msg_list   = arg_pack_iget_ptr( arg_pack  , 3 );
  bool manual_load             = arg_pack_iget_bool( arg_pack , 4 );
  int * result                 = arg_pack_iget_ptr( arg_pack  , 5 );

  int iens                     = run_arg_get_iens( run_arg );

  if (manual_load)
    state_map_update_undefined(enkf_fs_get_state_map( run_arg_get_result_fs(run_arg) ) , iens , STATE_INITIALIZED);

  enkf_state_load_from_forward_model( enkf_state , run_arg , result , interactive , msg_list );
  if (*result & REPORT_STEP_INCOMPATIBLE) {
    // If refcase has been used for observations: crash and burn.
    fprintf(stderr,"** Warning the timesteps in refcase and current simulation are not in accordance - something wrong with schedule file?\n");
    *result -= REPORT_STEP_INCOMPATIBLE;
  }

  if (interactive) {
    printf(".");
    fflush(stdout);
  }
  return NULL;
}




/**
   Observe that this function uses run_arg->step1 to load all the nodes which
   are needed in the restart file. I.e. if you have carefully prepared a funny
   state with dynamic/static data which do not agree with the current value of
   run_arg->step1 YOUR STATE WILL BE OVERWRITTEN.
*/

static void enkf_state_write_restart_file(enkf_state_type * enkf_state , const run_arg_type * run_arg , enkf_fs_type * fs) {
  const member_config_type * my_config = enkf_state->my_config;
  const bool fmt_file                  = ecl_config_get_formatted(enkf_state->shared_info->ecl_config);
  const int iens                       = member_config_get_iens( my_config );
  char * restart_file                  = ecl_util_alloc_filename(run_arg_get_runpath( run_arg ) , member_config_get_eclbase( enkf_state->my_config ) , ECL_RESTART_FILE , fmt_file , run_arg_get_step1(run_arg));
  fortio_type * fortio                 = fortio_open_writer(restart_file , fmt_file , ECL_ENDIAN_FLIP);
  const char * kw;
  int          ikw;

  if (stringlist_get_size(enkf_state->restart_kw_list) == 0)
    enkf_fs_fread_restart_kw_list(fs , run_arg_get_step1(run_arg) , iens , enkf_state->restart_kw_list);

  for (ikw = 0; ikw < stringlist_get_size(enkf_state->restart_kw_list); ikw++) {
    kw = stringlist_iget( enkf_state->restart_kw_list , ikw);
    /*
       Observe that here we are *ONLY* iterating over the
       restart_kw_list instance, and *NOT* the enkf_state
       instance. I.e. arbitrary dynamic keys, and no-longer-active
       static kewyords should not show up.

       If the restart kw_list asks for a keyword which we do not have,
       we assume it is a static keyword and add it it to the
       enkf_state instance.

       This is a bit unfortunate, because a bug/problem of some sort,
       might be masked (seemingly solved) by adding a static keyword,
       before things blow up completely at a later instant.
    */
    if (!ensemble_config_has_key(enkf_state->ensemble_config , kw))
      ensemble_config_ensure_static_key(enkf_state->ensemble_config , kw );

    if (!enkf_state_has_node(enkf_state , kw)) {
      const enkf_config_node_type * config_node = ensemble_config_get_node(enkf_state->ensemble_config , kw);
      enkf_state_add_node(enkf_state , kw , config_node);
    }

    {
      enkf_node_type * enkf_node = enkf_state_get_node(enkf_state , kw);
      enkf_var_type var_type = enkf_node_get_var_type(enkf_node);
      if (var_type == STATIC_STATE) {
        node_id_type node_id = {.report_step = run_arg_get_step1(run_arg) ,
                                .iens = iens ,
                                .state = run_arg_get_dynamic_init_state(run_arg) };
        enkf_node_load( enkf_node , fs , node_id);
      }
      if (var_type == DYNAMIC_STATE) {
        /* Pressure and saturations */
        if (enkf_node_get_impl_type(enkf_node) == FIELD)
          enkf_node_ecl_write(enkf_node , NULL , fortio , run_arg_get_step1(run_arg));
        else
          util_abort("%s: internal error wrong implementetion type:%d - node:%s aborting \n",__func__ , enkf_node_get_impl_type(enkf_node) , enkf_node_get_key(enkf_node));
      } else if (var_type == STATIC_STATE) {
        enkf_node_ecl_write(enkf_node , NULL , fortio , run_arg_get_step1(run_arg));
        enkf_node_free_data(enkf_node); /* Just immediately discard the static data. */
      } else {
        fprintf(stderr,"var_type: %d \n",var_type);
        fprintf(stderr,"node    : %s \n",enkf_node_get_key(enkf_node));
        util_abort("%s: internal error - should not be here ... \n",__func__);
      }

    }
  }
  fortio_fclose(fortio);
  free(restart_file);
}



/**
  This function writes out all the files needed by an ECLIPSE simulation, this
  includes the restart file, and the various INCLUDE files corresponding to
  parameteres estimated by EnKF.

  The writing of restart file is delegated to enkf_state_write_restart_file().
*/

void enkf_state_ecl_write(enkf_state_type * enkf_state, const run_arg_type * run_arg , enkf_fs_type * fs) {
  if (run_arg_get_step1(run_arg) > 0)
    enkf_state_write_restart_file(enkf_state , run_arg , fs);
  else {
    /*
      These keywords are added here becasue otherwise the main loop
      below will try to write them with ecl_write - and that will fail
      (for report_step 0).
    */
    stringlist_append_copy(enkf_state->restart_kw_list , "SWAT");
    stringlist_append_copy(enkf_state->restart_kw_list , "SGAS");
    stringlist_append_copy(enkf_state->restart_kw_list , "PRESSURE");
    stringlist_append_copy(enkf_state->restart_kw_list , "RV");
    stringlist_append_copy(enkf_state->restart_kw_list , "RS");
  }

  {
    /**
        This iteration manipulates the hash (thorugh the enkf_state_del_node() call)

        -----------------------------------------------------------------------------------------
        T H I S  W I L L  D E A D L O C K  I F  T H E   H A S H _ I T E R  A P I   I S   U S E D.
        -----------------------------------------------------------------------------------------
    */

    const shared_info_type * shared_info   = enkf_state->shared_info;
    const model_config_type * model_config = shared_info->model_config;
    const char * base_name                 = model_config_get_gen_kw_export_file(model_config);
    char * export_file_name                = util_alloc_filename( run_arg_get_runpath( run_arg ) , base_name  , NULL);
    FILE * export_file                     = util_mkdir_fopen(export_file_name, "w");


    const int num_keys = hash_get_size(enkf_state->node_hash);
    char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
    int iens = enkf_state_get_iens( enkf_state );
    int ikey;

    for (ikey = 0; ikey < num_keys; ikey++) {
      if (!stringlist_contains(enkf_state->restart_kw_list , key_list[ikey])) {          /* Make sure that the elements in the restart file are not written (again). */
        enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
        if (enkf_node_get_var_type( enkf_node ) != STATIC_STATE) {                        /* Ensure that no-longer-active static keywords do not create problems. */
          bool forward_init = enkf_node_use_forward_init( enkf_node );

          if ((run_arg_get_step1(run_arg) == 0) && (forward_init)) {
            node_id_type node_id = {.report_step = 0,
                                    .iens = iens ,
                                    .state = ANALYZED };

            if (enkf_node_has_data( enkf_node , fs , node_id))
              enkf_node_ecl_write(enkf_node , run_arg_get_runpath( run_arg ) , export_file , run_arg_get_step1(run_arg));
          } else
            enkf_node_ecl_write(enkf_node , run_arg_get_runpath( run_arg ) , export_file , run_arg_get_step1(run_arg));

        }
      }
    }
    util_free_stringlist(key_list , num_keys);

    fclose(export_file);
    free(export_file_name);
  }
}


/**
  This function takes a report_step and a analyzed|forecast state as
  input; the enkf_state instance is set accordingly and written to
  disk.
*/


void enkf_state_fwrite(const enkf_state_type * enkf_state , enkf_fs_type * fs , int mask , int report_step , state_enum state) {
  const member_config_type * my_config = enkf_state->my_config;
  const int num_keys = hash_get_size(enkf_state->node_hash);
  char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
  int ikey;

  for (ikey = 0; ikey < num_keys; ikey++) {
    enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
    if (enkf_node_include_type(enkf_node , mask)) {
      node_id_type node_id = {.report_step = report_step , .iens = member_config_get_iens( my_config ) , .state = state };
      enkf_node_store( enkf_node, fs , true , node_id );
    }
  }
  util_free_stringlist(key_list , num_keys);
}


void enkf_state_fread(enkf_state_type * enkf_state , enkf_fs_type * fs , int mask , int report_step , state_enum state) {
  const member_config_type * my_config = enkf_state->my_config;
  const int num_keys = hash_get_size(enkf_state->node_hash);
  char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
  int ikey;

  for (ikey = 0; ikey < num_keys; ikey++) {
    enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
    if (enkf_node_include_type(enkf_node , mask)) {
      node_id_type node_id = {.report_step = report_step ,
                              .iens = member_config_get_iens( my_config ) ,
                              .state = state };
      bool forward_init = enkf_node_use_forward_init( enkf_node );
      if (forward_init)
        enkf_node_try_load(enkf_node , fs , node_id );
      else
        enkf_node_load(enkf_node , fs , node_id);
    }
  }
  util_free_stringlist(key_list , num_keys);
}


/**
   This function will load all the nodes listed in the current
   restart_kw_list; in addition to all other variable of type
   DYNAMIC_STATE. Observe that for DYNAMIC state nodes it will try
   firt analyzed state and then forecast state.
*/


static void enkf_state_fread_state_nodes(enkf_state_type * enkf_state , enkf_fs_type * fs , int report_step , state_enum load_state) {
  const member_config_type * my_config = enkf_state->my_config;
  const int iens                       = member_config_get_iens( my_config );
  int ikey;


  /*
     First pass - load all the STATIC nodes. It is essential to use
     the restart_kw_list when loading static nodes, otherwise static
     nodes which were only present at e.g. step == 0 will create
     problems: (They are in the enkf_state hash table because they
     were seen at step == 0, but have not been seen subesquently and
     the loading fails.)
  */

  enkf_fs_fread_restart_kw_list( fs , report_step , iens , enkf_state->restart_kw_list);
  for (ikey = 0; ikey < stringlist_get_size( enkf_state->restart_kw_list) ; ikey++) {
    const char * key = stringlist_iget( enkf_state->restart_kw_list, ikey);
    enkf_node_type * enkf_node;
    enkf_var_type    var_type;

    /*
      The restart_kw_list mentions a keyword which is (not yet) part
      of the enkf_state object. This is assumed to be a static keyword
      and added as such.

      This will break hard for the following situation:

        1. Someone has simulated with a dynamic keyword (i.e. field
           FIELD1).

        2. the fellow decides to remove field1 from the configuraton
           and restart a simulation.

      In this case the code will find FIELD1 in the restart_kw_list,
      it will then be automatically added as a static keyword; and
      then final fread_node() function will fail with a type mismatch
      (or node not found); hopefully this scenario is not very
      probable.
    */

    /* add the config node. */
    if (!ensemble_config_has_key( enkf_state->ensemble_config , key))
      ensemble_config_ensure_static_key( enkf_state->ensemble_config , key);

    /* Add the state node */
    if (!enkf_state_has_node( enkf_state , key )) {
      const enkf_config_node_type * config_node = ensemble_config_get_node(enkf_state->ensemble_config , key);
      enkf_state_add_node(enkf_state , key , config_node);
    }

    enkf_node = hash_get(enkf_state->node_hash , key);
    var_type  = enkf_node_get_var_type( enkf_node );

    if (var_type == STATIC_STATE) {
      node_id_type node_id = { .report_step = report_step ,
                               .iens = iens ,
                               .state = load_state };
      enkf_node_load( enkf_node , fs , node_id);
    }
  }


  /* Second pass - DYNAMIC state nodes. */
  {
    const int num_keys = hash_get_size(enkf_state->node_hash);
    char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
    int ikey;

    for (ikey = 0; ikey < num_keys; ikey++) {
      enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
      enkf_var_type var_type = enkf_node_get_var_type( enkf_node );
      node_id_type node_id = {.report_step = report_step ,
                           .iens = iens ,
                           .state = BOTH };

      if (var_type == DYNAMIC_STATE) {
        /*
           Here the enkf_node_try_load() function is used NOT because we accept
           that the node is not present, but because the try_fread()
           function accepts the BOTH state type.
        */
        if (!enkf_node_try_load(enkf_node , fs , node_id))
          util_abort("%s: failed to load node:%s  report_step:%d iens:%d \n",__func__ , key_list[ikey] , report_step , iens  );
      }
    }
    util_free_stringlist(key_list , num_keys);
  }
}



/**
   This is a special function which is only used to load the initial
   state of dynamic_state nodes. It checks if the enkf_config_node has
   set a valid value for input_file, in that case that means we should
   also have an internalized representation of it, otherwise it will
   just return (i.e. for PRESSURE / SWAT).
*/

static void enkf_state_fread_initial_state(enkf_state_type * enkf_state , enkf_fs_type * fs) {
  const member_config_type * my_config = enkf_state->my_config;
  const int num_keys = hash_get_size(enkf_state->node_hash);
  char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
  int ikey;

  for (ikey = 0; ikey < num_keys; ikey++) {
    enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
    if (enkf_node_get_var_type(enkf_node) == DYNAMIC_STATE) {
      const enkf_config_node_type * config_node = enkf_node_get_config( enkf_node );

      /* Just checked for != NULL */
      char * load_file = enkf_config_node_alloc_infile( config_node , 0);
      if (load_file != NULL) {
        node_id_type node_id = {.report_step = 0 ,
                                .iens  = member_config_get_iens( my_config ) ,
                                .state = ANALYZED };
        enkf_node_load(enkf_node , fs , node_id);
      }

      util_safe_free( load_file );
    }
  }
  util_free_stringlist(key_list , num_keys);
}


void enkf_state_free_nodes(enkf_state_type * enkf_state, int mask) {
  const int num_keys = hash_get_size(enkf_state->node_hash);
  char ** key_list   = hash_alloc_keylist(enkf_state->node_hash);
  int ikey;

  for (ikey = 0; ikey < num_keys; ikey++) {
    enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , key_list[ikey]);
    if (enkf_node_include_type(enkf_node , mask))
      enkf_state_del_node(enkf_state , enkf_node_get_key(enkf_node));
  }
  util_free_stringlist(key_list , num_keys);
}






void enkf_state_free(enkf_state_type *enkf_state) {
  rng_free( enkf_state->rng );
  hash_free(enkf_state->node_hash);
  subst_list_free(enkf_state->subst_list);
  stringlist_free(enkf_state->restart_kw_list);
  member_config_free(enkf_state->my_config);
  shared_info_free(enkf_state->shared_info);
  free(enkf_state);
}



enkf_node_type * enkf_state_get_node(const enkf_state_type * enkf_state , const char * node_key) {
  if (hash_has_key(enkf_state->node_hash , node_key)) {
    enkf_node_type * enkf_node = hash_get(enkf_state->node_hash , node_key);
    return enkf_node;
  } else {
    util_abort("%s: node:[%s] not found in state object - aborting.\n",__func__ , node_key);
    return NULL; /* Compiler shut up */
  }
}



void enkf_state_del_node(enkf_state_type * enkf_state , const char * node_key) {
  if (hash_has_key(enkf_state->node_hash , node_key))
    hash_del(enkf_state->node_hash , node_key);
  else
    fprintf(stderr,"%s: tried to remove node:%s which is not in state - internal error?? \n",__func__ , node_key);
}


/**
   This function will set all the subst_kw key=value pairs which
   change with report step.
*/

static void enkf_state_set_dynamic_subst_kw__(enkf_state_type * enkf_state , const char * run_path , int step1 , int step2) {
  const ecl_config_type * ecl_config = enkf_state->shared_info->ecl_config;
  const bool fmt_file      = ecl_config_get_formatted( ecl_config );


  if (run_path != NULL) {
    /** Make absolutely sure the path available as <RUNPATH> is absolute. */
    char * abs_runpath = util_alloc_realpath( run_path );
    enkf_state_add_subst_kw(enkf_state , "RUNPATH"       , abs_runpath      , NULL);
    free( abs_runpath );
  }


  /* Time step */
  {
    char * step1_s           = util_alloc_sprintf("%d" , step1);
    char * step2_s           = util_alloc_sprintf("%d" , step2);
    char * step1_s04         = util_alloc_sprintf("%04d" , step1);
    char * step2_s04         = util_alloc_sprintf("%04d" , step2);

    enkf_state_add_subst_kw(enkf_state , "TSTEP1"        , step1_s       , NULL);
    enkf_state_add_subst_kw(enkf_state , "TSTEP2"        , step2_s       , NULL);
    enkf_state_add_subst_kw(enkf_state , "TSTEP1_04"     , step1_s04     , NULL);
    enkf_state_add_subst_kw(enkf_state , "TSTEP2_04"     , step2_s04     , NULL);

    free(step1_s);
    free(step2_s);
    free(step1_s04);
    free(step2_s04);
  }


  /* Restart file names and RESTART keyword in datafile. */
  {
    const char * eclbase     = member_config_get_eclbase( enkf_state->my_config );
    if (eclbase != NULL) {
      {
        char * restart_file1     = ecl_util_alloc_filename(NULL , eclbase , ECL_RESTART_FILE , fmt_file , step1);
        char * restart_file2     = ecl_util_alloc_filename(NULL , eclbase , ECL_RESTART_FILE , fmt_file , step2);

        enkf_state_add_subst_kw(enkf_state , "RESTART_FILE1" , restart_file1 , NULL);
        enkf_state_add_subst_kw(enkf_state , "RESTART_FILE2" , restart_file2 , NULL);

        free(restart_file1);
        free(restart_file2);
      }

      if (step1 > 0) {
        char * data_initialize = util_alloc_sprintf("RESTART\n   \'%s\'  %d  /\n" , eclbase , step1);
        enkf_state_add_subst_kw(enkf_state , "INIT" , data_initialize , NULL);
        free(data_initialize);
      }
    }
  }

  /**
     The <INIT> magic string:
  */
  if (step1 == 0) {
    const char * init_file = ecl_config_get_equil_init_file(ecl_config);
    if (init_file != NULL) {
      char * tmp_include = util_alloc_sprintf("INCLUDE\n   \'%s\' /\n",init_file);
      enkf_state_add_subst_kw(enkf_state , "INIT" , tmp_include , NULL);
      free(tmp_include);
    } /*
         if init_file == NULL that means the user has not supplied the INIT_SECTION keyword,
         and the EQUIL (or whatever) info to initialize the model is inlined in the datafile.
      */
  }


  {
    /**
        Adding keys for <RANDINT> and <RANDFLOAT> - these are only
        added for backwards compatibility, should be replaced with
        prober function callbacks.
    */
    char * randint_value    = util_alloc_sprintf( "%u"      , rng_forward( enkf_state->rng ));
    char * randfloat_value  = util_alloc_sprintf( "%12.10f" , rng_get_double( enkf_state->rng ));

    enkf_state_add_subst_kw( enkf_state , "RANDINT"   , randint_value   , NULL);
    enkf_state_add_subst_kw( enkf_state , "RANDFLOAT" , randfloat_value , NULL);

    free( randint_value );
    free( randfloat_value );
  }
}

static void enkf_state_set_dynamic_subst_kw(enkf_state_type * enkf_state , const run_arg_type * run_arg ) {
  enkf_state_set_dynamic_subst_kw__( enkf_state , run_arg_get_runpath( run_arg ) , run_arg_get_step1( run_arg ) , run_arg_get_step2( run_arg ));
}



void enkf_state_printf_subst_list(enkf_state_type * enkf_state , int step1 , int step2) {
  int ikw;
  const char * fmt_string = "%-16s %-40s :: %s\n";
  printf("\n\n");
  printf(fmt_string , "Key" , "Current value" , "Description");
  printf("------------------------------------------------------------------------------------------------------------------------\n");
  if (step1 >= 0)
    enkf_state_set_dynamic_subst_kw__(enkf_state , NULL , step1 , step2 );

  for (ikw = 0; ikw < subst_list_get_size( enkf_state->subst_list ); ikw++) {
    const char * key   = subst_list_iget_key( enkf_state->subst_list , ikw);
    const char * value = subst_list_iget_value( enkf_state->subst_list , ikw);
    const char * desc  = subst_list_iget_doc_string( enkf_state->subst_list , ikw );

    if (value != NULL)
      printf(fmt_string , key , value , desc);
    else
      printf(fmt_string , key , "[Not set]" , desc);
  }
  printf("------------------------------------------------------------------------------------------------------------------------\n");

}




/**
   init_step    : The parameters are loaded from this EnKF/report step.
   report_step1 : The simulation should start from this report step;
                  dynamic data are loaded from this step.
   report_step2 : The simulation should stop at this report step. (unless run_mode == ENSEMBLE_PREDICTION - where it just runs til end.)

   For a normal EnKF run we well have init_step == report_step1, but
   in the case where we want rerun from the beginning with updated
   parameters, they will be different. If init_step != report_step1,
   it is required that report_step1 == 0; otherwise the dynamic data
   will become completely inconsistent. We just don't allow that!
*/


void enkf_state_init_eclipse(enkf_state_type *enkf_state, const run_arg_type * run_arg ) {
  const member_config_type  * my_config = enkf_state->my_config;
  const ecl_config_type * ecl_config = enkf_state->shared_info->ecl_config;
  {
    if (member_config_pre_clear_runpath( my_config ))
      util_clear_directory( run_arg_get_runpath( run_arg ) , true , false );

    util_make_path(run_arg_get_runpath( run_arg ));
    {
      if (ecl_config_get_schedule_target( ecl_config ) != NULL) {

        char * schedule_file_target = util_alloc_filename(run_arg_get_runpath( run_arg ) , ecl_config_get_schedule_target( ecl_config ) , NULL);
        char * schedule_file_target_path = util_split_alloc_dirname(schedule_file_target);
        util_make_path(schedule_file_target_path);
        free(schedule_file_target_path);

        if (run_arg_get_run_mode(run_arg) == ENKF_ASSIMILATION)
          sched_file_fprintf_i( ecl_config_get_sched_file( ecl_config ) , run_arg_get_step2(run_arg) , schedule_file_target);
        else
          sched_file_fprintf( ecl_config_get_sched_file( ecl_config ) , schedule_file_target);

        free(schedule_file_target);
      }
    }


    /**
       For reruns of various kinds the parameters and the state are
       generally loaded from different timesteps:
    */
    {
      enkf_fs_type * init_fs = run_arg_get_init_fs( run_arg );
      /* Loading parameter information: loaded from timestep: run_arg->init_step_parameters. */
      enkf_state_fread(enkf_state , init_fs , PARAMETER , run_arg_get_parameter_init_step(run_arg) , run_arg_get_parameter_init_state(run_arg));


      /* Loading state information: loaded from timestep: run_arg->step1 */
      if (run_arg_get_step1(run_arg) == 0)
        enkf_state_fread_initial_state(enkf_state , init_fs);
      else
        enkf_state_fread_state_nodes( enkf_state , init_fs , run_arg_get_step1(run_arg) , run_arg_get_dynamic_init_state(run_arg));

      enkf_state_set_dynamic_subst_kw(  enkf_state , run_arg );
      ert_templates_instansiate( enkf_state->shared_info->templates , run_arg_get_runpath( run_arg ) , enkf_state->subst_list );
      enkf_state_ecl_write( enkf_state , run_arg , init_fs);

      if (member_config_get_eclbase( my_config ) != NULL) {

        /* Writing the ECLIPSE data file. */
        if (ecl_config_get_data_file( ecl_config ) != NULL) {
          char * data_file = ecl_util_alloc_filename(run_arg_get_runpath( run_arg ) , member_config_get_eclbase( my_config ) , ECL_DATA_FILE , true , -1);
          subst_list_filter_file(enkf_state->subst_list , ecl_config_get_data_file(ecl_config) , data_file);
          free( data_file );
        }
      }
    }
    member_config_get_jobname( my_config );
    /* This is where the job script is created */
    forward_model_python_fprintf( model_config_get_forward_model( enkf_state->shared_info->model_config ) , run_arg_get_runpath( run_arg ) , enkf_state->subst_list);
  }
}







bool enkf_state_complete_forward_modelOK__(void * arg );
bool enkf_state_complete_forward_modelEXIT__(void * arg );
bool enkf_state_complete_forward_modelRETRY__(void * arg );


/**
    This function is called when:

     1. The external queue system has said that everything is OK; BUT
        the ert layer failed to load all the data.

     2. The external queue system has seen the job fail.

    The parameter and state variables will be resampled before
    retrying. And all random elements in templates+++ will be
    resampled.
*/



static void enkf_state_internal_retry(enkf_state_type * enkf_state , run_arg_type * run_arg , bool load_failure) {
  const member_config_type  * my_config   = enkf_state->my_config;
  const shared_info_type    * shared_info = enkf_state->shared_info;
  const int iens                          = member_config_get_iens( my_config );

  if (load_failure)
    ert_log_add_fmt_message( 1 , NULL , "[%03d:%04d - %04d] Failed to load all data.",iens , run_arg_get_step1(run_arg) , run_arg_get_step2(run_arg));
  else
    ert_log_add_fmt_message( 1 , NULL , "[%03d:%04d - %04d] Forward model failed.",iens, run_arg_get_step1(run_arg) , run_arg_get_step2(run_arg));

  if (run_arg_can_retry( run_arg ) ) {
    ert_log_add_fmt_message( 1 , NULL , "[%03d] Resampling and resubmitting realization." ,iens);
    {
      /* Reinitialization of the nodes */
      stringlist_type * init_keys = ensemble_config_alloc_keylist_from_var_type( enkf_state->ensemble_config , DYNAMIC_STATE + PARAMETER );
      for (int ikey=0; ikey < stringlist_get_size( init_keys ); ikey++) {
        enkf_node_type * node = enkf_state_get_node( enkf_state , stringlist_iget( init_keys , ikey) );
        enkf_node_initialize( node , iens , enkf_state->rng );
      }
      stringlist_free( init_keys );
    }

    enkf_state_init_eclipse( enkf_state , run_arg  );                                               /* Possibly clear the directory and do a FULL rewrite of ALL the necessary files. */
    job_queue_iset_external_restart( shared_info->job_queue , run_arg_get_queue_index(run_arg) );   /* Here we inform the queue system that it should pick up this job and try again. */
    run_arg_increase_submit_count( run_arg );
  }
}











static void enkf_state_clear_runpath( const enkf_state_type * enkf_state , run_arg_type * run_arg) {
  const member_config_type  * my_config   = enkf_state->my_config;
  keep_runpath_type keep_runpath          = member_config_get_keep_runpath( my_config );

  bool unlink_runpath;
  if (keep_runpath == DEFAULT_KEEP) {
    if (run_arg_get_run_mode(run_arg) == ENKF_ASSIMILATION)
      unlink_runpath = true;   /* For assimilation the default is to unlink. */
    else
      unlink_runpath = false;  /* For experiments the default is to keep the directories around. */
  } else {
    /* We have explcitly set a value for the keep_runpath variable - with either KEEP_RUNAPTH or DELETE_RUNPATH. */
    if (keep_runpath == EXPLICIT_KEEP)
      unlink_runpath = false;
    else if (keep_runpath == EXPLICIT_DELETE)
      unlink_runpath = true;
    else {
      util_abort("%s: internal error \n",__func__);
      unlink_runpath = false; /* Compiler .. */
      }
  }

  if (unlink_runpath)
    util_clear_directory(run_arg_get_runpath( run_arg ) , true , true);
}


/**
    Observe that if run_arg == false, this routine will return with
    job_completeOK == true, that might be a bit misleading.

    Observe that if an internal retry is performed, this function will
    be called several times - MUST BE REENTRANT.
*/

static bool enkf_state_complete_forward_modelOK(enkf_state_type * enkf_state , run_arg_type * run_arg) {
  const member_config_type  * my_config   = enkf_state->my_config;
  const int iens                          = member_config_get_iens( my_config );
  int result                              = 0;


  /**
     The queue system has reported that the run is OK, i.e. it has
     completed and produced the targetfile it should. We then check
     in this scope whether the results can be loaded back; if that
     is OK the final status is updated, otherwise: restart.
  */
  ert_log_add_fmt_message( 2 , NULL , "[%03d:%04d-%04d] Forward model complete - starting to load results." , iens , run_arg_get_step1(run_arg), run_arg_get_step2(run_arg));
  enkf_state_load_from_forward_model(enkf_state , run_arg , &result , false , NULL);

  if (result & REPORT_STEP_INCOMPATIBLE) {
    // If refcase has been used for observations: crash and burn.
     fprintf(stderr,"** Warning the timesteps in refcase and current simulation are not in accordance - something wrong with schedule file?\n");
     result -= REPORT_STEP_INCOMPATIBLE;
  }


  if (0 == result) {
    /*
      The loading succeded - so this is a howling success! We set
      the main status to JOB_QUEUE_ALL_OK and inform the queue layer
      about the success. In addition we set the simple status
      (should be avoided) to JOB_RUN_OK.
    */
    run_arg_set_run_status( run_arg , JOB_RUN_OK);
    ert_log_add_fmt_message( 2 , NULL , "[%03d:%04d-%04d] Results loaded successfully." , iens , run_arg_get_step1(run_arg), run_arg_get_step2(run_arg));

    enkf_state_clear_runpath( enkf_state , run_arg );
    run_arg_complete_run(run_arg);              /* free() on runpath */
  }
  return (0 == result) ? true : false;
}


bool enkf_state_complete_forward_modelOK__(void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  enkf_state_type * enkf_state = enkf_state_safe_cast( arg_pack_iget_ptr( arg_pack , 0 ));
  run_arg_type * run_arg = run_arg_safe_cast( arg_pack_iget_ptr( arg_pack , 1 ));

  return enkf_state_complete_forward_modelOK( enkf_state , run_arg);
}



static bool enkf_state_complete_forward_model_EXIT_handler__(enkf_state_type * enkf_state , run_arg_type * run_arg , bool is_retry) {
  const member_config_type  * my_config   = enkf_state->my_config;
  const int iens                          = member_config_get_iens( my_config );
  /*
     The external queue system has said that the job failed - we
     might give it another try from this scope, possibly involving a
     resampling.
   */

  if (is_retry) {
    if (run_arg_can_retry(run_arg)) {
      enkf_state_internal_retry(enkf_state, run_arg , false);
      return true;
    } else {
      return false;
    }
  } else {
    ert_log_add_fmt_message( 1, NULL, "[%03d:%04d-%04d] FAILED COMPLETELY.", iens, run_arg_get_step1(run_arg), run_arg_get_step2(run_arg));

    if (run_arg_get_run_status(run_arg) != JOB_LOAD_FAILURE)
      run_arg_set_run_status( run_arg , JOB_RUN_FAILURE);

    state_map_type * state_map = enkf_fs_get_state_map(run_arg_get_result_fs( run_arg ));
    int iens = member_config_get_iens(enkf_state->my_config);
    state_map_iset(state_map, iens, STATE_LOAD_FAILURE);
    return false;
  }
}

static bool enkf_state_complete_forward_model_EXIT_handler(void * arg, bool allow_retry ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );

  enkf_state_type * enkf_state = arg_pack_iget_ptr( arg_pack , 0 );
  run_arg_type * run_arg = arg_pack_iget_ptr( arg_pack , 1 );

  return enkf_state_complete_forward_model_EXIT_handler__( enkf_state , run_arg , allow_retry );
}


bool enkf_state_complete_forward_modelEXIT__(void * arg ) {
  return enkf_state_complete_forward_model_EXIT_handler(arg, false );
}

bool enkf_state_complete_forward_modelRETRY__(void * arg ) {
  return enkf_state_complete_forward_model_EXIT_handler(arg, true );
}


void enkf_state_invalidate_cache( enkf_state_type * enkf_state ) {
  hash_iter_type * iter       = hash_iter_alloc(enkf_state->node_hash);
  while ( !hash_iter_is_complete(iter) ) {
    enkf_node_type * node = hash_iter_get_next_value(iter);
    enkf_node_invalidate_cache( node );
  }
  hash_iter_free(iter);
}


/*****************************************************************/


rng_type * enkf_state_get_rng( const enkf_state_type * enkf_state ) {
  return enkf_state->rng;
}

unsigned int enkf_state_get_random( enkf_state_type * enkf_state ) {
  return rng_forward( enkf_state->rng );
}



void enkf_state_set_keep_runpath( enkf_state_type * enkf_state , keep_runpath_type keep_runpath) {
  member_config_set_keep_runpath( enkf_state->my_config , keep_runpath);
}


keep_runpath_type enkf_state_get_keep_runpath( const enkf_state_type * enkf_state ) {
  return member_config_get_keep_runpath( enkf_state->my_config );
}

