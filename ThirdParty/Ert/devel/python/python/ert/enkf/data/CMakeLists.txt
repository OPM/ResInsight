set(PYTHON_SOURCES
    __init__.py
    custom_kw.py
    custom_kw_config.py
    enkf_config_node.py
    enkf_node.py
    field.py
    field_config.py
    gen_data.py
    gen_data_config.py
    gen_kw.py
    gen_kw_config.py
    summary_config.py
)

add_python_package("python.ert.enkf.data"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/data "${PYTHON_SOURCES}" True)

