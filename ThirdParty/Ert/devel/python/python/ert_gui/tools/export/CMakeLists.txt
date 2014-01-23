set(PYTHON_SOURCES
    __init__.py
    export_tool.py
)

add_python_package("python.ert_gui.tools.export"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/export "${PYTHON_SOURCES}" True)

