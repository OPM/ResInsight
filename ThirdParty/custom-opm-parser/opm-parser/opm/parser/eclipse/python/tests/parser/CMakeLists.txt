set (TEST_SOURCES
    __init__.py
    test_parser.py)

add_python_package("python.tests.parser" "${PYTHON_INSTALL_PREFIX}/tests/parser" "${TEST_SOURCES}" False)

addPythontest( python.test_parser tests.parser.test_parser.ParserTest )
