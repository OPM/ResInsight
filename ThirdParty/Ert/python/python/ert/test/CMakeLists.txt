set(PYTHON_SOURCES
    __init__.py
    ert_test_context.py
    ert_test_runner.py
    extended_testcase.py
    test_run.py
    source_enumerator.py
    test_area.py
    temp_area.py
    path_context.py
    lint_test_case.py
)

add_python_package("python.ert.test"  ${PYTHON_INSTALL_PREFIX}/ert/test "${PYTHON_SOURCES}" True)

add_subdirectory(ecl_mock)
