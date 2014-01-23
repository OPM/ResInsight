set(PYTHON_SOURCES
    __init__.py
    highlighter.py
    keyword.py
    parameter.py
)

add_python_package("python.ert_gui.ide"  ${PYTHON_INSTALL_PREFIX}/ert_gui/ide "${PYTHON_SOURCES}" True)

add_subdirectory(completers)
add_subdirectory(keywords)
add_subdirectory(wizards)
