add_executable( job_status_test job_status_test.c )
add_executable( job_loadOK job_loadOK.c )
add_executable( job_loadFail job_loadFail.c )
add_executable( job_node_test job_node_test.c )
add_executable( job_list_test job_list_test.c )
add_executable( create_file create_file.c )
add_executable( job_workflow_test job_workflow_test.c )
add_executable( job_lsf_parse_bsub_stdout job_lsf_parse_bsub_stdout.c )
add_executable( ext_joblist_test ext_joblist_test.c )


target_link_libraries( job_status_test job_queue test_util )
target_link_libraries( job_workflow_test job_queue test_util )
target_link_libraries( create_file job_queue  test_util )
target_link_libraries( job_loadOK job_queue  test_util )
target_link_libraries( job_loadFail job_queue  test_util )
target_link_libraries( ext_joblist_test job_queue  test_util )
target_link_libraries( job_lsf_parse_bsub_stdout job_queue test_util)
target_link_libraries( job_node_test job_queue test_util)
target_link_libraries( job_list_test job_queue test_util)

add_test( job_status_test ${EXECUTABLE_OUTPUT_PATH}/job_status_test)
add_test( job_lsf_parse_bsub_stdout ${EXECUTABLE_OUTPUT_PATH}/job_lsf_parse_bsub_stdout )

add_test( job_workflow_test ${EXECUTABLE_OUTPUT_PATH}/job_workflow_test ${EXECUTABLE_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/data/internal_job)

add_test( job_loadOK1 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/internalOK)
add_test( job_loadOK2 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/externalOK)
add_test( job_loadOK3 ${EXECUTABLE_OUTPUT_PATH}/job_loadOK ${CMAKE_CURRENT_SOURCE_DIR}/data/internalOK ${CMAKE_CURRENT_SOURCE_DIR}/data/externalOK)

add_test( job_loadFail1 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/internalFail)
add_test( job_loadFail2 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/externalFail)
add_test( job_loadFail3 ${EXECUTABLE_OUTPUT_PATH}/job_loadFail ${CMAKE_CURRENT_SOURCE_DIR}/data/internalFail ${CMAKE_CURRENT_SOURCE_DIR}/data/externalFail)

add_test( job_node_test ${EXECUTABLE_OUTPUT_PATH}/job_node_test)
add_test( job_list_test valgrind --leak-check=full --error-exitcode=1 ${EXECUTABLE_OUTPUT_PATH}/job_list_test)
add_test( ext_joblist_test ${EXECUTABLE_OUTPUT_PATH}/ext_joblist_test ${CMAKE_CURRENT_SOURCE_DIR}/data/jobs/util ${CMAKE_CURRENT_SOURCE_DIR})

add_executable( job_program_output job_program_output.c )
target_link_libraries( job_program_output ert_util test_util)

add_executable( job_queue_test job_job_queue_test.c )
target_link_libraries( job_queue_test job_queue test_util )
add_test( job_queue_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_test ${EXECUTABLE_OUTPUT_PATH}/job_program_output )

add_executable( job_queue_stress_task job_queue_stress_task.c )
target_link_libraries( job_queue_stress_task  ert_util )

add_executable( job_queue_stress_test job_queue_stress_test.c )
target_link_libraries( job_queue_stress_test job_queue test_util )
add_test( job_queue_stress_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_stress_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_stress_task False)
add_test( job_queue_user_exit ${EXECUTABLE_OUTPUT_PATH}/job_queue_stress_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_stress_task True)



add_executable( job_queue_driver_test job_queue_driver_test.c )
target_link_libraries( job_queue_driver_test job_queue test_util )
add_test( job_queue_driver_test ${EXECUTABLE_OUTPUT_PATH}/job_queue_driver_test )


if (ERT_LSF_SUBMIT_TEST)
   include( lsf_tests.cmake ) 
endif()


add_executable( job_torque_test job_torque_test.c )
target_link_libraries( job_torque_test job_queue ert_util test_util )
add_test( job_torque_test ${EXECUTABLE_OUTPUT_PATH}/job_torque_test )

add_executable( job_queue_manager job_queue_manager.c )
target_link_libraries( job_queue_manager job_queue ert_util test_util )
add_test( job_queue_manager ${EXECUTABLE_OUTPUT_PATH}/job_queue_manager )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/qsub_emulators/ DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_executable( job_torque_submit_test job_torque_submit_test.c )
target_link_libraries( job_torque_submit_test job_queue ert_util test_util )
add_test(NAME job_torque_submit_test WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH} COMMAND ${EXECUTABLE_OUTPUT_PATH}/job_torque_submit_test dummyparam)
set_property(TEST job_torque_submit_test PROPERTY ENVIRONMENT “setenv PATH ${EXECUTABLE_OUTPUT_PATH}:$PATH”)
set_property(TEST ext_joblist_test PROPERTY LABELS StatoilData )


