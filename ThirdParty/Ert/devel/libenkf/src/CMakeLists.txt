set( source_files ert_report.c time_map.c rng_config.c trans_func.c enkf_types.c enkf_obs.c obs_data.c block_obs.c enkf_config_node.c field_config.c field.c ecl_static_kw.c enkf_state.c enkf_util.c enkf_node.c gen_kw_config.c gen_kw.c enkf_fs.c fs_driver.c meas_data.c summary_obs.c summary.c summary_config.c gen_data_config.c gen_data.c gen_common.c gen_obs.c enkf_sched.c enkf_serialize.c ecl_config.c enkf_defaults.c ensemble_config.c model_config.c site_config.c active_list.c obs_vector.c field_trans.c  plain_driver.c local_ministep.c local_updatestep.c container_config.c container.c local_context.c local_config.c analysis_config.c misfit_ensemble.c misfit_member.c misfit_ts.c data_ranking.c misfit_ranking.c ranking_table.c fs_types.c block_fs_driver.c  plot_config.c ert_template.c member_config.c enkf_analysis.c enkf_main.c local_dataset.c local_obsset.c surface.c surface_config.c enkf_plot_data.c enkf_plot_member.c qc_config.c ert_report_list.c enkf_plot_arg.c)

set( header_files ert_report.h time_map.h rng_config.h enkf_analysis.h enkf_fs_type.h trans_func.h enkf_obs.h obs_data.h enkf_config_node.h block_obs.h field_config.h field.h enkf_macros.h ecl_static_kw.h enkf_state.h enkf_util.h enkf_main.h enkf_node.h enkf_fs.h gen_kw_config.h gen_kw.h enkf_types.h fs_driver.h  meas_data.h summary_obs.h summary_config.h summary_config.h gen_data_config.h gen_data.h gen_common.h gen_obs.h enkf_sched.h fs_types.h enkf_serialize.h plain_driver.h ecl_config.h ensemble_config.h model_config.h site_config.h active_list.h obs_vector.h field_trans.h plain_driver.h local_ministep.h container.h local_updatestep.h local_config.h analysis_config.h misfit_ensemble.h misfit_ensemble_typedef.h misfit_ts.h misfit_member.h data_ranking.h ranking_table.h ranking_common.h misfit_ranking.h block_fs_driver.h field_common.h gen_kw_common.h gen_data_common.h plot_config.h ert_template.h member_config.h enkf_defaults.h container_config.h local_dataset.h local_obsset.h surface.h surface_config.h local_context.h enkf_plot_data.h enkf_plot_member.h qc_config.h ert_report_list.h enkf_plot_arg.h)

add_library( enkf  ${LIBRARY_TYPE} ${source_files} )
set_target_properties( enkf PROPERTIES VERSION 1.0 SOVERSION 1.0 )

#configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/CMake/config/ert_build_info.h.in
#                ${CMAKE_CURRENT_BINARY_DIR}/ert_build_info.h )
#list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/ert_build_info.h )

target_link_libraries( enkf ecl sched analysis rms plot config job_queue )

#-----------------------------------------------------------------
install(TARGETS enkf DESTINATION ${CMAKE_INSTALL_LIBDIR})
foreach(header ${header_files})
    install(FILES ../include/ert/enkf/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/enkf)
endforeach()



