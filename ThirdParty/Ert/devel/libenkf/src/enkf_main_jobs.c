/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'enkf_main_jobs.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <unistd.h>

#include <ert/util/stringlist.h>
#include <ert/util/string_util.h>
#include <ert/util/int_vector.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/local_obsdata.h>
#include <ert/enkf/local_obsdata_node.h>



static bool_vector_type * alloc_iactive_vector_from_range(const stringlist_type * range, int startindex, int endindex, int ens_size) {
  bool_vector_type * iactive;
  int range_list_size = stringlist_get_size(range);
  if ((range_list_size > startindex) && (range_list_size >= endindex)) {
    char * arg_string = stringlist_alloc_joined_substring( range, startindex, endindex, "");
    iactive = bool_vector_alloc(ens_size, false);
    string_util_update_active_mask( arg_string, iactive );
    free ( arg_string );
  } else {
    iactive = bool_vector_alloc(ens_size, true);
  }
  return iactive;
}

void enkf_main_jobs_sleep(){
    while(true){
        printf("Hi from internal \n");
        sleep(1);
    }
}

void * enkf_main_exit_JOB(void * self , const stringlist_type * args ) {
  enkf_main_type  * enkf_main = enkf_main_safe_cast( self );
  enkf_main_exit( enkf_main );
  return NULL;
}


/**
   This job is purely added to make sure we have a job in the
   internal-gui/config subdirectory. As soon as there is a real job
   there; this can be removed.
*/

void * enkf_main_dummy_JOB(void * self , const stringlist_type * args ) {
  return NULL;
}





void * enkf_main_ensemble_run_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = alloc_iactive_vector_from_range(args, 0, stringlist_get_size(args), ens_size);

  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_exp( enkf_main , iactive);
  bool_vector_free(iactive);
  return NULL;
}


static void * enkf_main_smoother_JOB__( void * self , int iter , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive   = bool_vector_alloc( ens_size , true );
  bool valid                   = true;
  const char * target_case     = stringlist_iget( args , 0 );
  enkf_fs_type * source_fs     = enkf_main_job_get_fs( enkf_main );
  //Argument 2: Rerun. Default false.
  bool rerun = (stringlist_get_size(args) >= 2) ? stringlist_iget_as_bool(args, 1, &valid) : false;

  if (!valid) {
      fprintf(stderr, "** Warning: Function %s : Second argument must be a bool value. Exiting job\n", __func__);
      return NULL;
  }
  enkf_main_run_smoother( enkf_main , source_fs , target_case , iactive , iter , rerun);
  bool_vector_free( iactive );
  return NULL;
}


void * enkf_main_smoother_JOB( void * self , const stringlist_type * args ) {
  return enkf_main_smoother_JOB__( self, 0 , args );
}


void * enkf_main_smoother_with_iter_JOB( void * self , const stringlist_type * args ) {
  int iter;
  stringlist_type * sub_args = stringlist_alloc_shallow_copy_with_limits( args , 1 , stringlist_get_size( args ) - 1);
  util_sscanf_int( stringlist_iget(args , 0 ) , &iter );

  enkf_main_smoother_JOB__( self , iter , sub_args );

  stringlist_free( sub_args );
  return NULL;
}



void * enkf_main_iterated_smoother_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  const analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);
  analysis_iter_config_type * iter_config = analysis_config_get_iter_config(analysis_config);
  int num_iter = analysis_iter_config_get_num_iterations(iter_config);

  enkf_main_run_iterated_ES( enkf_main , num_iter);
  return NULL;
}


void * enkf_main_select_module_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );

  analysis_config_select_module( analysis_config , stringlist_iget( args , 0 ));

  return NULL;
}


void * enkf_main_scale_obs_std_JOB(void * self, const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );

  double scale_factor;
  if (util_sscanf_double(stringlist_iget(args, 0), &scale_factor)) {
    analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
    analysis_config_set_global_std_scaling( analysis_config , scale_factor );
  }
  return NULL;
}

/*****************************************************************/

/*
   Will create the new case if it does not exist.
*/
void * enkf_main_select_case_JOB( void * self , const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  const char * new_case = stringlist_iget( args , 0 );
  enkf_main_select_fs( enkf_main , new_case );
  return NULL;
}


void * enkf_main_create_case_JOB( void * self , const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  const char * new_case = stringlist_iget( args , 0 );
  enkf_fs_type * fs = enkf_main_mount_alt_fs( enkf_main , new_case , true );
  enkf_fs_decref( fs );
  return NULL;
}



