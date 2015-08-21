set(PYTHON_SOURCES
    __init__.py
    test_bool_argument.py
    test_configuration_line.py
    test_configuration_line_builder.py
    test_configuration_line_definition.py
    test_configuration_line_parser.py
    test_ert_keywords.py
    test_float_argument.py
    test_integer_argument.py
    test_path_argument.py
    test_percent_argument.py
    test_proper_name_argument.py
    test_proper_name_format_argument.py
    test_proper_name_format_string_argument.py
    test_range_string_argument.py
    test_tokens.py
)

add_python_package("python.tests.gui.ide"  ${PYTHON_INSTALL_PREFIX}/tests/gui/ide "${PYTHON_SOURCES}" True)

addPythonTest(gui.ide.bool_argument tests.gui.ide.test_bool_argument.BoolArgumentTest)
addPythonTest(gui.ide.integer_argument tests.gui.ide.test_integer_argument.IntegerArgumentTest)
addPythonTest(gui.ide.float_argument tests.gui.ide.test_float_argument.FloatArgumentTest)
addPythonTest(gui.ide.path_argument tests.gui.ide.test_path_argument.PathArgumentTest)
addPythonTest(gui.ide.proper_name_argument tests.gui.ide.test_proper_name_argument.ProperNameArgumentTest)
addPythonTest(gui.ide.proper_name_format_argument tests.gui.ide.test_proper_name_format_argument.ProperNameFormatArgumentTest)
addPythonTest(gui.ide.proper_name_format_string_argument tests.gui.ide.test_proper_name_format_string_argument.ProperNameFormatStringArgumentTest)
addPythonTest(gui.ide.range_string_argument tests.gui.ide.test_range_string_argument.RangeStringArgumentTest)
addPythonTest(gui.ide.test_configuration_line tests.gui.ide.test_configuration_line.ConfigurationLineTest)
addPythonTest(gui.ide.test_configuration_line_builder tests.gui.ide.test_configuration_line_builder.ConfigurationLineBuilderTest)
addPythonTest(gui.ide.test_configuration_line_definition tests.gui.ide.test_configuration_line_definition.ConfigurationLineDefinitionTest)
addPythonTest(gui.ide.test_configuration_line_parser tests.gui.ide.test_configuration_line_parser.ConfigurationLineParserTest)
addPythonTest(gui.ide.ert_keywords tests.gui.ide.test_ert_keywords.ErtKeywordTest)
addPythonTest(gui.ide.tokens tests.gui.ide.test_tokens.TokenTest)

add_subdirectory(wizards)
