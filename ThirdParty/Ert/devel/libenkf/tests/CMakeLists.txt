add_executable( enkf_runpath_list enkf_runpath_list.c )
target_link_libraries( enkf_runpath_list enkf test_util )

add_executable( enkf_site_config enkf_site_config.c )
target_link_libraries( enkf_site_config enkf test_util )

add_executable( enkf_plot_tvector enkf_plot_tvector.c )
target_link_libraries( enkf_plot_tvector enkf test_util )
add_test( enkf_plot_tvector ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_tvector)

add_executable( enkf_plot_data enkf_plot_data.c )
target_link_libraries( enkf_plot_data enkf test_util )
add_test( enkf_plot_data ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_data)

add_executable( enkf_gen_data_config enkf_gen_data_config.c )
target_link_libraries( enkf_gen_data_config enkf test_util )
add_test( enkf_gen_data_config ${EXECUTABLE_OUTPUT_PATH}/enkf_gen_data_config
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/gendata_test/RFT_E-3H_21
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/gendata_test/RFT_E-3H_21_empty)

add_executable( enkf_ert_run_context enkf_ert_run_context.c )
target_link_libraries( enkf_ert_run_context enkf test_util )
add_test( enkf_ert_run_context ${EXECUTABLE_OUTPUT_PATH}/enkf_ert_run_context)


add_executable( enkf_run_arg enkf_run_arg.c )
target_link_libraries( enkf_run_arg enkf test_util )
add_test( enkf_run_arg ${EXECUTABLE_OUTPUT_PATH}/enkf_run_arg)

add_executable( enkf_gen_obs_load enkf_gen_obs_load.c )
target_link_libraries( enkf_gen_obs_load enkf test_util )
add_test( enkf_gen_obs_load ${EXECUTABLE_OUTPUT_PATH}/enkf_gen_obs_load ${PROJECT_SOURCE_DIR}/test-data/local/config/gen_data/config )

add_executable( enkf_gen_data_config_parse enkf_gen_data_config_parse.c )
target_link_libraries( enkf_gen_data_config_parse enkf test_util )
add_test( enkf_gen_data_config_parse ${EXECUTABLE_OUTPUT_PATH}/enkf_gen_data_config_parse)

add_executable( enkf_enkf_config_node_gen_data enkf_enkf_config_node_gen_data.c )
target_link_libraries( enkf_enkf_config_node_gen_data enkf test_util )
add_test( enkf_enkf_config_node_gen_data ${EXECUTABLE_OUTPUT_PATH}/enkf_enkf_config_node_gen_data)

add_executable( enkf_block_obs enkf_block_obs.c )
target_link_libraries( enkf_block_obs enkf test_util )
add_test( enkf_block_obs ${EXECUTABLE_OUTPUT_PATH}/enkf_block_obs ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID)


add_executable( enkf_ert_workflow_list enkf_ert_workflow_list.c )
target_link_libraries( enkf_ert_workflow_list enkf test_util )
add_test( enkf_ert_workflow_list ${EXECUTABLE_OUTPUT_PATH}/enkf_ert_workflow_list ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/SCALE_STD)


add_executable( enkf_obs_fs enkf_obs_fs.c )
target_link_libraries( enkf_obs_fs enkf test_util )
add_test( enkf_obs_fs ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/obs_testing/config )

add_executable( enkf_magic_string_in_workflows enkf_magic_string_in_workflows.c )
target_link_libraries( enkf_magic_string_in_workflows enkf test_util )
add_test( enkf_magic_string_in_workflows ${EXECUTABLE_OUTPUT_PATH}/enkf_magic_string_in_workflows ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_data/config )

add_executable( enkf_obs_vector_fs enkf_obs_vector_fs.c )
target_link_libraries( enkf_obs_vector_fs enkf test_util )
add_test( enkf_obs_vector_fs ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_vector_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/obs_testing/config )

add_executable( enkf_obs_vector enkf_obs_vector.c )
target_link_libraries( enkf_obs_vector enkf test_util )
add_test( enkf_obs_vector ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_vector  )


add_executable( enkf_plot_data_fs enkf_plot_data_fs.c )
target_link_libraries( enkf_plot_data_fs enkf test_util )
add_test( enkf_plot_data_fs ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_data_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/plotData/config )

add_executable( enkf_time_map enkf_time_map.c )
target_link_libraries( enkf_time_map enkf test_util )
add_test( enkf_time_map1  ${EXECUTABLE_OUTPUT_PATH}/enkf_time_map )
add_test( enkf_time_map2  ${EXECUTABLE_OUTPUT_PATH}/enkf_time_map ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE 
                          ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/ModifiedSummary/EXTRA_TSTEP 
                          ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/ModifiedSummary/SHORT 
                          ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/ModifiedSummary/MISSING_TSTEP )


