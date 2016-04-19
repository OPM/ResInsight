set(PYTHON_SOURCES
    __init__.py
    color_chooser.py
    data_type_keys_list_model.py
    data_type_keys_widget.py
    data_type_proxy_model.py
    filter_popup.py
    plot_case_model.py
    plot_case_selection_widget.py
    plot_tool.py
    plot_widget.py
    plot_window.py
    style_chooser.py
)

add_python_package("python.ert_gui.tools.plot" ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/plot "${PYTHON_SOURCES}" True)

add_subdirectory(customize)
