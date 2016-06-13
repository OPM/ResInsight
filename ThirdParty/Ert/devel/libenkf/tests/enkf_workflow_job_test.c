/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_workflow_job_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/enkf/ert_test_context.h>

#include <ert/util/util.h>
#include <ert/util/string_util.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_main_jobs.h>


ert_test_context_type * create_context( const char * config_file, const char * name ) {
  ert_test_context_type * test_context = ert_test_context_alloc(name , config_file);
  test_assert_not_NULL(test_context);
  return test_context;
}

void test_create_case_job(ert_test_context_type * test_context, const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  stringlist_append_copy( args , "newly_created_case");
  test_assert_true( ert_test_context_install_workflow_job( test_context , job_name , job_file ));
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  char * new_case = util_alloc_filename( "storage" , "newly_created_case" , NULL);
  test_assert_true(util_is_directory(new_case));
  free(new_case);

  stringlist_free( args );
}


void test_init_case_job(ert_test_context_type * test_context, const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  enkf_main_type * enkf_main = ert_test_context_get_main(test_context);

  test_assert_true( ert_test_context_install_workflow_job( test_context , "JOB" , job_file ) );

  //Test init current case from existing
  {
    enkf_fs_type * cur_fs = enkf_main_mount_alt_fs( enkf_main , "new_current_case" , true );
    enkf_main_select_fs(enkf_main, "new_current_case");

    test_assert_ptr_not_equal(cur_fs , enkf_main_get_fs( enkf_main ));

    stringlist_append_copy( args, "default"); //case to init from
    test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );

    enkf_fs_decref(cur_fs);
  }

  {
    const char * current_case = enkf_main_get_current_fs( enkf_main );
    test_assert_string_equal(current_case, "new_current_case");
    test_assert_true(enkf_fs_has_node(enkf_main_get_fs(enkf_main), "PERMZ", PARAMETER, 0, 0));  // This had state = ANALYZED; might be unfixable.

    enkf_fs_type * default_fs          = enkf_main_mount_alt_fs( enkf_main , "default" , true  );
    state_map_type * default_state_map = enkf_fs_get_state_map(default_fs);
    state_map_type * current_state_map = enkf_fs_get_state_map(enkf_main_get_fs(enkf_main));
    test_assert_int_equal(state_map_get_size(default_state_map), state_map_get_size(current_state_map));
    enkf_fs_decref(default_fs);
  }


  //Test init case from existing case:
  stringlist_clear(args);
  stringlist_append_copy(args, "default"); //case to init from
  stringlist_append_copy(args, "new_not_current_case");
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );
  {
    enkf_fs_type * fs = enkf_main_mount_alt_fs(enkf_main, "new_not_current_case", true);
    test_assert_not_NULL( fs );
    test_assert_true( enkf_fs_has_node(fs, "PERMZ", PARAMETER, 0, 0));  // This had state = ANALYZED; might be unfixable.

    enkf_fs_type * default_fs          = enkf_main_mount_alt_fs( enkf_main , "default" , true );
    state_map_type * default_state_map = enkf_fs_get_state_map(default_fs);
    state_map_type * new_state_map     = enkf_fs_get_state_map(fs);
    test_assert_int_equal(state_map_get_size(default_state_map), state_map_get_size(new_state_map));
    enkf_fs_decref(fs);
  }

  stringlist_free( args );
}


void test_load_results_job(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );
  stringlist_append_copy( args , "0");
  stringlist_append_copy( args , ",");
  stringlist_append_copy( args , "1");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
  stringlist_free( args );
}


void test_load_results_iter_job(ert_test_context_type * test_context , const char * job_name , const char * job_file) {

  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );
  stringlist_append_copy( args , "0");
  stringlist_append_copy( args , "0");
  stringlist_append_copy( args , ",");
  stringlist_append_copy( args , "1");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
  stringlist_free( args );
}


void test_rank_realizations_on_observations_job(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );

  stringlist_append_copy( args , "NameOfObsRanking1");
  stringlist_append_copy( args , "|");
  stringlist_append_copy( args , "WOPR:*");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking2");
  stringlist_append_copy( args, "1-5");
  stringlist_append_copy( args, "55");
  stringlist_append_copy( args , "|");
  stringlist_append_copy( args , "WWCT:*");
  stringlist_append_copy( args , "WOPR:*");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking3");
  stringlist_append_copy( args, "5");
  stringlist_append_copy( args, "55");
  stringlist_append_copy( args, "|");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking4");
  stringlist_append_copy( args, "1,3,5-10");
  stringlist_append_copy( args, "55");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking5");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking6");
  stringlist_append_copy( args, "|");
  stringlist_append_copy( args , "UnrecognizableObservation");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_free( args );
}


