set(PYTHON_SOURCES
    __init__.py
    run_workflow_widget.py
    workflow_dialog.py
    workflows_tool.py
)

add_python_package("python.ert_gui.tools.workflows"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/workflows "${PYTHON_SOURCES}" True)

