function (opm_add_python_test TEST_NAME TEST_SCRIPT)
    add_test(NAME ${TEST_NAME} 
             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
             COMMAND ${TEST_SCRIPT} ${ARGN})

           set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "PYTHONPATH=${ERT_PYTHON_PATH}:${CWRAP_PYTHON_PATH}:${PYTHONPATH}")
endfunction(opm_add_python_test)
