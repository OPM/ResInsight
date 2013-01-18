add_executable( ecl_coarse_test ecl_coarse_test.c )
target_link_libraries( ecl_coarse_test ecl )

add_executable( ecl_restart_test ecl_restart_test.c )
target_link_libraries( ecl_restart_test ecl )

add_test( ecl_coarse_test  ${EXECUTABLE_OUTPUT_PATH}/ecl_coarse_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/LGCcase/LGC_TESTCASE2.EGRID )
add_test( ecl_restart_test ${EXECUTABLE_OUTPUT_PATH}/ecl_restart_test ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST )

set_property( TEST ecl_coarse_test PROPERTY LABELS Statoil )
set_property( TEST ecl_restart_test PROPERTY LABELS Statoil )
