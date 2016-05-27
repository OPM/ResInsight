set (TEST_SOURCES
    __init__.py
    test_table_manager.py)

add_python_package("python.tests.ecl_state" "${PYTHON_INSTALL_PREFIX}/tests/ecl_state" "${TEST_SOURCES}" False)

addPythontest( python.test_table_manager tests.ecl_state.test_table_manager.TableManagerTest )
