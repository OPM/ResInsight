set(PYTHON_SOURCES
    __init__.py
    table_index.py
    table.py
    table_manager.py
)

add_python_package("opm.ecl_state.tables"  ${PYTHON_INSTALL_PREFIX}/opm/ecl_state/tables "${PYTHON_SOURCES}" True)
