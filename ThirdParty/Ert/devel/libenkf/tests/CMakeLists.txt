add_executable( enkf_runpath_list enkf_runpath_list.c )
target_link_libraries( enkf_runpath_list enkf )

add_executable( enkf_site_config enkf_site_config.c )
target_link_libraries( enkf_site_config enkf )

add_executable( enkf_time_map enkf_time_map.c )
target_link_libraries( enkf_time_map enkf )

add_executable( enkf_ensemble_GEN_PARAM enkf_ensemble_GEN_PARAM.c )
target_link_libraries( enkf_ensemble_GEN_PARAM enkf )

add_executable( enkf_ensemble enkf_ensemble.c )
target_link_libraries( enkf_ensemble enkf )

add_executable( enkf_main enkf_main.c )
target_link_libraries( enkf_main enkf )
add_test( enkf_main  ${EXECUTABLE_OUTPUT_PATH}/enkf_main )

#-----------------------------------------------------------------

add_executable( enkf_forward_init_GEN_KW enkf_forward_init_GEN_KW.c )
target_link_libraries( enkf_forward_init_GEN_KW enkf )
add_test( enkf_forward_init_GEN_KW_TRUE   ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_KW ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/config_GEN_KW_true   TRUE)
add_test( enkf_forward_init_GEN_KW_FALSE  ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_KW ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/config_GEN_KW_false FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_GEN_PARAM enkf_forward_init_GEN_PARAM.c )
target_link_libraries( enkf_forward_init_GEN_PARAM enkf )
add_test( enkf_forward_init_GEN_PARAM_TRUE   ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_PARAM ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/config_GEN_PARAM_true   TRUE)
add_test( enkf_forward_init_GEN_PARAM_FALSE  ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_GEN_PARAM ${CMAKE_CURRENT_SOURCE_DIR}/data/config/forward/config_GEN_PARAM_false FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_SURFACE enkf_forward_init_SURFACE.c )
target_link_libraries( enkf_forward_init_SURFACE enkf )

add_test( enkf_forward_init_SURFACE_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_SURFACE 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/config_surface_true   
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/Surface.irap
          TRUE)

add_test( enkf_forward_init_SURFACE_FALSE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_SURFACE 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/config_surface_false
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/surface/Surface.irap
          FALSE)

#-----------------------------------------------------------------

add_executable( enkf_forward_init_FIELD enkf_forward_init_FIELD.c )
target_link_libraries( enkf_forward_init_FIELD enkf )

add_test( enkf_forward_init_FIELD_TRUE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_FIELD 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/config_field_true   
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/petro.grdecl
          TRUE)

add_test( enkf_forward_init_FIELD_FALSE   
          ${EXECUTABLE_OUTPUT_PATH}/enkf_forward_init_FIELD 
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/config_field_false
          ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/forward_init/field/petro.grdecl
          FALSE)

#-----------------------------------------------------------------


add_executable( enkf_iter_config enkf_iter_config.c )
target_link_libraries( enkf_iter_config enkf )
add_test( enkf_iter_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_iter_config )

add_executable( enkf_model_config enkf_model_config.c )
target_link_libraries( enkf_model_config enkf )
add_test( enkf_model_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_model_config )

add_executable( enkf_rng enkf_rng.c )
target_link_libraries( enkf_rng enkf )
add_test( enkf_rng  ${EXECUTABLE_OUTPUT_PATH}/enkf_rng ${CMAKE_CURRENT_SOURCE_DIR}/data/config/rng)

add_executable( enkf_report_list enkf_report_list.c )
target_link_libraries( enkf_report_list enkf )
add_test( enkf_report_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_report_list ${CMAKE_CURRENT_SOURCE_DIR}/data/config/ert_report_list)

add_executable( enkf_refcase_list enkf_refcase_list.c )
target_link_libraries( enkf_refcase_list enkf )
add_test( enkf_refcase_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_refcase_list ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat*/ECLIPSE) 
add_test( enkf_refcase_list2  ${EXECUTABLE_OUTPUT_PATH}/enkf_refcase_list ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat*/ECLIPSE.*) 
set_property( TEST enkf_refcase_list PROPERTY LABELS StatoilData )
set_property( TEST enkf_refcase_list2 PROPERTY LABELS StatoilData )


add_executable( enkf_ecl_config enkf_ecl_config.c )
target_link_libraries( enkf_ecl_config enkf )
add_test( enkf_ecl_config1  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config )
add_test( enkf_ecl_config2  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE)
set_property( TEST enkf_ecl_config2 PROPERTY LABELS StatoilData )

add_executable( enkf_ecl_config_config enkf_ecl_config_config.c )
target_link_libraries( enkf_ecl_config_config enkf )
add_test( enkf_ecl_config_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_ecl_config_config ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/ecl_config )
set_property( TEST enkf_ecl_config_config PROPERTY LABELS StatoilData )

add_test( enkf_runpath_list  ${EXECUTABLE_OUTPUT_PATH}/enkf_runpath_list )
add_test( enkf_site_config  ${EXECUTABLE_OUTPUT_PATH}/enkf_site_config /project/res/etc/ERT/site-config)
add_test( enkf_time_map1  ${EXECUTABLE_OUTPUT_PATH}/enkf_time_map )
add_test( enkf_time_map2  ${EXECUTABLE_OUTPUT_PATH}/enkf_time_map ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST )
add_test( enkf_ensemble_GEN_PARAM  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble_GEN_PARAM ${CMAKE_CURRENT_SOURCE_DIR}/data/ensemble/GEN_PARAM )
add_test( enkf_ensemble  ${EXECUTABLE_OUTPUT_PATH}/enkf_ensemble )


set_property( TEST enkf_time_map2     PROPERTY LABELS StatoilData )
set_property( TEST enkf_site_config   PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_SURFACE_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_SURFACE_TRUE   PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_FIELD_FALSE  PROPERTY LABELS StatoilData )
set_property( TEST enkf_forward_init_FIELD_TRUE   PROPERTY LABELS StatoilData )

