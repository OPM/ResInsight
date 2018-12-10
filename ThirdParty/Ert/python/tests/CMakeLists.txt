configure_file(
   __init__.py 
   "${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/tests/__init__.py"
   @ONLY
)

set(CTEST_PYTHONPATH ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX})

add_subdirectory(geometry_tests)
add_subdirectory(util_tests)
add_subdirectory(ecl_tests)
add_subdirectory(well_tests)
add_subdirectory(global_tests)
add_subdirectory(bin_tests)

if (INSTALL_ERT_LEGACY)
   add_subdirectory(legacy_tests)
endif()

configure_file( test_install.in ${EXECUTABLE_OUTPUT_PATH}/test_install @ONLY )