void * enkf_main_init_case_from_existing_JOB( void * self , const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );

  const char * source_case = stringlist_iget( args , 0 );
  enkf_fs_type * source_fs = enkf_main_mount_alt_fs( enkf_main , source_case , true );
  {
    enkf_fs_type * target_fs;

    if (stringlist_get_size(args) > 1) {
      const char * current_case = enkf_main_get_current_fs(enkf_main);
      const char * target_case = stringlist_iget( args , 1 );
      if (0 != strcmp(current_case, target_case)) {
        target_fs = enkf_main_mount_alt_fs( enkf_main , target_case , true );
      } else
        target_fs = enkf_fs_get_ref( enkf_main_job_get_fs(enkf_main) );  // Using get_ref so that we can unconditionally call decref() further down.
    } else
      target_fs = enkf_fs_get_ref( enkf_main_job_get_fs(enkf_main) );    // Using get_ref so that we can unconditionally call decref() further down.

    enkf_main_init_case_from_existing(enkf_main, source_fs, 0, target_fs); // Removed ANALYZED argument
    enkf_fs_decref(target_fs);
  }
  enkf_fs_decref(source_fs);

  return NULL;
}


/*****************************************************************/

static void * enkf_main_load_results_JOB__( enkf_main_type * enkf_main , int iter , const stringlist_type * args) {
  bool_vector_type * iactive = alloc_iactive_vector_from_range(args, 0, stringlist_get_size(args), enkf_main_get_ensemble_size(enkf_main));
  int ens_size = enkf_main_get_ensemble_size(enkf_main);
  stringlist_type ** realizations_msg_list = util_calloc(ens_size, sizeof * realizations_msg_list);
  for (int iens = 0; iens < ens_size; ++iens)
    realizations_msg_list[iens] = stringlist_alloc_new();

  enkf_main_load_from_forward_model(enkf_main, iter , iactive, realizations_msg_list);

  for (int iens = 0; iens < ens_size; ++iens) {
    stringlist_type * msg = realizations_msg_list[iens];
    if (stringlist_get_size(msg)) {
      int msg_count = 0;
      for (; msg_count < stringlist_get_size(msg); ++msg_count)
        fprintf(stderr, "** Warning: Function %s : Load of realization number %d returned the following warning: %s\n", __func__, iens, stringlist_iget(msg, msg_count));
    }
    stringlist_free(msg);
  }

  free(realizations_msg_list);
  bool_vector_free(iactive);
  return NULL;
}


void * enkf_main_load_results_JOB( void * self , const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  int iter = 0;
  {
    const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
    if (model_config_runpath_requires_iter( model_config ))
      fprintf(stderr,"**Warning: the runpath format:%s requires an iteration number - using default:0. Use the job: LOAD_RESULT_ITER instead.\n" , model_config_get_runpath_as_char( model_config ));
  }
  return enkf_main_load_results_JOB__(enkf_main , iter , args );
}


void * enkf_main_load_results_iter_JOB( void * self , const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  stringlist_type * iens_args = stringlist_alloc_shallow_copy_with_limits( args , 1 , stringlist_get_size( args ) - 1);
  int iter;

  util_sscanf_int( stringlist_iget( args , 0 ) , &iter);
  enkf_main_load_results_JOB__(enkf_main , iter , iens_args );
  stringlist_free( iens_args );

  return NULL;
}


/*****************************************************************/

static void enkf_main_jobs_export_field(const enkf_main_type * enkf_main, const stringlist_type * args, field_file_format_type file_type) {
  const char *      field            = stringlist_iget(args, 0);
  const char *      file_name        = stringlist_iget(args, 1);
  int               report_step      = 0;
  util_sscanf_int(stringlist_iget(args,2), &report_step);

  bool_vector_type * iactive = alloc_iactive_vector_from_range(args, 4, stringlist_get_size(args), enkf_main_get_ensemble_size(enkf_main));
  enkf_main_export_field(enkf_main,field, file_name, iactive, file_type, report_step ) ;
  bool_vector_free(iactive);
}



void * enkf_main_export_field_JOB(void * self, const stringlist_type * args) {
  const char * file_name = stringlist_iget(args, 1);
  field_file_format_type file_type = field_config_default_export_format(file_name);

  if ((RMS_ROFF_FILE == file_type) || (ECL_GRDECL_FILE == file_type)) {
    enkf_main_type * enkf_main = enkf_main_safe_cast( self );
    enkf_main_jobs_export_field(enkf_main, args, file_type);
  } else
    printf("EXPORT_FIELD filename argument: File extension must be either .roff or .grdecl\n");

  return NULL;
}

void * enkf_main_export_field_to_RMS_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  enkf_main_jobs_export_field(enkf_main, args, RMS_ROFF_FILE);
  return NULL;
}

