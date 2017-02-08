set( source_files
     time_map.c
     rng_config.c
     trans_func.c
     enkf_types.c
     enkf_obs.c
     obs_data.c
     block_obs.c
     enkf_config_node.c
     field_config.c
     field.c
     enkf_state.c
     enkf_util.c
     enkf_node.c
     gen_kw_config.c
     gen_kw.c
     enkf_fs.c
     fs_driver.c
     meas_data.c
     summary_obs.c
     summary.c
     summary_config.c
     gen_data_config.c
     gen_data.c
     gen_common.c
     gen_obs.c
     enkf_serialize.c
     ecl_config.c
     enkf_defaults.c
     ensemble_config.c
     model_config.c
     site_config.c
     active_list.c
     obs_vector.c
     field_trans.c
     plain_driver.c
     local_ministep.c
     local_updatestep.c
     container_config.c
     container.c
     local_context.c
     local_config.c
     analysis_config.c
     misfit_ensemble.c
     misfit_member.c
     misfit_ts.c
     data_ranking.c
     misfit_ranking.c
     ranking_table.c
     fs_types.c
     block_fs_driver.c
     plot_settings.c
     ert_template.c
     member_config.c
     enkf_analysis.c
     enkf_main.c
     local_dataset.c
     surface.c
     surface_config.c
     enkf_plot_data.c
     enkf_plot_tvector.c
     enkf_plot_gendata.c
     enkf_plot_genvector.c
     enkf_plot_gen_kw.c
     enkf_plot_gen_kw_vector.c
     hook_manager.c
     hook_workflow.c
     runpath_list.c
     ert_workflow_list.c
     analysis_iter_config.c
     enkf_main_jobs.c
     ecl_refcase_list.c
     local_obsdata_node.c
     local_obsdata.c
     pca_plot_data.c
     pca_plot_vector.c
     state_map.c
     cases_config.c
     state_map.c
     summary_key_set.c
     summary_key_matcher.c
     ert_test_context.c
     ert_log.c
     run_arg.c
     ert_run_context.c
     ert_init_context.c
     custom_kw.c
     custom_kw_config.c
     custom_kw_config_set.c
     forward_load_context.c
)

set( header_files
     time_map.h
     rng_config.h
     enkf_analysis.h
     enkf_fs_type.h
     trans_func.h
     enkf_obs.h
     obs_data.h
     enkf_config_node.h
     block_obs.h
     field_config.h
     field.h
     enkf_macros.h
     enkf_state.h
     enkf_util.h
     enkf_main.h
     enkf_main_jobs.h
     enkf_node.h
     enkf_fs.h
     gen_kw_config.h
     gen_kw.h
     enkf_types.h
     fs_driver.h
     meas_data.h
     summary_obs.h
     summary_config.h
     summary_config.h
     gen_data_config.h
     gen_data.h
     gen_common.h
     gen_obs.h
     fs_types.h
     enkf_serialize.h
     plain_driver.h
     ecl_config.h
     ensemble_config.h
     model_config.h
     site_config.h
     active_list.h
     obs_vector.h
     field_trans.h
     plain_driver.h
     local_ministep.h
     container.h
     local_updatestep.h
     local_config.h
     analysis_config.h
     misfit_ensemble.h
     misfit_ensemble_typedef.h
     misfit_ts.h
     misfit_member.h
     data_ranking.h
     ranking_table.h
     ranking_common.h
     misfit_ranking.h
     block_fs_driver.h
     field_common.h
     gen_kw_common.h
     gen_data_common.h
     plot_settings.h
     ert_template.h
     member_config.h
     enkf_defaults.h
     container_config.h
     local_dataset.h
     surface.h
     surface_config.h
     local_context.h
     enkf_plot_data.h
     enkf_plot_tvector.h
     enkf_plot_gendata.h
     enkf_plot_genvector.h
     enkf_plot_gen_kw.h
     enkf_plot_gen_kw_vector.h
     hook_manager.h
     runpath_list.h
     ert_workflow_list.h
     analysis_iter_config.h
     ecl_refcase_list.h
     local_obsdata_node.h
     local_obsdata.h
     pca_plot_data.h
     pca_plot_vector.h
     state_map.h
     summary_key_set.h
     summary_key_matcher.h
     cases_config.h
     state_map.h
     ert_test_context.h
     ert_log.h
     run_arg.h
     run_arg_type.h
     ert_run_context.h
     ert_init_context.h
     custom_kw.h
     custom_kw_config.h
     custom_kw_config_set.h
     forward_load_context.h
)


add_library( enkf  ${LIBRARY_TYPE} ${source_files} )
set_target_properties( enkf PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR} )

set( STATOIL_SITE_CONFIG /project/res/etc/ERT/site-config)
if (STATOIL_TESTDATA_ROOT)
   set( SITE_CONFIG_FILE ${STATOIL_SITE_CONFIG} CACHE FILEPATH "Path to global ERT Configuration file")
else()
   if (EXISTS ${STATOIL_SITE_CONFIG})
      set( SITE_CONFIG_FILE ${STATOIL_SITE_CONFIG})
   else()
      set( SITE_CONFIG_FILE "${PROJECT_SOURCE_DIR}/share/site-config")
   endif()
endif()

set_source_files_properties( site_config.c PROPERTIES COMPILE_DEFINITIONS "SITE_CONFIG_FILE=\"${SITE_CONFIG_FILE}\"")

target_link_libraries( enkf ecl sched analysis rms config job_queue )

if (USE_RUNPATH)
   add_runpath( enkf )
endif()

if(ERT_MAC)
    add_custom_target(ert_share_symlinking ALL
        COMMENT "Symlinking to the development share directory."
        COMMAND ln -sfh ${PROJECT_SOURCE_DIR}/share ${PROJECT_BINARY_DIR}/share
    )

else()
    add_custom_target(ert_share_symlinking ALL
    	COMMENT "Symlinking to the development share directory."
        COMMAND ln -sf -t ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/share
    )
endif()

#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS enkf DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/enkf/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/enkf)
   endforeach()
endif()

#-----------------------------------------------------------------


