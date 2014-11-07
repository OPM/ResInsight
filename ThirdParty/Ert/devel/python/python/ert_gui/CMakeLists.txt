set(PYTHON_SOURCES
    __init__.py
    ert_splash.py
    gert_main.py
    ide_test.py
    main_window.py
    newconfig.py
    about_dialog.py
)

add_python_package("python.ert_gui"  ${PYTHON_INSTALL_PREFIX}/ert_gui "${PYTHON_SOURCES}" True)

add_subdirectory(ide)
add_subdirectory(models)
add_subdirectory(pages)
add_subdirectory(simulation)
add_subdirectory(tools)
add_subdirectory(viewer)
add_subdirectory(widgets)

#-----------------------------------------------------------------

set(site_config_target "${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/ert_gui/site_config.py")
set(destination "${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PREFIX}/ert_gui")
set(install_target "${destination}/site_config.py") 

configure_file(site_config.py.in ${site_config_target})
install(FILES ${site_config_target} DESTINATION ${destination} )
install(CODE "execute_process(COMMAND ${PROJECT_SOURCE_DIR}/cmake/cmake_pyc_file ${install_target})")