void test_rank_realizations_on_data_job(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );

  stringlist_append_copy( args , "NameOfDataRanking");
  stringlist_append_copy( args , "PORO:1,2,3");
  stringlist_append_copy( args , "false");
  stringlist_append_copy( args , "0");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfDataRanking2");
  stringlist_append_copy( args , "PORO:1,2,3");
  stringlist_append_copy( args , "false");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_free( args );
}

void test_export_ranking(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );

  stringlist_append_copy( args , "NameOfDataRanking");
  stringlist_append_copy( args , "/tmp/fileToSaveDataRankingIn.txt");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking1");
  stringlist_append_copy( args , "/tmp/fileToSaveObservationRankingIn1.txt");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_clear(args);
  stringlist_append_copy( args , "NameOfObsRanking6");
  stringlist_append_copy( args , "/tmp/fileToSaveObservationRankingIn6.txt");
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  stringlist_free( args );
}


void test_init_misfit_table(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  stringlist_type * args = stringlist_alloc_new();
  ert_test_context_install_workflow_job( test_context , job_name , job_file );

  enkf_main_type * enkf_main = ert_test_context_get_main(test_context);
  enkf_fs_type               * fs              = enkf_main_get_fs(enkf_main);

  misfit_ensemble_type * misfit_ensemble = enkf_fs_get_misfit_ensemble( fs );
  test_assert_false(misfit_ensemble_initialized(misfit_ensemble));

  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  test_assert_true(misfit_ensemble_initialized(misfit_ensemble));

  stringlist_free( args );
}




static void test_export_runpath_file(ert_test_context_type * test_context,
                                    const char * job_name,
                                    const char * job_file,
                                    stringlist_type * args,
                                    int_vector_type * iens_values,
                                    int_vector_type * iter_values) {

  ert_test_context_install_workflow_job( test_context , job_name , job_file );
  test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );

  {
    const enkf_main_type * enkf_main = ert_test_context_get_main(test_context);
    hook_manager_type * hook_manager       = enkf_main_get_hook_manager( enkf_main );
    const char * runpath_file_name   = hook_manager_get_runpath_list_file(hook_manager);

    ecl_config_type * ecl_config            = enkf_main_get_ecl_config(enkf_main);
    const model_config_type * model_config  = enkf_main_get_model_config(enkf_main);
    const char * base_fmt                   = ecl_config_get_eclbase(ecl_config);
    const char * runpath_fmt                = model_config_get_runpath_as_char(model_config);

    test_assert_true(util_file_exists(runpath_file_name));
    FILE * file = util_fopen(runpath_file_name, "r");

    int file_iens = 0;
    char file_path[256];
    char file_base[256];
    int file_iter = 0;
    char * cwd = util_alloc_cwd();
    int counter = 0;
    int iens_index = 0;
    int iter_index = 0;

    while (4 == fscanf( file , "%d %s %s %d" , &file_iens , file_path , file_base, &file_iter)) {
      ++ counter;

      test_assert_true(int_vector_size(iens_values) >= iens_index+1);
      test_assert_true(int_vector_size(iter_values) >= iter_index+1);

      int iens = int_vector_iget(iens_values, iens_index);
      int iter = int_vector_iget(iter_values, iter_index);

      test_assert_int_equal(file_iens, iens);
      test_assert_int_equal(file_iter, iter);

      char * base = util_alloc_sprintf("--%d", iens);
      if (base_fmt && (util_int_format_count(base_fmt) == 1))
        base = util_alloc_sprintf(base_fmt, iens);

      test_assert_string_equal(base, file_base);

      char * runpath = "";
      if (util_int_format_count(runpath_fmt) == 1)
        runpath = util_alloc_sprintf(runpath_fmt, iens);
      else if (util_int_format_count(runpath_fmt) == 2)
        runpath = util_alloc_sprintf(runpath_fmt, iens,iter);

      test_assert_string_equal(runpath, file_path);

      if (iens_index+1 < int_vector_size(iens_values))
        ++iens_index;
      else if ((iens_index+1 == int_vector_size(iens_values))) {
        ++iter_index;
        iens_index = 0;
      }

      free(base);
      free(runpath);
    }

    int linecount = int_vector_size(iens_values) * int_vector_size(iter_values);
    test_assert_int_equal(linecount, counter);
    free(cwd);
    fclose(file);
  }
}



