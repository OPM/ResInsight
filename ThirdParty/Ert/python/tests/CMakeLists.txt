set(TEST_SOURCES
    __init__.py
    ctest_import.py
    ctest_run.py
    import_tester.py
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


add_subdirectory(core)
add_subdirectory(share)

if(BUILD_ERT)
    add_subdirectory(ert)

    add_test( NAME python.tests.ert.import
              WORKING_DIRECTORY ${INSTALL_DIRECTORY}
              COMMAND tests/ctest_import.py ${INSTALL_DIRECTORY} "ert")

    if (ERT_BUILD_GUI)
        add_subdirectory(gui)

        add_test( NAME python.tests.ert_gui.import
                  WORKING_DIRECTORY ${INSTALL_DIRECTORY}
                  COMMAND tests/ctest_import.py ${INSTALL_DIRECTORY} "ert_gui")
    endif()


endif()

