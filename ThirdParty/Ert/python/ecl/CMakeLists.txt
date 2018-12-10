set(PYTHON_SOURCES
    __init__.py
    ecl_type.py
    ecl_util.py
)
add_python_package("python.ecl"  ${PYTHON_INSTALL_PREFIX}/ecl "${PYTHON_SOURCES}" True)

add_subdirectory(eclfile)
add_subdirectory(grid)
add_subdirectory(rft)
add_subdirectory(gravimetry)
add_subdirectory(summary)
add_subdirectory(util)
add_subdirectory(well)


if (INSTALL_ERT_LEGACY)
  add_subdirectory(ecl)
endif()

configure_file(ecl_lib_info_build.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ecl/__ecl_lib_info.py )
configure_file(ecl_lib_info_install.py.in ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ecl_lib_info_install.py )
install(FILES ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ecl_lib_info_install.py DESTINATION ${PYTHON_INSTALL_PREFIX}/ecl RENAME __ecl_lib_info.py)
