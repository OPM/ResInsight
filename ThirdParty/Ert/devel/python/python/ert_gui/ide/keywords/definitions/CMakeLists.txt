set(PYTHON_SOURCES
    __init__.py
    argument_definition.py
    bool_argument.py
    configuration_line_definition.py
    float_argument.py
    integer_argument.py
    keyword_definition.py
    number_list_string_argument.py
    path_argument.py
    percent_argument.py
    proper_name_argument.py
    proper_name_format_argument.py
    proper_name_format_string_argument.py
    range_string_argument.py
    string_argument.py
)

add_python_package("python.ert_gui.ide.keywords.definitions" ${PYTHON_INSTALL_PREFIX}/ert_gui/ide/keywords/definitions "${PYTHON_SOURCES}" True)

