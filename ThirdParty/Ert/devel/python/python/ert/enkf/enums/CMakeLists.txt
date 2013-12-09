set(PYTHON_SOURCES
    __init__.py
    enkf_init_modes_enum.py
    enkf_obs_impl_type_enum.py
    enkf_run_enum.py
    enkf_state_type_enum.py
    enkf_var_type_enum.py
    ert_impl_type_enum.py
    load_fail_type_enum.py
    realization_state_enum.py
)

add_python_package("python.ert.enkf.enums"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/enums "${PYTHON_SOURCES}" True)

