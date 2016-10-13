add_executable( enkf_runpath_list enkf_runpath_list.c )
target_link_libraries( enkf_runpath_list enkf test_util )
add_test( enkf_runpath_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_runpath_list ${CMAKE_CURRENT_SOURCE_DIR}/data/config/runpath_list/config )

add_executable( enkf_plot_tvector enkf_plot_tvector.c )
target_link_libraries( enkf_plot_tvector enkf test_util )
add_test( enkf_plot_tvector ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_tvector)

add_executable( enkf_plot_data enkf_plot_data.c )
target_link_libraries( enkf_plot_data enkf test_util )
add_test( enkf_plot_data ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_data)

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


add_executable( enkf_ert_workflow_list enkf_ert_workflow_list.c )
target_link_libraries( enkf_ert_workflow_list enkf test_util )
add_test( enkf_ert_workflow_list ${EXECUTABLE_OUTPUT_PATH}/enkf_ert_workflow_list ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/SCALE_STD)


add_executable( enkf_obs_vector enkf_obs_vector.c )
target_link_libraries( enkf_obs_vector enkf test_util )
add_test( enkf_obs_vector ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_vector  )




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
add_test( enkf_analysis_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config)

add_executable( enkf_analysis_config_ext_module enkf_analysis_config_ext_module.c )
target_link_libraries( enkf_analysis_config_ext_module enkf test_util )

ert_module_name( VAR_RML  rml_enkf  ${LIBRARY_OUTPUT_PATH} )
add_test( enkf_analysis_config_ext_module ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config_ext_module 
          rml_enkf ${VAR_RML} )

add_executable( enkf_analysis_config_analysis_load enkf_analysis_config_analysis_load.c )
target_link_libraries( enkf_analysis_config_analysis_load enkf test_util)
add_test( enkf_analysis_config_analysis_load ${EXECUTABLE_OUTPUT_PATH}/enkf_analysis_config_analysis_load ${CMAKE_CURRENT_SOURCE_DIR}/data/config/analysis_load_config)
set_property( TEST enkf_analysis_config_analysis_load PROPERTY ENVIRONMENT "ERT_SITE_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/data/config/analysis_load_site_config" )

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

add_executable( enkf_fs enkf_fs.c )
target_link_libraries( enkf_fs enkf test_util )
add_test( enkf_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_fs )

add_executable( enkf_workflow_job_test_version enkf_workflow_job_test_version.c )
target_link_libraries( enkf_workflow_job_test_version enkf test_util )
add_test( enkf_workflow_job_test_version  ${EXECUTABLE_OUTPUT_PATH}/enkf_workflow_job_test_version 
  ${CMAKE_CURRENT_SOURCE_DIR}/data/workflow_jobs )


add_executable( enkf_ert_test_context enkf_ert_test_context.c )
target_link_libraries( enkf_ert_test_context enkf test_util )
add_test( enkf_ert_test_context  ${EXECUTABLE_OUTPUT_PATH}/enkf_ert_test_context 
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/config
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/wf_job
          ${CMAKE_CURRENT_SOURCE_DIR}/data/config/test_context/wf_job_fail)



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


add_executable( enkf_umask_config_test enkf_umask_config_test.c )
target_link_libraries( enkf_umask_config_test enkf test_util )

add_test( enkf_umask_config_test
          ${EXECUTABLE_OUTPUT_PATH}/enkf_umask_config_test
          ${PROJECT_SOURCE_DIR}/test-data/local/simple_config/config_umask)

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


add_executable( enkf_hook_manager_test enkf_hook_manager_test.c )
target_link_libraries( enkf_hook_manager_test enkf test_util )
add_test( enkf_hook_manager_test ${EXECUTABLE_OUTPUT_PATH}/enkf_hook_manager_test )
 
add_executable(enkf_obs_tests enkf_obs_tests.c)
target_link_libraries(enkf_obs_tests enkf test_util )
add_test(enkf_obs_tests ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_tests)

add_executable(obs_vector_tests obs_vector_tests.c)
target_link_libraries(obs_vector_tests enkf test_util )
add_test(obs_vector_tests ${EXECUTABLE_OUTPUT_PATH}/obs_vector_tests)


add_executable( enkf_state_map enkf_state_map.c )
target_link_libraries( enkf_state_map enkf test_util )
add_test( enkf_state_map  ${EXECUTABLE_OUTPUT_PATH}/enkf_state_map )


add_executable( enkf_meas_data enkf_meas_data.c )
target_link_libraries( enkf_meas_data enkf test_util )
add_test( enkf_meas_data  ${EXECUTABLE_OUTPUT_PATH}/enkf_meas_data )

add_executable( enkf_ensemble_GEN_PARAM enkf_ensemble_GEN_PARAM.c )
target_link_libraries( enkf_ensemble_GEN_PARAM enkf test_util )
add_test( enkf_ensemble_GEN_PARAM  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble_GEN_PARAM ${CMAKE_CURRENT_SOURCE_DIR}/data/ensemble/GEN_PARAM )

add_executable( enkf_ensemble enkf_ensemble.c )
target_link_libraries( enkf_ensemble enkf test_util )
add_test( enkf_ensemble  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble )
