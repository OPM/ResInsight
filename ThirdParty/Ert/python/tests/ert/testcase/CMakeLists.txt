set(TEST_SOURCES
    __init__.py
    test_testcase.py
    test_mini_config.py
)

add_python_package("python.tests.ert.testcase" ${PYTHON_INSTALL_PREFIX}/tests/ert/testcase "${TEST_SOURCES}" False)

addPythonTest(ert.testcase.testcase tests.ert.testcase.test_testcase.TestTestCase)
addPythonTest(ert.testcase.mini_config tests.ert.testcase.test_mini_config.MiniConfigTest)