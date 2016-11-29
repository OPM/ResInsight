set(PYTHON_SOURCES
    __init__.py
    ensemble_plot_data.py
    ensemble_plot_data_vector.py
    ensemble_plot_gen_data.py
    ensemble_plot_gen_data_vector.py
    ensemble_plot_gen_kw.py
    ensemble_plot_gen_kw_vector.py
    pca_plot_data.py
    pca_plot_vector.py
    plot_block_data.py
    plot_block_data_loader.py
    plot_block_vector.py
)

add_python_package("python.ert.enkf.plot_data"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/plot_data "${PYTHON_SOURCES}" True)

