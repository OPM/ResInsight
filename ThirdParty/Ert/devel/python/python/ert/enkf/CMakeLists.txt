set(PYTHON_SOURCES
    __init__.py
    analysis_config.py
    analysis_iter_config.py
    block_obs.py
    ecl_config.py
    enkf_fs.py
    enkf_fs_manager.py
    enkf_main.py
    enkf_obs.py
    enkf_simulation_runner.py
    enkf_state.py
    ens_config.py
    ert_template.py
    ert_templates.py
    ert_workflow_list.py
    local_config.py
    model_config.py
    plot_config.py
    site_config.py
    state_map.py
)

add_python_package("python.ert.enkf"  ${PYTHON_INSTALL_PREFIX}/ert/enkf "${PYTHON_SOURCES}" True)

add_subdirectory(data)
add_subdirectory(ensemble_data)
add_subdirectory(enums)
add_subdirectory(observations)
add_subdirectory(plot)
add_subdirectory(util)