void test_export_runpath_files(const char * config_file,
                               const char * config_file_iterations,
                               const char * job_file_export_runpath) {

  stringlist_type * args = stringlist_alloc_new();
  const char * job_name  = "export_job";

  ert_test_context_type * test_context_iterations = create_context( config_file_iterations, "enkf_workflow_job_test_export_runpath_iter" );

  {
    int_vector_type * iens_values = int_vector_alloc(5,0);
    const int iens[5] = {0,1,2,3,4};
    int_vector_set_many(iens_values, 0, &iens[0], 5);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context_iterations, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);
  }
  {
    stringlist_append_copy( args, "0-2"); //realization range

    int_vector_type * iens_values = int_vector_alloc(3,0);
    const int iens[] = {0,1,2};
    int_vector_set_many(iens_values, 0, &iens[0], 3);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context_iterations, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }
  {
    stringlist_append_copy( args, "0,3-5"); //realization range

    int_vector_type * iens_values = int_vector_alloc(4,0);
    const int iens[] = {0,3,4,5};
    int_vector_set_many(iens_values, 0, &iens[0], 4);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context_iterations, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }
  {
    stringlist_append_copy( args, "1-2"); //realization range
    stringlist_append_copy( args, "|");   //delimiter
    stringlist_append_copy( args, "1-3"); //iteration range

    int_vector_type * iens_values = int_vector_alloc(2,0);
    int iens[] = {1,2};
    int_vector_set_many(iens_values, 0, &iens[0], 2);
    int_vector_type * iter_values = int_vector_alloc(3,0);
    int iter[] = {1,2,3};
    int_vector_set_many(iter_values, 0, &iter[0], 3);

    test_export_runpath_file(test_context_iterations, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }
  {
    stringlist_append_copy( args, "*");   //realization range
    stringlist_append_copy( args, "|");   //delimiter
    stringlist_append_copy( args, "*");   //iteration range

    int_vector_type * iens_values = int_vector_alloc(5,0);
    int iens[] = {0,1,2,3,4};
    int_vector_set_many(iens_values, 0, &iens[0], 5);
    int_vector_type * iter_values = int_vector_alloc(4,0);
    int iter[] = {0,1,2,3};
    int_vector_set_many(iter_values, 0, &iter[0], 4);

    test_export_runpath_file(test_context_iterations, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }

  ert_test_context_free(test_context_iterations);
  ert_test_context_type * test_context = create_context( config_file, "enkf_workflow_job_test_export_runpath" );

  {
    int_vector_type * iens_values = int_vector_alloc(1,0);
    int_vector_init_range(iens_values, 0, 25, 1);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }
  {
    stringlist_append_copy( args, "1-3"); //realization range

    int_vector_type * iens_values = int_vector_alloc(3,0);
    int iens[] = {1,2,3};
    int_vector_set_many(iens_values, 0, &iens[0], 3);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }
  {
    stringlist_append_copy( args, "1,2"); //realization range
    stringlist_append_copy( args, "|");   //delimiter
    stringlist_append_copy( args, "1-3"); //iteration range

    int_vector_type * iens_values = int_vector_alloc(2,0);
    int iens[] = {1,2};
    int_vector_set_many(iens_values, 0, &iens[0], 2);
    int_vector_type * iter_values = int_vector_alloc(1,0);

    test_export_runpath_file(test_context, job_name, job_file_export_runpath, args, iens_values, iter_values);

    int_vector_free(iens_values);
    int_vector_free(iter_values);

    stringlist_clear(args);
  }


  ert_test_context_free(test_context);


  stringlist_free( args );
}





int main(int argc , const char ** argv) {
  enkf_main_install_SIGNALS();

  const char * config_file                  = argv[1];
  const char * config_file_iterations       = argv[2];
  const char * job_file_create_case         = argv[3];
  const char * job_file_init_case_job       = argv[4];
  const char * job_file_load_results        = argv[5];
  const char * job_file_load_results_iter   = argv[6];
  const char * job_file_observation_ranking = argv[7];
  const char * job_file_data_ranking        = argv[8];
  const char * job_file_ranking_export      = argv[9];
  const char * job_file_init_misfit_table   = argv[10];
  const char * job_file_export_runpath      = argv[11];


  ert_test_context_type * test_context = create_context( config_file, "enkf_workflow_job_test" );
  {
    test_create_case_job(test_context, "JOB1" , job_file_create_case);
    test_init_case_job(test_context, "JOB2", job_file_init_case_job);
    test_load_results_job(test_context, "JOB3" , job_file_load_results);
    test_load_results_iter_job( test_context, "JOB4" , job_file_load_results_iter );
    test_init_misfit_table(test_context, "JOB5" , job_file_init_misfit_table);
    test_rank_realizations_on_observations_job(test_context, "JOB6" , job_file_observation_ranking);
    test_rank_realizations_on_data_job(test_context , "JOB7" , job_file_data_ranking);
    test_export_ranking(test_context, "JOB8" , job_file_ranking_export);
  }
  ert_test_context_free( test_context );

  test_export_runpath_files(config_file, config_file_iterations, job_file_export_runpath);

  exit(0);
}
