# This should be a space separated list of servers which will be 
# tried out when testing the LSF submit capability. The test program
# will interpret the special string 'NULL' as submit with library 
# functions.
set(LSF_SERVERS "LOCAL NULL" CACHE STRING  "List of LSF servers for testing")

add_executable( loadOK loadOK.c )
target_link_libraries( loadOK job_queue util )

add_executable( job_lsf_test job_lsf_test.c )
target_link_libraries( job_lsf_test job_queue util )

add_executable( job_lsf_submit_test job_lsf_submit_test.c )
target_link_libraries( job_lsf_submit_test job_queue util )

add_executable( job_program job_program.c )

add_test( loadOK ${EXECUTABLE_OUTPUT_PATH}/loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/internalOK)
add_test( job_lsf_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_test )
if (LSF_SERVERS)
   add_test( job_lsf_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_program ${LSF_SERVERS})
endif()
