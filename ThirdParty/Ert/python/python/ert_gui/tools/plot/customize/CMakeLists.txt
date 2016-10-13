set(PYTHON_SOURCES
    __init__.py
    customization_view.py
    customize_plot_dialog.py
    default_customization_view.py
    limits_customization_view.py
    statistics_customization_view.py
    style_customization_view.py
)

add_python_package("python.ert_gui.tools.plot.customize" ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/plot/customize "${PYTHON_SOURCES}" True)

