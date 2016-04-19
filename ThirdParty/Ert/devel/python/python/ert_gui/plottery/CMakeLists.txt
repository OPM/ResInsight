set(PYTHON_SOURCES
    __init__.py
    plot_config.py
    plot_config_history.py
    plot_config_factory.py
    plot_context.py
    plot_data_gatherer.py
    plot_limits.py
    plot_style.py
)

add_python_package("python.ert_gui.plottery" ${PYTHON_INSTALL_PREFIX}/ert_gui/plottery "${PYTHON_SOURCES}" True)

add_subdirectory(plots)
