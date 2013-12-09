set(PYTHON_SOURCES
    __init__.py
    gert_main.py
    ide_test.py
    main_window.py
    newconfig.py
)

add_python_package("python.ert_gui"  ${PYTHON_INSTALL_PREFIX}/ert_gui "${PYTHON_SOURCES}" True)

add_subdirectory(ide)
add_subdirectory(models)
add_subdirectory(pages)
add_subdirectory(simulation)
add_subdirectory(tools)
add_subdirectory(widgets)
