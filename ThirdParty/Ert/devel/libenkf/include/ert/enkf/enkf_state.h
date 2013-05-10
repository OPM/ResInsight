/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_state.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_STATE_H__
#define __ENKF_STATE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/hash.h>
#include <ert/util/subst_list.h>
#include <ert/util/rng.h>
#include <ert/util/stringlist.h>
#include <ert/util/matrix.h>
#include <ert/util/log.h>

#include <ert/sched/sched_file.h>
 
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_file.h>

#include <ert/job_queue/forward_model.h>
#include <ert/job_queue/ext_joblist.h>
#include <ert/job_queue/job_queue.h>

#include <ert/enkf/model_config.h>
#include <ert/enkf/site_config.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/ert_template.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_serialize.h>

typedef struct enkf_state_struct    enkf_state_type;

  bool               enkf_state_get_pre_clear_runpath( const enkf_state_type * enkf_state );
  void               enkf_state_set_pre_clear_runpath( enkf_state_type * enkf_state , bool pre_clear_runpath );
  
  keep_runpath_type  enkf_state_get_keep_runpath( const enkf_state_type * enkf_state );
  void               enkf_state_set_keep_runpath( enkf_state_type * enkf_state , keep_runpath_type keep_runpath);
  keep_runpath_type  member_config_get_keep_runpath(const member_config_type * member_config);
  //void             * enkf_state_complete_forward_model__(void * arg );
  job_status_type    enkf_state_get_run_status( const enkf_state_type * enkf_state );
  time_t             enkf_state_get_start_time( const enkf_state_type * enkf_state );
  time_t             enkf_state_get_submit_time( const enkf_state_type * enkf_state );
  bool               enkf_state_resubmit_simulation( enkf_state_type * enkf_state , enkf_fs_type * fs , bool resample);
  bool               enkf_state_kill_simulation( const enkf_state_type * enkf_state );
  void *             enkf_state_load_from_forward_model_mt( void * arg );
  void               enkf_state_initialize(enkf_state_type * enkf_state , enkf_fs_type * fs, const stringlist_type * param_list , bool force_init);
  void               enkf_state_fread(enkf_state_type *  , enkf_fs_type * fs , int  , int  , state_enum );
  bool               enkf_state_get_analyzed(const enkf_state_type * );
  void               enkf_state_set_analyzed(enkf_state_type * , bool );
  void               enkf_state_swapout_node(const enkf_state_type * , const char *);
  void               enkf_state_swapin_node(const enkf_state_type *  , const char *);
  void               enkf_state_iset_eclpath(enkf_state_type * , int , const char *);
  enkf_node_type   * enkf_state_get_node(const enkf_state_type * , const char * );
  void               enkf_state_del_node(enkf_state_type * , const char * );
  void               enkf_state_load_ecl_summary(enkf_state_type * , bool , int );
  void             * enkf_state_run_eclipse__(void * );
  void             * enkf_state_start_forward_model__(void * );

  void enkf_state_load_from_forward_model(enkf_state_type * enkf_state , 
                                          enkf_fs_type * fs , 
                                          bool * loadOK , 
                                          bool interactive , 
                                          stringlist_type * msg_list);

  void enkf_state_forward_init(enkf_state_type * enkf_state , 
                               enkf_fs_type * fs , 
                               bool * loadOK );
    
  enkf_state_type  * enkf_state_alloc(int ,
                                      rng_type        * main_rng , 
                                      enkf_fs_type * fs, 
                                      const char * casename , 
                                      bool         pre_clear_runpath, 
                                      keep_runpath_type , 
                                      model_config_type * ,
                                      ensemble_config_type * ,
                                      const site_config_type * ,
                                      const ecl_config_type * ,
                                      log_type * logh,
                                      ert_templates_type * templates,
                                      subst_list_type    * parent_subst);
  void               enkf_state_update_node( enkf_state_type * enkf_state , const char * node_key );
  void               enkf_state_update_jobname( enkf_state_type * enkf_state );
  void               enkf_state_update_eclbase( enkf_state_type * enkf_state );
  void               enkf_state_invalidate_cache( enkf_state_type * enkf_state );
  void               enkf_state_add_node(enkf_state_type * , const char *  , const enkf_config_node_type * );
  void               enkf_state_load_ecl_restart(enkf_state_type * , bool , int );
  void               enkf_state_sample(enkf_state_type * , int);
  void               enkf_state_fwrite(const enkf_state_type *  , enkf_fs_type * fs , int  , int  , state_enum );
  void               enkf_state_ens_read(       enkf_state_type * , const char * , int);
  void               enkf_state_ecl_write(enkf_state_type *, enkf_fs_type * fs);
  void               enkf_state_free(enkf_state_type * );
  void               enkf_state_apply(enkf_state_type * , enkf_node_ftype1 * , int );
  void               enkf_state_serialize(enkf_state_type * , size_t);
  void               enkf_state_set_iens(enkf_state_type *  , int );
  int                enkf_state_get_iens(const enkf_state_type * );
  member_config_type *enkf_state_get_member_config(const enkf_state_type * enkf_state);
  const char       * enkf_state_get_run_path(const enkf_state_type * );
  const char       * enkf_state_get_eclbase( const enkf_state_type * enkf_state );
  void               enkf_state_printf_subst_list(enkf_state_type * enkf_state , int step1 , int step2);

  rng_type         * enkf_state_get_rng( const enkf_state_type * enkf_state );
  unsigned int       enkf_state_get_random( enkf_state_type * enkf_state );

/*****************************************************************/
  void enkf_state_set_inactive(enkf_state_type * state);
  
  void enkf_state_init_run(enkf_state_type * state , 
                           run_mode_type           ,  
                           bool active             , 
                           int max_internal_submit , 
                           int init_step_parameter , 
                           state_enum init_state_parameter , 
                           state_enum init_state_dynamic , 
                           int load_start , 
                           int step1 , 
                           int step2 );
  int enkf_state_get_queue_index(const enkf_state_type * enkf_state);
  
  
  run_status_type enkf_state_get_simple_run_status(const enkf_state_type * state);
#ifdef __cplusplus
}
#endif
#endif
