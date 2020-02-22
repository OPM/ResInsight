set(TEST_SOURCES
    __init__.py
    test_summary_resample.py
)

add_python_package("python.tests.bin_tests"  ${PYTHON_INSTALL_PREFIX}/tests/bin_tests "${TEST_SOURCES}" False)

addPythonTest(tests.bin_tests.test_summary_resample.SummaryResampleTest)
