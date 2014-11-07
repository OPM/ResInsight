set(PYTHON_SOURCES
    __init__.py
    configuration_panel.py
    ide_panel.py
    ide_tool.py
    ide_window.py
)

add_python_package("python.ert_gui.tools.ide"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/ide "${PYTHON_SOURCES}" True)

