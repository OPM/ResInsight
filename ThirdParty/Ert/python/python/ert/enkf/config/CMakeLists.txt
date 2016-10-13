set(PYTHON_SOURCES
    __init__.py
    custom_kw_config.py
    enkf_config_node.py
    field_config.py
    gen_data_config.py
    gen_kw_config.py
    summary_config.py
)

add_python_package("python.ert.enkf.config" ${PYTHON_INSTALL_PREFIX}/ert/enkf/config "${PYTHON_SOURCES}" True)

