set(PYTHON_SOURCES
    __init__.py
    clearable_line_edit.py
    custom_date_edit.py
    copy_style_to_dialog.py
)

add_python_package("python.ert_gui.tools.plot.widgets" ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/plot/widgets "${PYTHON_SOURCES}" True)

