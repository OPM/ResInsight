# This should be a space separated list of up to three servers which
# will be tried out when testing the util_ping() functionality. The
# list of servers should behave like this:
#
# 1. First server - this should be an existing server which should return true.
# 2. This should be an invalid hostname - should return false.
# 3. This should be an valid host which does not answer ping - i.e currently off?
set(PING_SERVERS "" CACHE STRING  "List of servers for testing ping")


link_directories( ${ERT_BINARY_DIR}/libert_util/src )

add_executable( ert_util_type_vector_test ert_util_type_vector_test.c )
target_link_libraries( ert_util_type_vector_test ert_util test_util )
add_test( ert_util_type_vector_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_type_vector_test )

add_executable( ert_util_filename ert_util_filename.c )
target_link_libraries( ert_util_filename ert_util test_util )
add_test( ert_util_filename ${EXECUTABLE_OUTPUT_PATH}/ert_util_filename )

add_executable( ert_util_string_util ert_util_string_util.c )
target_link_libraries( ert_util_string_util ert_util test_util )
add_test( ert_util_string_util ${EXECUTABLE_OUTPUT_PATH}/ert_util_string_util )

add_executable( ert_util_vector_test ert_util_vector_test.c )
target_link_libraries( ert_util_vector_test ert_util test_util )
add_test( ert_util_vector_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_vector_test )

add_executable( ert_util_cwd_test ert_util_cwd_test.c )
target_link_libraries( ert_util_cwd_test ert_util test_util )
add_test( ert_util_cwd_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_cwd_test ${CMAKE_CURRENT_BINARY_DIR})

add_executable( ert_util_relpath_test ert_util_relpath_test.c )
target_link_libraries( ert_util_relpath_test ert_util test_util )
add_test( ert_util_relpath_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_relpath_test )

add_executable( ert_util_path_stack_test ert_util_path_stack_test.c )
target_link_libraries( ert_util_path_stack_test ert_util test_util )
add_test( ert_util_path_stack_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_path_stack_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable( ert_util_PATH_test ert_util_PATH_test.c )
target_link_libraries( ert_util_PATH_test ert_util test_util )
add_test( ert_util_PATH_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_PATH_test )

add_executable( ert_util_strcat_test ert_util_strcat_test.c )
target_link_libraries( ert_util_strcat_test ert_util test_util )
add_test( ert_util_strcat_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_strcat_test )

add_executable( ert_util_sprintf_escape ert_util_sprintf_escape.c )
target_link_libraries( ert_util_sprintf_escape ert_util test_util )
add_test( ert_util_sprintf_escape ${EXECUTABLE_OUTPUT_PATH}/ert_util_sprintf_escape )


add_executable( ert_util_stringlist_test ert_util_stringlist_test.c )
target_link_libraries( ert_util_stringlist_test ert_util test_util )
add_test( ert_util_stringlist_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_stringlist_test )

add_executable( ert_util_realpath ert_util_realpath.c )
target_link_libraries( ert_util_realpath ert_util test_util )
add_test( ert_util_realpath ${EXECUTABLE_OUTPUT_PATH}/ert_util_realpath )

add_executable( ert_util_hash_test ert_util_hash_test.c )
target_link_libraries( ert_util_hash_test ert_util test_util )
add_test( ert_util_hash_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_hash_test )

add_executable( ert_util_binary_split ert_util_binary_split.c )
target_link_libraries( ert_util_binary_split ert_util test_util )
add_test( ert_util_binary_split ${EXECUTABLE_OUTPUT_PATH}/ert_util_binary_split )

add_executable( ert_util_logh ert_util_logh.c )
target_link_libraries( ert_util_logh ert_util test_util )
add_test( ert_util_logh ${EXECUTABLE_OUTPUT_PATH}/ert_util_logh )

add_executable( ert_util_rng ert_util_rng.c )
target_link_libraries( ert_util_rng ert_util test_util )
add_test( ert_util_rng ${EXECUTABLE_OUTPUT_PATH}/ert_util_rng )

add_executable( ert_util_time_interval ert_util_time_interval.c )
target_link_libraries( ert_util_time_interval ert_util test_util )
add_test( ert_util_time_interval ${EXECUTABLE_OUTPUT_PATH}/ert_util_time_interval )

add_executable( ert_util_before_after ert_util_before_after.c )
target_link_libraries( ert_util_before_after ert_util test_util )
add_test( ert_util_before_after ${EXECUTABLE_OUTPUT_PATH}/ert_util_before_after )

add_executable( ert_util_approx_equal ert_util_approx_equal.c )
target_link_libraries( ert_util_approx_equal ert_util test_util )
add_test( ert_util_approx_equal ${EXECUTABLE_OUTPUT_PATH}/ert_util_approx_equal )

add_executable( ert_util_ping ert_util_ping.c )
target_link_libraries( ert_util_ping ert_util test_util )
add_test( ert_util_ping ${EXECUTABLE_OUTPUT_PATH}/ert_util_ping ${PING_SERVERS})

add_executable( ert_util_file_readable ert_util_file_readable.c )
target_link_libraries( ert_util_file_readable ert_util test_util )
add_test( ert_util_file_readable ${EXECUTABLE_OUTPUT_PATH}/ert_util_file_readable ${FILE_READABLE_SERVERS})

add_executable( ert_util_type_vector_functions ert_util_type_vector_functions.c )
target_link_libraries( ert_util_type_vector_functions ert_util test_util )
add_test( ert_util_type_vector_functions ${EXECUTABLE_OUTPUT_PATH}/ert_util_type_vector_functions)

add_executable( ert_util_addr2line ert_util_addr2line.c )
target_link_libraries( ert_util_addr2line ert_util test_util )
add_test( ert_util_addr2line ${EXECUTABLE_OUTPUT_PATH}/ert_util_addr2line)

add_executable( ert_util_work_area ert_util_work_area.c )
target_link_libraries( ert_util_work_area ert_util test_util )
add_test( NAME ert_util_work_area 
          COMMAND ${EXECUTABLE_OUTPUT_PATH}/ert_util_work_area data/file1.txt ${CMAKE_CURRENT_SOURCE_DIR}/data/file2.txt data
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})



if (WITH_LATEX)
   add_executable( ert_util_latex_test ert_util_latex_test.c )
   target_link_libraries(ert_util_latex_test ert_util test_util )
   add_test( ert_util_latex_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_latex_test ${CMAKE_CURRENT_SOURCE_DIR}/data/latex_OK.tex )
endif()


add_executable( ert_util_arg_pack ert_util_arg_pack.c)
set_target_properties( ert_util_arg_pack PROPERTIES COMPILE_FLAGS "-Werror")
target_link_libraries( ert_util_arg_pack ert_util test_util )
add_test( ert_util_arg_pack ${EXECUTABLE_OUTPUT_PATH}/ert_util_arg_pack)

add_executable( ert_util_abort_gnu_tests ert_util_abort_gnu_tests.c)
target_link_libraries( ert_util_abort_gnu_tests ert_util test_util)
add_test( ert_util_abort_gnu_tests ${EXECUTABLE_OUTPUT_PATH}/ert_util_abort_gnu_tests)
