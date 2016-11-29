set(TEST_SOURCES
    __init__.py
    test_config.py
)

add_python_package("python.tests.ert.config" ${PYTHON_INSTALL_PREFIX}/tests/ert/config "${TEST_SOURCES}" True)

addPythonTest(ert.config tests.ert.config.test_config.ConfigTest)