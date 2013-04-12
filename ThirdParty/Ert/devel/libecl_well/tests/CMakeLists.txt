add_executable( well_ts well_ts.c )
target_link_libraries( well_ts ecl_well )
add_test( well_ts  ${EXECUTABLE_OUTPUT_PATH}/well_ts ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/CO2case/BASE_CASE )
set_property( TEST well_ts PROPERTY LABELS Statoil )


add_executable( well_dualp well_dualp.c )
target_link_libraries( well_dualp ecl_well )
add_test( well_dualp  ${EXECUTABLE_OUTPUT_PATH}/well_dualp ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST 
                                                            ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/DualPoro/DUALPORO.X0005  )


add_executable( well_lgr_load well_lgr_load.c )
target_link_libraries( well_lgr_load ecl_well )
add_test( well_lgr_load  ${EXECUTABLE_OUTPUT_PATH}/well_lgr_load ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/0.9.2_LGR/BASE_REF_XY3Z1_T30_WI.X0003) 
set_property( TEST well_lgr_load PROPERTY LABELS Statoil )
