set(PYTHON_SOURCES
    __init__.py
    ensemble_plot_data.py
    histogram_plot_data.py
    histogram_plot_data_factory.py
    observation_plot_data.py
    plot_data.py
    plot_data_fetcher.py
    refcase_plot_data.py
)

add_python_package("python.ert_gui.tools.plot.data"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/plot/data "${PYTHON_SOURCES}" True)