add_executable( enkf_ensemble_config enkf_ensemble_config.c )
target_link_libraries( enkf_ensemble_config enkf test_util )
add_test( enkf_ensemble_config ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble_config)

add_executable( enkf_pca_plot enkf_pca_plot.c )
target_link_libraries( enkf_pca_plot enkf test_util)
add_test( enkf_pca_plot ${EXECUTABLE_OUTPUT_PATH}/enkf_pca_plot)

add_executable( enkf_cases_config enkf_cases_config.c )
target_link_libraries( enkf_cases_config enkf test_util )
add_test( enkf_cases_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_cases_config )

add_executable( enkf_analysis_config enkf_analysis_config.c )
target_link_libraries( enkf_analysis_config enkf test_util )
add_test( enkf_analysis  ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config)

add_executable( enkf_analysis_config_ext_module enkf_analysis_config_ext_module.c )
target_link_libraries( enkf_analysis_config_ext_module enkf test_util )

ert_module_name( VAR_RML  rml_enkf  ${LIBRARY_OUTPUT_PATH} )
add_test( enkf_analysis_config_ext_module ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config_ext_module 
          rml_enkf ${VAR_RML} )

add_executable( enkf_analysis_config_analysis_load enkf_analysis_config_analysis_load.c )
target_link_libraries( enkf_analysis_config_analysis_load enkf test_util)
add_test( enkf_analysis_config_analysis_load ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config_analysis_load ${CMAKE_CURRENT_SOURCE_DIR}/data/config/analysis_load_config)
set_property( TEST enkf_analysis_config_analysis_load PROPERTY ENVIRONMENT "ERT_SITE_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/data/config/analysis_load_site_config" )

add_executable( enkf_state_map enkf_state_map.c )
target_link_libraries( enkf_state_map enkf test_util )

add_executable( enkf_meas_data enkf_meas_data.c )
target_link_libraries( enkf_meas_data enkf test_util )

add_executable( enkf_ensemble_GEN_PARAM enkf_ensemble_GEN_PARAM.c )
target_link_libraries( enkf_ensemble_GEN_PARAM enkf test_util )

add_executable( enkf_ensemble enkf_ensemble.c )
target_link_libraries( enkf_ensemble enkf test_util )

add_executable( enkf_local_obsdata_node enkf_local_obsdata_node.c )
target_link_libraries( enkf_local_obsdata_node enkf test_util)
add_test( enkf_local_obsdata_node ${EXECUTABLE_OUTPUT_PATH}/enkf_local_obsdata_node )

add_executable( enkf_local_obsdata enkf_local_obsdata.c )
target_link_libraries( enkf_local_obsdata enkf test_util)
add_test( enkf_local_obsdata ${EXECUTABLE_OUTPUT_PATH}/enkf_local_obsdata )

add_executable( enkf_active_list enkf_active_list.c )
target_link_libraries( enkf_active_list enkf test_util)
add_test( enkf_active_list ${EXECUTABLE_OUTPUT_PATH}/enkf_active_list )

add_executable( enkf_main enkf_main.c )
target_link_libraries( enkf_main enkf test_util )
add_test( enkf_main  ${EXECUTABLE_OUTPUT_PATH}/enkf_main )

add_executable( enkf_main_fs enkf_main_fs.c )
target_link_libraries( enkf_main_fs enkf test_util )
add_test( enkf_main_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_main_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/plotData/config )

add_executable( enkf_main_fs_current_file_test enkf_main_fs_current_file_test.c )
target_link_libraries( enkf_main_fs_current_file_test enkf test_util )
add_test( enkf_main_fs_current_file_test  ${EXECUTABLE_OUTPUT_PATH}/enkf_main_fs_current_file_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/plotData/config )

add_executable( enkf_fs enkf_fs.c )
target_link_libraries( enkf_fs enkf test_util )
add_test( enkf_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_fs )


add_executable( enkf_ert_test_context enkf_ert_test_context.c )
target_link_libraries( enkf_ert_test_context enkf test_util )
add_test( enkf_ert_test_context  ${EXECUTABLE_OUTPUT_PATH}/enkf_ert_test_context 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/config
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/wf_job
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/wf_job_fail)


