set (TEST_SOURCES
    __init__.py
    test_parse_context.py)

add_python_package("python.tests.parse_context" "${PYTHON_INSTALL_PREFIX}/tests/parse_context" "${TEST_SOURCES}" False)

addPythontest( python.test_parse_context tests.parse_context.test_parse_context.ParseContextTest )
