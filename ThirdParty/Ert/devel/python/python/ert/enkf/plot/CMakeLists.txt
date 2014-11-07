set(PYTHON_SOURCES
    __init__.py
    block_observation_data_fetcher.py
    data_fetcher.py
    ensemble_block_data_fetcher.py
    ensemble_data_fetcher.py
    ensemble_gen_data_fetcher.py
    ensemble_gen_kw_fetcher.py
    observation_data_fetcher.py
    observation_gen_data_fetcher.py
    pca_fetcher.py
    refcase_data_fetcher.py
)

add_python_package("python.ert.enkf.plot"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/plot "${PYTHON_SOURCES}" True)

