set(PYTHON_SOURCES
    __init__.py
    help_tool.py
    help_window.py
)

add_python_package("python.ert_gui.tools.help"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/help "${PYTHON_SOURCES}" True)

