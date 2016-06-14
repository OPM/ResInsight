set(PYTHON_SOURCES
    __init__.py
)

add_python_package("opm"  ${PYTHON_INSTALL_PREFIX}/opm "${PYTHON_SOURCES}" True)

add_subdirectory( parser )
add_subdirectory( deck )
add_subdirectory( ecl_state )


configure_file(opm_lib_info_build.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/opm/__opm_lib_info.py )
configure_file(opm_lib_info_install.py.in ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/opm_lib_info_install.py )
install(FILES ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/opm_lib_info_install.py DESTINATION ${PYTHON_INSTALL_PREFIX}/opm RENAME __opm_lib_info.py)


# The cmake variable OPM_SITE_INIT can optionally point to a python
# source file with site specific initialization. This file will be
# renamed 'opm_site_init.py' and copied to opm/ directory in the
# Python tree. The opm/__init__.py file contains the statement:
#
# try:
#    import opm_site_init
# except ImportError:
#    pass
#
# which will try to import this module.

set( OPM_SITE_INIT  ""  CACHE FILE "Name of optional external site init module for top level opm package")
if (EXISTS ${OPM_SITE_INIT})
   message(STATUS "Using opm site init module: ${OPM_SITE_INIT}")
   install(FILES ${OPM_SITE_INIT} DESTINATION ${PYTHON_INSTALL_PREFIX}/opm RENAME opm_site_init.py)
endif()
