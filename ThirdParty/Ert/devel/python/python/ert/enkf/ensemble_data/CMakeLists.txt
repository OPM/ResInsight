set(PYTHON_SOURCES
    __init__.py
    ensemble_plot_data.py
    ensemble_plot_data_vector.py
)

add_python_package("python.ert.enkf.ensemble_data"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/ensemble_data "${PYTHON_SOURCES}" True)

