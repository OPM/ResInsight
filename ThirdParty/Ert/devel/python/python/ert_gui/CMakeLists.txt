set(PYTHON_SOURCES
    __init__.py
    about_dialog.py
    ert_splash.py
    ertplot.py
    gert_main.py
    main_window.py
    newconfig.py
)

add_python_package("python.ert_gui" ${PYTHON_INSTALL_PREFIX}/ert_gui "${PYTHON_SOURCES}" True)

add_subdirectory(ide)
add_subdirectory(models)
add_subdirectory(pages)
add_subdirectory(plottery)
add_subdirectory(shell)
add_subdirectory(simulation)
add_subdirectory(tools)
add_subdirectory(widgets)
