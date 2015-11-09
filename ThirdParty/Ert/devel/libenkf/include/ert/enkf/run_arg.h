/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'run_arg.c' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __RUN_ARG_H__
#define __RUN_ARG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_fs.h>


typedef struct run_arg_struct run_arg_type;


UTIL_SAFE_CAST_HEADER( run_arg );
UTIL_IS_INSTANCE_HEADER( run_arg );


  run_arg_type * run_arg_alloc_ENSEMBLE_EXPERIMENT(enkf_fs_type * fs , int iens , int iter , const char * runpath);
  run_arg_type * run_arg_alloc_INIT_ONLY(enkf_fs_type * init_fs , int iens , int iter , const char * runpath);
  run_arg_type * run_arg_alloc_SMOOTHER_RUN(enkf_fs_type * simulate_fs , enkf_fs_type * update_target_fs , int iens , int iter , const char * runpath);
  run_arg_type * run_arg_alloc_ENKF_ASSIMILATION(enkf_fs_type * fs ,
                                                 int iens ,
                                                 state_enum init_state_parameter ,
                                                 state_enum init_state_dynamic   ,
                                                 int step1                       ,
                                                 int step2                       ,
                                                 const char * runpath);



  state_enum     run_arg_get_dynamic_init_state( const run_arg_type * run_arg );
  state_enum     run_arg_get_parameter_init_state( const run_arg_type * run_arg );
  int            run_arg_get_parameter_init_step( const run_arg_type * run_arg );
  int            run_arg_get_step1( const run_arg_type * run_arg );
  int            run_arg_get_step2( const run_arg_type * run_arg );
  run_mode_type  run_arg_get_run_mode( const run_arg_type * run_arg );
  int            run_arg_get_load_start( const run_arg_type * run_arg );
  int            run_arg_get_iens( const run_arg_type * run_arg );
  int            run_arg_get_iter( const run_arg_type * run_arg );
  void           run_arg_increase_submit_count( run_arg_type * run_arg );
  void           run_arg_set_queue_index( run_arg_type * run_arg , int queue_index);

  void run_arg_free(run_arg_type * run_arg);
  void run_arg_free__(void * arg);
  const char * run_arg_get_runpath( const run_arg_type * run_arg);
  void run_arg_complete_run(run_arg_type * run_arg);
  run_status_type run_arg_get_run_status( const run_arg_type * run_arg );

  int  run_arg_get_queue_index( const run_arg_type * run_arg );
  bool run_arg_is_submitted( const run_arg_type * run_arg );

  bool run_arg_can_retry( const run_arg_type * run_arg );

  run_status_type run_arg_get_run_status( const run_arg_type * run_arg);
  void            run_arg_set_run_status( run_arg_type * run_arg , run_status_type run_status);

  enkf_fs_type * run_arg_get_init_fs(const run_arg_type * run_arg);
  enkf_fs_type * run_arg_get_update_target_fs(const run_arg_type * run_arg);
  enkf_fs_type * run_arg_get_result_fs(const run_arg_type * run_arg);

#ifdef __cplusplus
}
#endif
#endif