add_executable( enkf_scale_correlated_std enkf_scale_correlated_std.c )
target_link_libraries( enkf_scale_correlated_std enkf test_util )
add_test( enkf_scale_correlated_std ${EXECUTABLE_OUTPUT_PATH}/enkf_scale_correlated_std
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_data/config 
          ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/STD_SCALE_CORRELATED_OBS )

add_executable( enkf_plot_gen_kw enkf_plot_gen_kw.c )
target_link_libraries( enkf_plot_gen_kw enkf test_util )
add_test( enkf_plot_gen_kw  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gen_kw )

add_executable( enkf_plot_gen_kw_vector enkf_plot_gen_kw_vector.c )
target_link_libraries( enkf_plot_gen_kw_vector enkf test_util )
add_test( enkf_plot_gen_kw_vector  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gen_kw_vector )

add_executable( enkf_plot_gen_kw_fs enkf_plot_gen_kw_fs.c )
target_link_libraries( enkf_plot_gen_kw_fs enkf  test_util )
add_test( enkf_plot_gen_kw_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gen_kw_fs ${CMAKE_CURRENT_SOURCE_DIR}/data/config/gen_kw_plot/config )

add_executable( enkf_plot_genvector enkf_plot_genvector.c )
target_link_libraries( enkf_plot_genvector enkf test_util )
add_test( enkf_plot_genvector  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_genvector )

add_executable( enkf_plot_gendata_fs enkf_plot_gendata_fs.c )
target_link_libraries( enkf_plot_gendata_fs  enkf test_util )
add_test( enkf_plot_gendata_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gendata_fs
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_GEN_DATA/config )

add_executable( enkf_plot_gendata enkf_plot_gendata.c )
target_link_libraries( enkf_plot_gendata enkf test_util )
add_test( enkf_plot_gendata  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gendata )

add_executable( enkf_config_node enkf_config_node.c )
target_link_libraries( enkf_config_node enkf test_util )
add_test( enkf_config_node  ${EXECUTABLE_OUTPUT_PATH}/enkf_config_node )


#-----------------------------------------------------------------

add_executable( gen_kw_test gen_kw_test.c )
target_link_libraries( gen_kw_test enkf test_util )

add_test( gen_kw_test
          ${EXECUTABLE_OUTPUT_PATH}/gen_kw_test
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/ert/config_GEN_KW_true)


add_executable( gen_kw_logarithmic_test gen_kw_logarithmic_test.c )
target_link_libraries( gen_kw_logarithmic_test enkf test_util )

add_test( gen_kw_logarithmic_test
          ${EXECUTABLE_OUTPUT_PATH}/gen_kw_logarithmic_test
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/gen_kw_logarithmic/config_GEN_KW_logarithmic)



#-----------------------------------------------------------------

add_executable( enkf_forward_init_GEN_KW enkf_forward_init_GEN_KW.c )
target_link_libraries( enkf_forward_init_GEN_KW enkf test_util )

add_test( enkf_forward_init_GEN_KW_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_KW 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/ert config_GEN_KW_true TRUE)

add_test( enkf_forward_init_GEN_KW_FALSE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_KW 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/ert config_GEN_KW_false FALSE)


#-----------------------------------------------------------------

add_executable( enkf_state_report_step_compatible enkf_state_report_step_compatible.c )
target_link_libraries( enkf_state_report_step_compatible enkf test_util )

add_test( enkf_state_report_step_compatible_TRUE 
          ${EXECUTABLE_OUTPUT_PATH}/enkf_state_report_step_compatible ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_sum_compatible_true  config_ecl_sum_compatible_true TRUE)

add_test( enkf_state_report_step_compatible_FALSE 
          ${EXECUTABLE_OUTPUT_PATH}/enkf_state_report_step_compatible ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_sum_compatible_false  config_ecl_sum_compatible_false FALSE)
                                                                      

#-----------------------------------------------------------------

add_executable( enkf_state_manual_load_test enkf_state_manual_load_test.c )
target_link_libraries( enkf_state_manual_load_test enkf test_util )
add_test( enkf_state_manual_load_test  ${EXECUTABLE_OUTPUT_PATH}/enkf_state_manual_load_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_sum_compatible_true  config_ecl_sum_compatible_true)

#-----------------------------------------------------------------


add_executable( enkf_state_skip_summary_load_test enkf_state_skip_summary_load_test.c )
target_link_libraries( enkf_state_skip_summary_load_test enkf test_util )

add_test( enkf_state_summary_vars_present
          ${EXECUTABLE_OUTPUT_PATH}/enkf_state_skip_summary_load_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_summary_vars_config  config_summary_vars)

