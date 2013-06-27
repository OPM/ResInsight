add_executable( ecl_coarse_test ecl_coarse_test.c )
target_link_libraries( ecl_coarse_test ecl )
add_test( ecl_coarse_test  ${EXECUTABLE_OUTPUT_PATH}/ecl_coarse_test  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/LGCcase/LGC_TESTCASE2 )


add_executable( ecl_restart_test ecl_restart_test.c )
target_link_libraries( ecl_restart_test ecl )
add_test( ecl_restart_test ${EXECUTABLE_OUTPUT_PATH}/ecl_restart_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST )



add_executable( ecl_lgr_test ecl_lgr_test.c )
target_link_libraries( ecl_lgr_test ecl )
add_test( ecl_lgr_test1    ${EXECUTABLE_OUTPUT_PATH}/ecl_lgr_test     ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/10kcase/TEST10K_FLT_LGR_NNC.EGRID)
add_test( ecl_lgr_test2    ${EXECUTABLE_OUTPUT_PATH}/ecl_lgr_test     ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/10kcase/TEST10K_FLT_LGR_NNC.GRID)
add_test( ecl_lgr_test3    ${EXECUTABLE_OUTPUT_PATH}/ecl_lgr_test     ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID )


add_executable( ecl_grid_simple ecl_grid_simple.c )
target_link_libraries( ecl_grid_simple ecl )
add_test( ecl_grid_simple ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_simple  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID )


add_executable( ecl_grid_dims ecl_grid_dims.c )
target_link_libraries( ecl_grid_dims ecl )

add_test( ecl_grid_dims0 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  )
add_test( ecl_grid_dims1 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.INIT )
add_test( ecl_grid_dims2 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.GRID  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.INIT)
add_test( ecl_grid_dims3 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID )
add_test( ecl_grid_dims4 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.GRID  ) 
add_test( ecl_grid_dims5 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_dims  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/AmalgLGRcase/TESTCASE_AMALG_LGR.EGRID 
                                                                  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/AmalgLGRcase/TESTCASE_AMALG_LGR.INIT )





add_executable( ecl_kw_grdecl ecl_kw_grdecl.c )
target_link_libraries( ecl_kw_grdecl ecl )
add_test( ecl_kw_grdecl ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_grdecl )

add_executable( ecl_kw_equal ecl_kw_equal.c )
target_link_libraries( ecl_kw_equal ecl )
add_test( ecl_kw_equal ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_equal )


add_executable( ecl_dualp ecl_dualp.c )
target_link_libraries( ecl_dualp ecl )
add_test( ecl_dualp ${EXECUTABLE_OUTPUT_PATH}/ecl_dualp  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/LGCcase/LGC_TESTCASE2 )

add_executable( ecl_util_month_range ecl_util_month_range.c )
target_link_libraries( ecl_util_month_range ecl )
add_test( ecl_util_month_range ${EXECUTABLE_OUTPUT_PATH}/ecl_util_month_range  )

add_executable( ecl_sum_test ecl_sum_test.c )
target_link_libraries( ecl_sum_test ecl )
add_test( ecl_sum_test ${EXECUTABLE_OUTPUT_PATH}/ecl_sum_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE )

add_executable( ecl_fortio ecl_fortio.c )
target_link_libraries( ecl_fortio  ecl )
add_test( ecl_fortio ${EXECUTABLE_OUTPUT_PATH}/ecl_fortio ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST )

add_executable( ecl_file ecl_file.c )
target_link_libraries( ecl_file  ecl )
add_test( ecl_file ${EXECUTABLE_OUTPUT_PATH}/ecl_file ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST ECLIPSE.UNRST)

add_executable( ecl_fmt ecl_fmt.c )
target_link_libraries( ecl_fmt  ecl )
add_test( ecl_fmt ${EXECUTABLE_OUTPUT_PATH}/ecl_fmt ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.DATA)


add_executable( ecl_rsthead ecl_rsthead.c )
target_link_libraries( ecl_rsthead ecl )
add_test( ecl_rsthead ${EXECUTABLE_OUTPUT_PATH}/ecl_rsthead ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST 
                                                            ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/DualPoro/DUALPORO.X0005 ) 

set_property( TEST ecl_fmt         PROPERTY LABELS Statoil )
set_property( TEST ecl_coarse_test PROPERTY LABELS Statoil )
set_property( TEST ecl_restart_test PROPERTY LABELS Statoil )
set_property( TEST ecl_lgr_test1 PROPERTY LABELS Statoil )
set_property( TEST ecl_lgr_test2 PROPERTY LABELS Statoil )
set_property( TEST ecl_lgr_test3 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_simple PROPERTY LABELS Statoil )
set_property( TEST ecl_dualp PROPERTY LABELS Statoil )
set_property( TEST ecl_sum_test PROPERTY LABELS Statoil )
set_property( TEST ecl_fortio PROPERTY LABELS Statoil)
set_property( TEST ecl_grid_dims1 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_dims2 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_dims3 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_dims4 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_dims5 PROPERTY LABELS Statoil )
set_property( TEST ecl_file PROPERTY LABELS Statoil)
set_property( TEST ecl_rsthead PROPERTY LABELS Statoil)