void * enkf_main_export_field_to_ECL_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  enkf_main_jobs_export_field(enkf_main, args, ECL_GRDECL_FILE);
  return NULL;
}


/*****************************************************************/

void * enkf_main_rank_on_observations_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main  = enkf_main_safe_cast( self );
  const char * ranking_name   = stringlist_iget(args, 0);

  bool step_arguments = false;
  bool obs_arguments  = false;
  int  delimiter      = 0;
  {
    delimiter = stringlist_find_first(args, "|");
    if (delimiter > -1) {
      step_arguments = (delimiter > 1) ? true : false;
      obs_arguments  = (stringlist_get_size(args) > delimiter + 1) ? true : false;
    } else if (stringlist_get_size(args) > 1) {
        step_arguments = true;
        delimiter     = stringlist_get_size(args);
    }
  }

  int_vector_type * steps_vector = NULL;
  {
    char * report_steps = NULL;

    if (step_arguments)
      report_steps = stringlist_alloc_joined_substring(args, 1, delimiter, ",");
    else
      report_steps = util_alloc_sprintf("0-%d", enkf_main_get_history_length(enkf_main));

    steps_vector = string_util_alloc_value_list(report_steps);

    free(report_steps);
  }


  stringlist_type * obs_ranking_keys = NULL;
  {
    char * obs_key_char = NULL;
    if (obs_arguments)
      obs_key_char = stringlist_alloc_joined_substring( args , delimiter+1 , stringlist_get_size(args) , " ");

    enkf_obs_type * enkf_obs = enkf_main_get_obs(enkf_main);
    obs_ranking_keys = enkf_obs_alloc_matching_keylist( enkf_obs , obs_key_char );

    if ((obs_arguments) && (stringlist_get_size(obs_ranking_keys) == 0)) {
      fprintf(stderr,"The input string : \"%s\" did not resolve to any valid observation keys. Job not started\n", obs_key_char);
      return NULL;
    }

    if (obs_arguments)
      free(obs_key_char);
  }


  enkf_main_rank_on_observations(enkf_main, ranking_name, obs_ranking_keys, steps_vector);

  stringlist_free(obs_ranking_keys);
  int_vector_free(steps_vector);
  return NULL;
}


void * enkf_main_rank_on_data_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  const char * ranking_name  = stringlist_iget(args, 0);
  const char * data_key      = stringlist_iget(args, 1);
  bool valid = true;
  bool sort_increasing       = stringlist_iget_as_bool(args, 2, &valid);

  if (!valid) {
    fprintf(stderr,"** Third argument \"sort increasing\" not recognized as bool value, job not started\n");
    return NULL;
  }

  int report_step = (stringlist_get_size(args) > 3) ? stringlist_iget_as_int(args, 3, &valid) : enkf_main_get_history_length(enkf_main) ;
  if (!valid) {
    fprintf(stderr,"** Fourth argument \"step\" not recognized as integer value, job not started\n");
    return NULL;
  }

  if (report_step < 0) {
    fprintf(stderr,"** Negative report step, job not started\n");
    return NULL;
  }

  enkf_main_rank_on_data(enkf_main, ranking_name, data_key, sort_increasing, report_step);
  return NULL;
}


void * enkf_main_export_ranking_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  const char * ranking_name  = stringlist_iget(args, 0);
  const char * ranking_file  = stringlist_iget(args, 1);

  enkf_main_export_ranking(enkf_main, ranking_name, ranking_file);
  return NULL;
}

void * enkf_main_init_misfit_table_JOB(void * self, const stringlist_type * args) {
  enkf_main_type * enkf_main   = enkf_main_safe_cast( self );
  int history_length           = enkf_main_get_history_length(enkf_main);
  enkf_obs_type * enkf_obs     = enkf_main_get_obs(enkf_main);
  int ens_size                 = enkf_main_get_ensemble_size(enkf_main);
  enkf_fs_type * fs            = enkf_main_job_get_fs(enkf_main);
  bool force_update            = true;
  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config(enkf_main);


  misfit_ensemble_type * misfit_ensemble = enkf_fs_get_misfit_ensemble( fs );
  misfit_ensemble_initialize( misfit_ensemble , ensemble_config , enkf_obs , fs , ens_size , history_length, force_update);

  return NULL;
}




