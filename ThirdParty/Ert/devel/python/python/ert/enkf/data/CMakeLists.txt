set(PYTHON_SOURCES
    __init__.py
    enkf_config_node.py
    enkf_node.py
    gen_data_config.py
    gen_kw_config.py
    field.py
    field_config.py
    summary_config.py
)

add_python_package("Python ert.enkf.data" ${PYTHON_INSTALL_PREFIX}/ert/enkf/data "${PYTHON_SOURCES}" True)