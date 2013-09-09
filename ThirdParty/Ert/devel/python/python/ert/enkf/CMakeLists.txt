add_subdirectory( data )
add_subdirectory( util )

set(PYTHON_SOURCES
    __init__.py
    analysis_config.py
    block_obs.py
    ecl_config.py
    enkf_enum.py
    enkf_fs.py
    enkf_main.py
    enkf_obs.py
    enkf_state.py
    ens_config.py
    ert_template.py
    ert_templates.py
    local_config.py
    model_config.py
    plot_config.py
    site_config.py
)

add_python_package("Python ert.enkf" ${PYTHON_INSTALL_PREFIX}/ert/enkf "${PYTHON_SOURCES}" True)


