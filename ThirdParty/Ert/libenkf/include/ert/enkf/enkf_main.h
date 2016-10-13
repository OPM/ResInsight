/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'enkf_main.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ENKF_MAIN_H
#define ERT_ENKF_MAIN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>
#include <ert/util/set.h>
#include <ert/util/subst_list.h>
#include <ert/util/bool_vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/matrix.h>
#include <ert/util/path_fmt.h>
#include <ert/util/ui_return.h>
#include <ert/util/log.h>

#include <ert/sched/sched_file.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/ext_joblist.h>
#include <ert/job_queue/forward_model.h>

#include <ert/enkf/plot_config.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/misfit_ensemble.h>
#include <ert/enkf/analysis_config.h>
#include <ert/enkf/site_config.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/ert_template.h>
#include <ert/enkf/enkf_plot_data.h>
#include <ert/enkf/ranking_table.h>
#include <ert/enkf/hook_manager.h>
#include <ert/enkf/rng_config.h>
#include <ert/enkf/pca_plot_data.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/ert_run_context.h>
#include <ert/enkf/ert_init_context.h>

  /*****************************************************************/

  typedef struct enkf_main_struct enkf_main_type;
  ui_return_type *              enkf_main_set_eclbase( enkf_main_type * enkf_main , const char * eclbase_fmt);
  ui_return_type *              enkf_main_set_data_file( enkf_main_type * enkf_main , const char * data_file );
  void                          enkf_main_set_user_config_file( enkf_main_type * enkf_main , const char * user_config_file );
  const char                  * enkf_main_get_user_config_file( const enkf_main_type * enkf_main );
  void                          enkf_main_set_rft_config_file( enkf_main_type * enkf_main , const char * rft_config_file );
  const char                  * enkf_main_get_rft_config_file( const enkf_main_type * enkf_main );
  bool                          enkf_main_get_pre_clear_runpath( const enkf_main_type * enkf_main );
  void                          enkf_main_set_pre_clear_runpath( enkf_main_type * enkf_main , bool pre_clear_runpath);
  bool                          enkf_main_set_refcase( enkf_main_type * enkf_main , const char * refcase_path);
  ui_return_type              * enkf_main_validata_refcase( const enkf_main_type * enkf_main , const char * refcase_path);

  ert_templates_type          * enkf_main_get_templates( enkf_main_type * enkf_main );
  void                          enkf_main_set_log_file( enkf_main_type * enkf_main , const char * log_file );
  const char                  * enkf_main_get_log_file( const enkf_main_type * enkf_main );
  void                          enkf_main_set_log_level( enkf_main_type * enkf_main , int log_level );
  int                           enkf_main_get_log_level( const enkf_main_type * enkf_main );

  member_config_type          * enkf_main_iget_member_config(const enkf_main_type * enkf_main , int iens);
  void                          enkf_main_del_unused_static(enkf_main_type * , int );
  const char                  * enkf_main_get_data_file(const enkf_main_type * );
  const char                 ** enkf_main_get_well_list_ref(const enkf_main_type * , int *);


  bool                          enkf_main_get_endian_swap(const enkf_main_type * );
  bool                          enkf_main_get_fmt_file(const enkf_main_type * );
  bool                          enkf_main_has_key(const enkf_main_type * , const char *);
  void                          enkf_main_add_gen_kw(enkf_main_type * , const char * );
  void                          enkf_main_add_type(enkf_main_type * , const char * , enkf_var_type , ert_impl_type , const char * , const void *);
  void                          enkf_main_add_type0(enkf_main_type * , const char * , int , enkf_var_type , ert_impl_type );
  void                          enkf_main_add_well(enkf_main_type * , const char * , int , const char ** );
  void                          enkf_main_analysis(enkf_main_type * );
  void                          enkf_main_free(enkf_main_type * );
  void                          enkf_main_exit(enkf_main_type * enkf_main);
  void                          enkf_main_init_eclipse(enkf_main_type * , int , int );
  void                          enkf_main_init_run( enkf_main_type * enkf_main, const ert_run_context_type * run_context , init_mode_type init_mode);
  void                          enkf_main_load_ecl_init_mt(enkf_main_type * enkf_main , int );
  void                          enkf_main_load_ecl_complete_mt(enkf_main_type *);
  void                          enkf_main_iload_ecl_mt(enkf_main_type *enkf_main , int );

  bool                          enkf_main_UPDATE(enkf_main_type * enkf_main , const int_vector_type * step_list, enkf_fs_type * source_fs, enkf_fs_type * target_fs , int target_step , run_mode_type run_mode);
  bool                          enkf_main_smoother_update(enkf_main_type * enkf_main , enkf_fs_type * source_fs, enkf_fs_type * target_fs);
  void                          enkf_main_create_run_path(enkf_main_type * enkf_main , const bool_vector_type * iactive , int iter);
  int                           enkf_main_run_simple_step( enkf_main_type* enkf_main, bool_vector_type* iactive, init_mode_type init_mode, int iter );

  void                          enkf_main_run_tui_exp(enkf_main_type * enkf_main ,
                                                      bool_vector_type * iactive);


  void                          enkf_main_run_smoother(enkf_main_type * enkf_main , enkf_fs_type * source_fs, const char * target_fs_name , bool_vector_type * iactive , int iter , bool rerun);
  void                          enkf_main_run_iterated_ES(enkf_main_type * enkf_main, int num_iterations);
  void                          enkf_main_set_data_kw(enkf_main_type * , const char * , const char *);
  void                          enkf_main_set_state_run_path(const enkf_main_type * , int );
  void                          enkf_main_set_state_eclbase(const enkf_main_type * , int );
  void                          enkf_main_interactive_set_runpath__(void * );
  enkf_main_type              * enkf_main_bootstrap(const char * model_config, bool strict, bool verbose);
  void                          enkf_main_create_new_config( const char * config_file , const char * storage_path , const char * dbase_type , int num_realizations);

  enkf_node_type             ** enkf_main_get_node_ensemble(const enkf_main_type * enkf_main , enkf_fs_type * src_fs , const char * key , int report_step);
  void                          enkf_main_node_mean( const enkf_node_type ** ensemble , int ens_size , enkf_node_type * mean );
  void                          enkf_main_node_std( const enkf_node_type ** ensemble , int ens_size , const enkf_node_type * mean , enkf_node_type * std);

  ert_impl_type                enkf_main_impl_type(const enkf_main_type *, const char * );
  enkf_state_type             * enkf_main_iget_state(const enkf_main_type * , int );
  enkf_state_type            ** enkf_main_get_ensemble( enkf_main_type * enkf_main);
  const enkf_state_type      ** enkf_main_get_ensemble_const( const enkf_main_type * enkf_main);

  const enkf_config_node_type * enkf_main_get_config_node(const enkf_main_type * , const char *);
  const sched_file_type       * enkf_main_get_sched_file(const enkf_main_type *);
  ranking_table_type          * enkf_main_get_ranking_table( const enkf_main_type * enkf_main );
  ecl_config_type             * enkf_main_get_ecl_config(const enkf_main_type * enkf_main);
  ensemble_config_type        * enkf_main_get_ensemble_config(const enkf_main_type * enkf_main);
  int                           enkf_main_get_ensemble_size( const enkf_main_type * enkf_main );
  int                           enkf_main_get_history_length( const enkf_main_type * );
  bool                          enkf_main_has_prediction( const enkf_main_type *  );
  //const enkf_sched_type       * enkf_main_get_enkf_sched(const enkf_main_type *);
  model_config_type           * enkf_main_get_model_config( const enkf_main_type * );
  local_config_type           * enkf_main_get_local_config( const enkf_main_type * enkf_main );
  plot_config_type            * enkf_main_get_plot_config( const enkf_main_type * enkf_main );
  void                          enkf_main_load_obs( enkf_main_type * enkf_main , const char * obs_config_file , bool clear_existing);
  enkf_obs_type               * enkf_main_get_obs(const enkf_main_type * );
  bool                          enkf_main_have_obs( const enkf_main_type * enkf_main );
  analysis_config_type        * enkf_main_get_analysis_config(const enkf_main_type * );

  void       * enkf_main_get_enkf_config_node_type(const ensemble_config_type *, const char *);
  void         enkf_main_set_field_config_iactive(const ensemble_config_type *, int);
  const char * enkf_main_get_image_viewer(const enkf_main_type * );
  const char * enkf_main_get_plot_driver(const enkf_main_type * enkf_main );
  const char * enkf_main_get_image_type(const enkf_main_type * enkf_main);




  void                     enkf_main_list_users(  set_type * users , const char * executable );
  const ext_joblist_type * enkf_main_get_installed_jobs( const enkf_main_type * enkf_main );

  subst_list_type        * enkf_main_get_data_kw( const enkf_main_type * enkf_main );
  void                     enkf_main_clear_data_kw( enkf_main_type * enkf_main );
  site_config_type       * enkf_main_get_site_config( const enkf_main_type * enkf_main );
  void                     enkf_main_resize_ensemble( enkf_main_type * enkf_main , int new_ens_size );
  void                     enkf_main_get_observations( const enkf_main_type * enkf_main, const char * user_key , int obs_count , time_t * obs_time , double * y , double * std);
  int                      enkf_main_get_observation_count( const enkf_main_type * enkf_main, const char * user_key );

  keep_runpath_type        enkf_main_iget_keep_runpath( const enkf_main_type * enkf_main , int iens );
  void                     enkf_main_iset_keep_runpath( enkf_main_type * enkf_main , int iens , keep_runpath_type keep_runpath);

  /*****************************************************************/
  void                        enkf_main_install_SIGNALS(void);
  const                char * enkf_main_get_SVN_VERSION( void );
  const                char * enkf_main_get_COMPILE_TIME( void );
  void                        enkf_main_del_node(enkf_main_type * enkf_main , const char * key);
  void                        enkf_main_add_node(enkf_main_type * enkf_main, enkf_config_node_type * enkf_config_node);
  void                        enkf_main_update_node( enkf_main_type * enkf_main , const char * key );
  int_vector_type           * enkf_main_update_alloc_step_list( const enkf_main_type * enkf_main , int load_start , int step2 , int stride);

  hook_manager_type         * enkf_main_get_hook_manager( const enkf_main_type * enkf_main );

  void enkf_main_get_PC( const matrix_type * S,
                         const matrix_type * dObs,
                         double truncation ,
                         int ncomp ,
                         matrix_type * PC ,
                         matrix_type * PC_obs,
                         double_vector_type * singular_values);


  void enkf_main_fprintf_PC(const char * filename ,
                            matrix_type * PC ,
                            matrix_type * PC_obs);


  void                   enkf_main_set_verbose( enkf_main_type * enkf_main , bool verbose);
  bool                   enkf_main_get_verbose( const enkf_main_type * enkf_main );

  ert_workflow_list_type * enkf_main_get_workflow_list( enkf_main_type * enkf_main );
  void                     enkf_main_run_workflows( enkf_main_type * enkf_main , const stringlist_type * workflows);
  bool                     enkf_main_run_workflow( enkf_main_type * enkf_main , const char * workflow);

  enkf_main_type      * enkf_main_alloc_empty( );

  rng_config_type     * enkf_main_get_rng_config( const enkf_main_type * enkf_main );
  void                  enkf_main_rng_init( enkf_main_type * enkf_main);


  char * enkf_main_alloc_abs_path_to_init_file(const enkf_main_type * enkf_main, const enkf_config_node_type * enkf_config_node);

  bool enkf_main_export_field(const enkf_main_type * enkf_main,
                              const char * kw,
                              const char * path,
                              bool_vector_type * iactive,
                              field_file_format_type file_type,
                              int report_step);

  bool enkf_main_export_field_with_fs(const enkf_main_type * enkf_main,
                              const char * kw,
                              const char * path,
                              bool_vector_type * iactive,
                              field_file_format_type file_type,
                              int report_step,
                              enkf_fs_type * fs);


  void enkf_main_load_from_forward_model_with_fs(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, stringlist_type ** realizations_msg_list, enkf_fs_type * fs);
  void enkf_main_load_from_forward_model(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, stringlist_type ** realizations_msg_list);
  void enkf_main_load_from_forward_model_from_gui(enkf_main_type * enkf_main, int iter , bool_vector_type * iactive, enkf_fs_type * fs);

  void enkf_main_rank_on_observations(enkf_main_type * enkf_main,
                                      const char * ranking_key,
                                      const stringlist_type * obs_ranking_keys,
                                      const int_vector_type * steps);



  void enkf_main_rank_on_data(enkf_main_type * enkf_main,
                              const char * ranking_key,
                              const char * data_key,
                              bool sort_increasing,
                              int step);


  void enkf_main_export_ranking(enkf_main_type * enkf_main, const char * ranking_key, const char * ranking_file);







  /********* File System / Case Management **********
          Implementation: enkf_main_manage_fs.c     */


  bool              enkf_main_case_is_current(const enkf_main_type * enkf_main , const char * case_path);
  char *            enkf_main_read_alloc_current_case_name(const enkf_main_type * enkf_main);
  stringlist_type * enkf_main_alloc_caselist( const enkf_main_type * enkf_main );
  void              enkf_main_set_case_table( enkf_main_type * enkf_main , const char * case_table_file );

  void              enkf_main_initialize_from_scratch(enkf_main_type * enkf_main ,
						      enkf_fs_type * init_fs,
						      const stringlist_type * param_list ,
						      const bool_vector_type * iens_mask ,
						      init_mode_type init_mode);

  void              enkf_main_init_current_case_from_existing(enkf_main_type * enkf_main,
                                                              enkf_fs_type * source_case_fs,
                                                              int source_report_step);

  void              enkf_main_init_current_case_from_existing_custom(enkf_main_type * enkf_main,
                                                                    enkf_fs_type * source_case_fs,
                                                                    int source_report_step,
                                                                    stringlist_type * node_list,
                                                                    bool_vector_type * iactive);


  void              enkf_main_init_case_from_existing(const enkf_main_type * enkf_main,
                                                      enkf_fs_type * source_case_fs,
                                                      int source_report_step,
                                                      enkf_fs_type * target_case);

  void              enkf_main_init_case_from_existing_custom(const enkf_main_type * enkf_main,
                                                             enkf_fs_type * source_case_fs,
                                                             int source_report_step,
                                                             enkf_fs_type * target_case,
                                                             stringlist_type * node_list,
                                                             bool_vector_type * iactive);

  bool              enkf_main_case_is_initialized( const enkf_main_type * enkf_main ,
                                                   const char * case_name ,
                                                   bool_vector_type * __mask);

  bool              enkf_main_is_initialized( const enkf_main_type * enkf_main ,bool_vector_type * __mask);

  char            * enkf_main_alloc_mount_point( const enkf_main_type * enkf_main , const char * case_path);
  enkf_fs_type    * enkf_main_get_fs(const enkf_main_type * );
  enkf_fs_type    * enkf_main_tui_get_fs(const enkf_main_type * );
  enkf_fs_type    * enkf_main_job_get_fs(const enkf_main_type * );
  enkf_fs_type    * enkf_main_get_fs_ref(const enkf_main_type * enkf_main);
  const char      * enkf_main_get_current_fs( const enkf_main_type * enkf_main );
  enkf_fs_type    * enkf_main_mount_alt_fs(const enkf_main_type * enkf_main , const char * case_path ,  bool create);
  void              enkf_main_set_fs( enkf_main_type * enkf_main , enkf_fs_type * fs , const char * case_path );
  void              enkf_main_user_select_fs(enkf_main_type * enkf_main , const char * case_path );
  void              enkf_main_select_fs( enkf_main_type * enkf_main , const char * case_path );
  bool              enkf_main_fs_exists(const enkf_main_type * enkf_main, const char * input_case);
  const      char * enkf_main_get_mount_root( const enkf_main_type * enkf_main);


  state_map_type  * enkf_main_alloc_readonly_state_map( const enkf_main_type * enkf_main , const char * case_path);
  time_map_type   * enkf_main_alloc_readonly_time_map( const enkf_main_type * enkf_main , const char * case_path );

  runpath_list_type    * enkf_main_get_runpath_list( const enkf_main_type * enkf_main );
  ert_run_context_type * enkf_main_alloc_ert_run_context_ENSEMBLE_EXPERIMENT(const enkf_main_type * enkf_main , enkf_fs_type * fs , bool_vector_type * iactive , int iter);
  ert_init_context_type * enkf_main_alloc_ert_init_context(const enkf_main_type * enkf_main , enkf_fs_type * fs, const bool_vector_type * iactive , init_mode_type init_mode , int iter);


UTIL_SAFE_CAST_HEADER(enkf_main);
UTIL_IS_INSTANCE_HEADER(enkf_main);

#ifdef __cplusplus
}
#endif
#endif
