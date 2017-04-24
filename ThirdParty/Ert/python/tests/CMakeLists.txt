set(TEST_SOURCES
    __init__.py
    ctest_import.py
    ctest_run.py
    import_tester.py
    test_pylint.py
)

set(TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(INSTALL_DIRECTORY ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX})

add_python_package("python.tests" "${PYTHON_INSTALL_PREFIX}/tests" "${TEST_SOURCES}" False)

function (addPythonTest TEST_NAME TEST_CLASS)
    set(oneValueArgs LABELS)
    set(multiValueArgs ARGUMENTS ENVIRONMENT)
    cmake_parse_arguments(TEST_OPTIONS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(NAME python.tests.${TEST_NAME}
             WORKING_DIRECTORY ${INSTALL_DIRECTORY}
             COMMAND tests/ctest_run.py ${INSTALL_DIRECTORY} ${TEST_CLASS} ${TEST_OPTIONS_ARGUMENTS})

    if(TEST_OPTIONS_LABELS)
        set_property(TEST python.tests.${TEST_NAME} PROPERTY LABELS "Python:${TEST_OPTIONS_LABELS}")
    else()
        set_property(TEST python.tests.${TEST_NAME} PROPERTY LABELS "Python")
    endif()

    if(TEST_OPTIONS_ENVIRONMENT)
        set_property(TEST python.tests.${TEST_NAME} PROPERTY ENVIRONMENT ${TEST_OPTIONS_ENVIRONMENT})
    endif()
endfunction(addPythonTest)


add_subdirectory(geometry)
add_subdirectory(util)
add_subdirectory(ecl)
add_subdirectory(well)
add_subdirectory(cwrap)
addPythonTest(lint test_pylint.LintErt)