add_test( enkf_state_no_summary_vars_present 
          ${EXECUTABLE_OUTPUT_PATH}/enkf_state_skip_summary_load_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_no_summary_vars_config  config_no_summary_vars)
                                                                      

#-----------------------------------------------------------------


add_executable( enkf_export_field_test enkf_export_field_test.c )
target_link_libraries( enkf_export_field_test  enkf test_util )

add_test( enkf_export_field_test
          ${EXECUTABLE_OUTPUT_PATH}/enkf_export_field_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/export_fields/config
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/EXPORT_FIELD
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/EXPORT_FIELD_ECL_GRDECL
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/EXPORT_FIELD_RMS_ROFF)



add_executable( enkf_workflow_job_test enkf_workflow_job_test.c )
target_link_libraries( enkf_workflow_job_test enkf test_util )

add_test( enkf_workflow_job_test
          ${EXECUTABLE_OUTPUT_PATH}/enkf_workflow_job_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_data/config
                                                           ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/enkf_state_runpath/config_runpath_multiple_iterations
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal-tui/config/CREATE_CASE
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal-tui/config/INIT_CASE_FROM_EXISTING
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/LOAD_RESULTS
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/LOAD_RESULTS_ITER
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/OBSERVATION_RANKING
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/DATA_RANKING
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/EXPORT_RANKING
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/INIT_MISFIT_TABLE
                                                           ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/EXPORT_RUNPATH)



add_executable( enkf_select_case_job enkf_select_case_job.c )
target_link_libraries( enkf_select_case_job  enkf test_util )

add_test( enkf_select_case_job
          ${EXECUTABLE_OUTPUT_PATH}/enkf_select_case_job 
          ${PROJECT_SOURCE_DIR}/test-data/local/snake_oil/snake_oil.ert
          ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal-tui/config/SELECT_CASE)


#-----------------------------------------------------------------


add_executable( enkf_forward_init_GEN_PARAM enkf_forward_init_GEN_PARAM.c )
target_link_libraries( enkf_forward_init_GEN_PARAM enkf test_util )

add_test( enkf_forward_init_GEN_PARAM_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_PARAM 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/ert config_GEN_PARAM_true TRUE)

add_test( enkf_forward_init_GEN_PARAM_FALSE  
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_PARAM 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/ert config_GEN_PARAM_false FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_SURFACE enkf_forward_init_SURFACE.c )
target_link_libraries( enkf_forward_init_SURFACE enkf test_util )

add_test( enkf_forward_init_SURFACE_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_SURFACE 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface config_surface_true   
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/Surface.irap
          TRUE)

add_test( enkf_forward_init_SURFACE_FALSE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_SURFACE 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface config_surface_false
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/Surface.irap
          FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_FIELD enkf_forward_init_FIELD.c )
target_link_libraries( enkf_forward_init_FIELD enkf test_util )

add_test( enkf_forward_init_FIELD_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_FIELD 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field config_field_true   
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/petro.grdecl
          TRUE)

add_test( enkf_forward_init_FIELD_FALSE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_FIELD 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field config_field_false
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/petro.grdecl
          FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_transform enkf_forward_init_transform.c )
target_link_libraries( enkf_forward_init_transform enkf test_util )

add_test( enkf_forward_init_transform_TRUE
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_transform
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/transform transform_forward_init_true
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/transform/petro.grdecl
          TRUE)

add_test( enkf_forward_init_transform_FALSE
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_transform
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/transform transform_forward_init_false
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/transform/petro.grdecl
          FALSE)

#-----------------------------------------------------------------

add_executable( enkf_export_inactive_cells enkf_export_inactive_cells.c )
target_link_libraries( enkf_export_inactive_cells enkf test_util )

add_test( enkf_export_inactive_cells
          ${EXECUTABLE_OUTPUT_PATH}/enkf_export_inactive_cells
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/export_inactive_cells/config
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/export_inactive_cells/petro.grdecl)

#-----------------------------------------------------------------


add_executable( enkf_iter_config enkf_iter_config.c )
target_link_libraries( enkf_iter_config enkf test_util )
add_test( enkf_iter_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_iter_config )


add_executable( enkf_model_config enkf_model_config.c )
target_link_libraries( enkf_model_config enkf test_util )
add_test( enkf_model_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_model_config )

add_executable( enkf_rng enkf_rng.c )
target_link_libraries( enkf_rng enkf test_util )
add_test( enkf_rng  ${EXECUTABLE_OUTPUT_PATH}/enkf_rng ${CMAKE_CURRENT_SOURCE_DIR}/data/config rng)

