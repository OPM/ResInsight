set(PYTHON_SOURCES
    __init__.py
    gert_main.py
    main_window.py
    newconfig.py
)

add_python_package("Python ert_gui"  ${PYTHON_INSTALL_PREFIX}/ert_gui "${PYTHON_SOURCES}" True)

add_subdirectory(models)
add_subdirectory(pages)
add_subdirectory(widgets)
