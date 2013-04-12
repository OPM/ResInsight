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


add_executable( ecl_grid_simple ecl_grid_simple.c )
target_link_libraries( ecl_grid_simple ecl )
add_test( ecl_grid_simple ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_simple  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID )


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

add_executable( ecl_rsthead ecl_rsthead.c )
target_link_libraries( ecl_rsthead ecl )
add_test( ecl_rsthead ${EXECUTABLE_OUTPUT_PATH}/ecl_rsthead ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST 
                                                            ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/DualPoro/DUALPORO.X0005 ) 

set_property( TEST ecl_coarse_test PROPERTY LABELS Statoil )
set_property( TEST ecl_restart_test PROPERTY LABELS Statoil )
set_property( TEST ecl_lgr_test1 PROPERTY LABELS Statoil )
set_property( TEST ecl_lgr_test2 PROPERTY LABELS Statoil )
set_property( TEST ecl_grid_simple PROPERTY LABELS Statoil )
set_property( TEST ecl_dualp PROPERTY LABELS Statoil )
set_property( TEST ecl_sum_test PROPERTY LABELS Statoil )
