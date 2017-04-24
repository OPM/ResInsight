configure_file(test_env.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/test_env.py )

add_subdirectory(cwrap)
add_subdirectory( ert )
add_subdirectory( bin )
