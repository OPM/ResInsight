set (TEST_SOURCES
    __init__.py
    test_table_manager.py)

add_python_package("python.tests.ecl_state.table" "${PYTHON_INSTALL_PREFIX}/tests/ecl_state/table" "${TEST_SOURCES}" False)

addPythontest( python.test_table_manager tests.ecl_state.table.test_table_manager.TableManagerTest )
