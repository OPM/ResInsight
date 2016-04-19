set(PYTHON_SOURCES
    __init__.py
)
add_python_package("python.ert"  ${PYTHON_INSTALL_PREFIX}/ert "${PYTHON_SOURCES}" True)

add_subdirectory(cwrap)
add_subdirectory(ecl)
add_subdirectory(geo)
add_subdirectory(test)
add_subdirectory(util)
add_subdirectory(well)

IF (BUILD_ERT)
   set( ERT_SITE_INIT_TARGET  ""  CACHE FILE "Name of optional external site init module for top level ert package")
   if (EXISTS ${ERT_SITE_INIT_TARGET})
      message(STATUS "Using ert site init module: ${ERT_SITE_INIT_TARGET}")
      install(FILES ${ERT_SITE_INIT_TARGET} DESTINATION ${PYTHON_INSTALL_PREFIX}/ert RENAME ert_site_init.py)
   endif()
   add_subdirectory(analysis)
   add_subdirectory(config)
   add_subdirectory(enkf)
   add_subdirectory(job_queue)
   add_subdirectory(rms)
   add_subdirectory(sched)
   add_subdirectory(server)
ENDIF()


configure_file(ert_lib_info_build.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ert/__ert_lib_info.py )
configure_file(ert_lib_info_install.py.in ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ert_lib_info_install.py )
install(FILES ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ert_lib_info_install.py DESTINATION ${PYTHON_INSTALL_PREFIX}/ert RENAME __ert_lib_info.py)
