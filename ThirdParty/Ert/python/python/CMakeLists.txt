configure_file(test_env.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/test_env.py )

add_subdirectory(cwrap)
add_subdirectory( ecl )

if (INSTALL_ERT_LEGACY)
   add_subdirectory( legacy/ert )
endif()
