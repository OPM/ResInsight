#add_executable( sched_load sched_load.c )
#target_link_libraries( sched_load sched test_util )
#add_test( sched_load  ${EXECUTABLE_OUTPUT_PATH}/sched_load  ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Heidrun/Schedule.sch )

#add_executable( sched_tokenize sched_tokenize.c )
#target_link_libraries( sched_tokenize sched test_util )
#add_test( sched_tokenize  ${EXECUTABLE_OUTPUT_PATH}/sched_tokenize  ${CMAKE_CURRENT_SOURCE_DIR}/test-data/token_test1 )

add_executable( sched_history_summary sched_history_summary.c )
target_link_libraries( sched_history_summary sched test_util )
add_test( sched_history_summary1  ${EXECUTABLE_OUTPUT_PATH}/sched_history_summary ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE )
add_test( sched_history_summary2  ${EXECUTABLE_OUTPUT_PATH}/sched_history_summary ${PROJECT_SOURCE_DIR}/test-data/Statoil/ECLIPSE/Snorre/SNORRE )

#set_property( TEST sched_load PROPERTY LABELS StatoilData)
set_property( TEST sched_history_summary1 PROPERTY LABELS StatoilData)
set_property( TEST sched_history_summary2 PROPERTY LABELS StatoilData)
