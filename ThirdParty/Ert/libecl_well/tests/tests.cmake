add_executable( well_conn_collection well_conn_collection.c )
target_link_libraries( well_conn_collection ecl_well  )
set_target_properties( well_conn_collection PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_conn_collection ${EXECUTABLE_OUTPUT_PATH}/well_conn_collection )

add_executable( well_branch_collection well_branch_collection.c )
target_link_libraries( well_branch_collection ecl_well  )
set_target_properties( well_branch_collection PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_branch_collection ${EXECUTABLE_OUTPUT_PATH}/well_branch_collection )

add_executable( well_conn well_conn.c )
target_link_libraries( well_conn ecl_well  )
set_target_properties( well_conn PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_conn ${EXECUTABLE_OUTPUT_PATH}/well_conn )

add_executable( well_state well_state.c )
target_link_libraries( well_state ecl_well  )
set_target_properties( well_state PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_state ${EXECUTABLE_OUTPUT_PATH}/well_state )

add_executable( well_segment well_segment.c )
target_link_libraries( well_segment ecl_well  )
set_target_properties( well_segment PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_segment ${EXECUTABLE_OUTPUT_PATH}/well_segment )

add_executable( well_segment_conn well_segment_conn.c )
target_link_libraries( well_segment_conn ecl_well  )
set_target_properties( well_segment_conn PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_segment_conn ${EXECUTABLE_OUTPUT_PATH}/well_segment_conn )

add_executable( well_segment_collection well_segment_collection.c )
target_link_libraries( well_segment_collection ecl_well  )
set_target_properties( well_segment_collection PROPERTIES COMPILE_FLAGS "-Werror")                                    
add_test( well_segment_collection ${EXECUTABLE_OUTPUT_PATH}/well_segment_collection )

