set(PYTHON_SOURCES
    __init__.py
    popup_dialog.py
    queue_system_configuration.py
    run_dialog.py
    summary_panel.py
)

add_python_package("python.ert_gui.pages"  ${PYTHON_INSTALL_PREFIX}/ert_gui/pages "${PYTHON_SOURCES}" True)