add_executable( enkf_forward_load_context enkf_forward_load_context.c )
target_link_libraries( enkf_forward_load_context enkf test_util )
add_test( enkf_forward_load_context  ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_load_context ${CMAKE_CURRENT_SOURCE_DIR}/data/config forward_load_context)

add_executable( enkf_refcase_list enkf_refcase_list.c )
target_link_libraries( enkf_refcase_list enkf test_util )
add_test( enkf_refcase_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_refcase_list ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat*/ECLIPSE) 
add_test( enkf_refcase_list2  ${EXECUTABLE_OUTPUT_PATH}/enkf_refcase_list ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat*/ECLIPSE.*) 
set_property( TEST enkf_refcase_list PROPERTY LABELS StatoilData )
set_property( TEST enkf_refcase_list2 PROPERTY LABELS StatoilData )

add_executable( enkf_ecl_config enkf_ecl_config.c )
target_link_libraries( enkf_ecl_config enkf test_util )
add_test( enkf_ecl_config1  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config )
add_test( enkf_ecl_config2  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE)
set_property( TEST enkf_ecl_config2 PROPERTY LABELS StatoilData )

add_executable( enkf_ecl_config_config enkf_ecl_config_config.c )
target_link_libraries( enkf_ecl_config_config enkf test_util )
add_test( enkf_ecl_config_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config_config ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_config )
set_property( TEST enkf_ecl_config_config PROPERTY LABELS StatoilData )

add_executable( enkf_hook_manager_test enkf_hook_manager_test.c )
target_link_libraries( enkf_hook_manager_test enkf test_util )
add_test( enkf_hook_manager_test ${EXECUTABLE_OUTPUT_PATH}/enkf_hook_manager_test )
 
add_executable(enkf_obs_tests enkf_obs_tests.c)
target_link_libraries(enkf_obs_tests enkf test_util )
add_test(enkf_obs_tests ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_tests)

add_executable(obs_vector_tests obs_vector_tests.c)
target_link_libraries(obs_vector_tests enkf test_util )
add_test(obs_vector_tests ${EXECUTABLE_OUTPUT_PATH}/obs_vector_tests)

add_test( enkf_runpath_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_runpath_list ${CMAKE_CURRENT_SOURCE_DIR}/data/config/runpath_list/config )
add_test( enkf_site_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_site_config /project/res/etc/ERT/site-config)

add_test( enkf_ensemble_GEN_PARAM  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble_GEN_PARAM ${CMAKE_CURRENT_SOURCE_DIR}/data/ensemble/GEN_PARAM )
add_test( enkf_ensemble  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble )
add_test( enkf_state_map  ${EXECUTABLE_OUTPUT_PATH}/enkf_state_map )
add_test( enkf_meas_data  ${EXECUTABLE_OUTPUT_PATH}/enkf_meas_data )

set_property( TEST enkf_plot_data_fs  PROPERTY LABELS StatoilData )
set_property( TEST enkf_time_map2     PROPERTY LABELS StatoilData )
set_property( TEST enkf_site_config   PROPERTY LABELS StatoilData )
set_property( TEST enkf_state_report_step_compatible_TRUE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_state_report_step_compatible_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_SURFACE_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_SURFACE_TRUE   PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_FIELD_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_FIELD_TRUE   PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_transform_TRUE   PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_transform_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_main_fs   PROPERTY LABELS StatoilData )
set_property( TEST enkf_state_summary_vars_present PROPERTY LABELS StatoilData )
set_property( TEST enkf_state_no_summary_vars_present PROPERTY LABELS StatoilData )
set_property( TEST enkf_export_field_test PROPERTY LABELS StatoilData )
set_property( TEST enkf_workflow_job_test PROPERTY LABELS StatoilData )
set_property( TEST enkf_main_fs_current_file_test  PROPERTY LABELS StatoilData )
set_property( TEST enkf_state_manual_load_test PROPERTY LABELS StatoilData )
set_property( TEST enkf_block_obs PROPERTY LABELS StatoilData )
set_property( TEST enkf_plot_gendata_fs PROPERTY LABELS StatoilData )
set_property( TEST enkf_export_inactive_cells PROPERTY LABELS StatoilData )
set_property( TEST enkf_obs_fs PROPERTY LABELS StatoilData )
set_property( TEST enkf_obs_vector_fs PROPERTY LABELS StatoilData )
set_property( TEST enkf_magic_string_in_workflows PROPERTY LABELS StatoilData )
set_property( TEST enkf_gen_data_config PROPERTY LABELS StatoilData )
set_property( TEST enkf_scale_correlated_std PROPERTY LABELS StatoilData )
