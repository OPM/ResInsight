set(PYTHON_SOURCES
    __init__.py
    data_fetcher.py
    ensemble_data_fetcher.py
    observation_data_fetcher.py
    plot_data_fetcher.py
    refcase_data_fetcher.py
)

add_python_package("python.ert.enkf.plot"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/plot "${PYTHON_SOURCES}" True)

add_subdirectory(data)
