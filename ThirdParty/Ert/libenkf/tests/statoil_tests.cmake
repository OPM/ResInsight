add_executable( enkf_site_config enkf_site_config.c )
target_link_libraries( enkf_site_config enkf test_util )
add_test( enkf_site_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_site_config /project/res/etc/ERT/site-config)

add_executable( enkf_gen_data_config enkf_gen_data_config.c )
target_link_libraries( enkf_gen_data_config enkf test_util )
add_test( enkf_gen_data_config ${EXECUTABLE_OUTPUT_PATH}/enkf_gen_data_config
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/gendata_test/RFT_E-3H_21
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/gendata_test/RFT_E-3H_21_empty)

add_executable( enkf_block_obs enkf_block_obs.c )
target_link_libraries( enkf_block_obs enkf test_util )
add_test( enkf_block_obs ${EXECUTABLE_OUTPUT_PATH}/enkf_block_obs ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID)

add_executable( enkf_obs_fs enkf_obs_fs.c )
target_link_libraries( enkf_obs_fs enkf test_util )
add_test( enkf_obs_fs ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/obs_testing/config )

add_executable( enkf_magic_string_in_workflows enkf_magic_string_in_workflows.c )
target_link_libraries( enkf_magic_string_in_workflows enkf test_util )
add_test( enkf_magic_string_in_workflows ${EXECUTABLE_OUTPUT_PATH}/enkf_magic_string_in_workflows ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_data/config )

add_executable( enkf_obs_vector_fs enkf_obs_vector_fs.c )
target_link_libraries( enkf_obs_vector_fs enkf test_util )
add_test( enkf_obs_vector_fs ${EXECUTABLE_OUTPUT_PATH}/enkf_obs_vector_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/obs_testing/config )

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

add_executable( enkf_main_fs enkf_main_fs.c )
target_link_libraries( enkf_main_fs enkf test_util )
add_test( enkf_main_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_main_fs ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/plotData/config )

add_executable( enkf_main_fs_current_file_test enkf_main_fs_current_file_test.c )
target_link_libraries( enkf_main_fs_current_file_test enkf test_util )
add_test( enkf_main_fs_current_file_test  ${EXECUTABLE_OUTPUT_PATH}/enkf_main_fs_current_file_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/plotData/config )

add_executable( enkf_scale_correlated_std enkf_scale_correlated_std.c )
target_link_libraries( enkf_scale_correlated_std enkf test_util )
add_test( enkf_scale_correlated_std ${EXECUTABLE_OUTPUT_PATH}/enkf_scale_correlated_std
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_data/config 
          ${PROJECT_SOURCE_DIR}/share/workflows/jobs/internal/config/STD_SCALE_CORRELATED_OBS )

add_executable( enkf_plot_gendata_fs enkf_plot_gendata_fs.c )
target_link_libraries( enkf_plot_gendata_fs  enkf test_util )
add_test( enkf_plot_gendata_fs  ${EXECUTABLE_OUTPUT_PATH}/enkf_plot_gendata_fs
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/with_GEN_DATA/config )

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