static void enkf_main_export_runpath_file(enkf_main_type * enkf_main,
                                          const int_vector_type * realizations,
                                          const int_vector_type * iterations) {

  ecl_config_type * ecl_config            = enkf_main_get_ecl_config(enkf_main);
  const model_config_type * model_config  = enkf_main_get_model_config(enkf_main);
  const char * basename_fmt               = ecl_config_get_eclbase(ecl_config);
  const char * runpath_fmt                = model_config_get_runpath_as_char(model_config);
  const hook_manager_type * hook_manager        = enkf_main_get_hook_manager( enkf_main );

  runpath_list_type * runpath_list = runpath_list_alloc( hook_manager_get_runpath_list_file( hook_manager ));

  for (int iter = 0; iter < int_vector_size(iterations); ++iter) {
    for (int iens = 0; iens < int_vector_size(realizations); ++iens) {
      int iter_value = int_vector_iget(iterations, iter);
      int iens_value = int_vector_iget(realizations, iens);
      char * basename;
      char * runpath;

      if (basename_fmt)
        basename = util_alloc_sprintf(basename_fmt, iens_value);
      else
        basename = util_alloc_sprintf("--%d", iens_value);

      if (model_config_runpath_requires_iter(model_config))
        runpath = util_alloc_sprintf(runpath_fmt, iens_value, iter_value);
      else
        runpath = util_alloc_sprintf(runpath_fmt, iens_value);

      runpath_list_add(runpath_list, iens_value, iter_value, runpath, basename);

      free(basename);
      free(runpath);
    }
  }
  runpath_list_fprintf(runpath_list);
  runpath_list_free(runpath_list);
}




void * enkf_main_export_runpath_file_JOB(void * self, const stringlist_type * args)  {
  enkf_main_type * enkf_main              = enkf_main_safe_cast( self );
  int ensemble_size                       = enkf_main_get_ensemble_size(enkf_main);
  analysis_config_type * analysis_config  = enkf_main_get_analysis_config(enkf_main);
  analysis_iter_config_type * iter_config = analysis_config_get_iter_config(analysis_config);
  int num_iterations                      = analysis_iter_config_get_num_iterations(iter_config);
  const model_config_type * model_config  = enkf_main_get_model_config(enkf_main);
  int_vector_type * realizations          = int_vector_alloc(1, 0);
  int_vector_init_range(realizations, 0, ensemble_size, 1);
  int_vector_type * iterations            = int_vector_alloc(1, 0);


  if (stringlist_get_size(args) > 0) {
    int offset = 0;
    while (true) {
      if (offset == stringlist_get_size( args ))
        break;
      if (0 == strcmp("|" , stringlist_iget( args, offset )))
        break;
       ++offset;
    }

    if (0 != strcmp("*", stringlist_iget(args,0))) {
      char * range_str = stringlist_alloc_joined_substring( args, 0, offset, "");
      string_util_init_value_list(range_str, realizations);
      free(range_str);
    }

    if ((offset < stringlist_get_size(args)) && model_config_runpath_requires_iter(model_config)) {
      if (0 == strcmp("*", stringlist_iget(args, (offset+1))))
        int_vector_init_range(iterations, 0, num_iterations, 1);
      else {
        char * range_str = stringlist_alloc_joined_substring( args, offset+1, stringlist_get_size(args), "");
        string_util_init_value_list(range_str, iterations);
        free(range_str);
       }
    }
  }

  enkf_main_export_runpath_file(enkf_main, realizations, iterations);

  int_vector_free(realizations);
  int_vector_free(iterations);

  return NULL;
}



void * enkf_main_std_scale_correlated_obs_JOB(void * self, const stringlist_type * args)  {

  if (stringlist_get_size(args) > 0) {
    enkf_main_type * enkf_main              = enkf_main_safe_cast( self );
    int ensemble_size                       = enkf_main_get_ensemble_size(enkf_main);
    enkf_fs_type * fs                       = enkf_main_job_get_fs( enkf_main );
    enkf_obs_type * obs                     = enkf_main_get_obs( enkf_main );
    int_vector_type * realizations          = int_vector_alloc(1, 0);
    local_obsdata_type * obsdata = local_obsdata_alloc( "OBS-JOB" );

    int_vector_init_range(realizations, 0, ensemble_size, 1);

    for (int iarg = 0; iarg < stringlist_get_size(args); iarg++) {
      const char * arg_key = stringlist_iget( args , iarg );
      stringlist_type * key_list = enkf_obs_alloc_matching_keylist(obs, arg_key);
      for (int iobs=0; iobs < stringlist_get_size( key_list ); iobs++) {
        const char * obs_key = stringlist_iget( key_list , iobs);
        const obs_vector_type * obs_vector = enkf_obs_get_vector(obs, obs_key);
        local_obsdata_add_node( obsdata , obs_vector_alloc_local_node(obs_vector) );
      }
      stringlist_free( key_list );
    }

    if (local_obsdata_get_size(obsdata) > 0)
      enkf_obs_scale_correlated_std(obs, fs, realizations, obsdata );

    local_obsdata_free( obsdata );
  }

  return NULL;
}




