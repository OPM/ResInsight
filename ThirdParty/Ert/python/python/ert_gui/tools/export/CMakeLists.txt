set(PYTHON_SOURCES
    __init__.py
    export_panel.py
    export_tool.py
    exporter.py
    export_keyword_model.py
    export_model.py
)

add_python_package("python.ert_gui.tools.export"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/export "${PYTHON_SOURCES}" True)

