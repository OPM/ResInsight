set(TEST_SOURCES
    __init__.py
)

add_python_package("python.tests" "${PYTHON_INSTALL_PREFIX}/tests" "${TEST_SOURCES}" False)
set(CTEST_PYTHONPATH ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX})

add_subdirectory(geometry)
add_subdirectory(util)
add_subdirectory(ecl)
add_subdirectory(well)
add_subdirectory(cwrap)
add_subdirectory(global)

if (INSTALL_ERT_LEGACY)
   add_subdirectory(legacy)
endif()


configure_file( test_install.in ${EXECUTABLE_OUTPUT_PATH}/test_install @ONLY )
