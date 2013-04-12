add_executable( job_loadOK job_loadOK.c )
add_executable( job_loadFail job_loadFail.c )
add_executable( create_file create_file.c )
add_executable( job_workflow_test job_workflow_test.c )


target_link_libraries( job_workflow_test job_queue )
target_link_libraries( create_file job_queue  )
target_link_libraries( job_loadOK job_queue  )
target_link_libraries( job_loadFail job_queue  )


add_test( job_workflow_test ${EXECUTABLE_OUTPUT_PATH}/job_workflow_test ${EXECUTABLE_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/data/internal_job)

add_test( job_loadOK1 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/internalOK)
add_test( job_loadOK2 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/externalOK)
add_test( job_loadOK3 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/internalOK ${CMAKE_CURRENT_SOURCE_DIR}/data/externalOK)

add_test( job_loadFail1 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/internalFail)
add_test( job_loadFail2 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/externalFail)
add_test( job_loadFail3 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/internalFail ${CMAKE_CURRENT_SOURCE_DIR}/data/externalFail)


add_executable( job_queue_test job_queue_test.c )
target_link_libraries( job_queue_test job_queue )
add_test( job_queue_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_test )


# This should be a space separated list of servers which will be 
# tried out when testing the LSF submit capability. The test program
# will interpret the special string 'NULL' as submit with library 
# functions.
set(LSF_SERVERS "LOCAL NULL" CACHE STRING  "List of LSF servers for testing")


add_executable( job_lsf_test job_lsf_test.c )
target_link_libraries( job_lsf_test job_queue util )

add_executable( job_lsf_submit_test job_lsf_submit_test.c )
target_link_libraries( job_lsf_submit_test job_queue util )

add_executable( job_program job_program.c )

if (HAVE_LSF_LIBRARY)
   add_executable( job_lsb job_lsb.c )
   target_link_libraries( job_lsb job_queue util )
   add_test( job_lsb ${EXECUTABLE_OUTPUT_PATH}/job_lsb )
endif()


add_test( job_lsf_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_test )
if (LSF_SERVERS)
   add_test( job_lsf_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_program ${LSF_SERVERS})
endif()
