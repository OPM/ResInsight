set(TEST_SOURCES
    __init__.py
    test_run.py
)

add_python_package("python.tests.ert.run" ${PYTHON_INSTALL_PREFIX}/tests/ert/run "${TEST_SOURCES}" False)

addPythonTest(ert.test_run tests.ert.run.test_run.RunTest )