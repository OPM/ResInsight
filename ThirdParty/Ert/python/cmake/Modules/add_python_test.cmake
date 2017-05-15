# This macro will create a ctest based on the supplied TEST_CLASS. The
# TEST_CLASS argument should correspond to a valid Python path, i.e.
#
# >> import ${TEST_CLASS} 
#
# should work. The actual test is by running a small test script which
# will invoke normal Python test discovery functionality. This is a
# macro, and relevant variables must be crrectly set in calling scope
# before it is invoked:
#
#   PYTHON_TEST_RUNNER: Path to executable which will load the testcase
#      given by ${TEST_CLASS} and run it.
#
#
#   CTEST_PYTHONPATH: Normal colon separated path variable, should at
#      least include the binary root directory of the current python
#      installation, but can in addition contain the path to
#      additional packages. The PYTHON_TEST_RUNNER should inspect the
#      $CTEST_PYTHONPATH environment variable and update sys.path
#      accordingly.

macro( addPythonTest TEST_CLASS )
    set(TEST_NAME ${TEST_CLASS})

    add_test(NAME ${TEST_NAME}
             WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}"
             COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_BINARY_DIR}/bin/ctest_run_python ${TEST_CLASS} )

    set(oneValueArgs LABELS)
    cmake_parse_arguments(TEST_OPTIONS "" "${oneValueArgs}" "" ${ARGN})
    if(TEST_OPTIONS_LABELS)
        set_property(TEST ${TEST_NAME} PROPERTY LABELS "Python:${TEST_OPTIONS_LABELS}")
    else()
        set_property(TEST ${TEST_NAME} PROPERTY LABELS "Python")
    endif()

    set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "CTEST_PYTHONPATH=${CTEST_PYTHONPATH}")
endmacro( )
