set(PYTHON_SOURCES
    __init__.py
    configuration_panel.py
    message_center.py
    popup_dialog.py
    queue_system_configuration.py
    run_dialog.py
)

add_python_package("python.ert_gui.pages"  ${PYTHON_INSTALL_PREFIX}/ert_gui/pages "${PYTHON_SOURCES}" True)

