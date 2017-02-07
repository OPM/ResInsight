set(PYTHON_SOURCES
    __init__.py
    run_analysis_panel.py
    run_analysis_tool.py
)

add_python_package("python.ert_gui.tools.run_analysis" ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/run_analysis "${PYTHON_SOURCES}" True)
