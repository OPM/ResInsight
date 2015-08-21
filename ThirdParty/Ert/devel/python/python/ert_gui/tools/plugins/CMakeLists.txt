set(PYTHON_SOURCES
    __init__.py
    plugin.py
    plugin_handler.py
    plugin_runner.py
    plugins_tool.py
    process_job_dialog.py
)

add_python_package("python.ert_gui.tools.plugins"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/plugins "${PYTHON_SOURCES}" True)

