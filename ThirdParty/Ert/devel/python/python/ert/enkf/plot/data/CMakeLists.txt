set(PYTHON_SOURCES
    __init__.py
    dict_property.py
    plot_data.py
    sample.py
    sample_list.py
    sample_list_collection.py
    sample_statistics.py
)

add_python_package("python.ert.enkf.plot.data"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/plot/data "${PYTHON_SOURCES}" True)

