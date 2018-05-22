set(TEST_SOURCES
    __init__.py
    test_import.py
    test_pylint.py
)

add_python_package("python.tests.global_tests"  "${PYTHON_INSTALL_PREFIX}/tests/global_tests" "${TEST_SOURCES}" False)

addPythonTest(tests.global_tests.test_import.ImportEcl)
addPythonTest(tests.global_tests.test_pylint.LintErt)

 
