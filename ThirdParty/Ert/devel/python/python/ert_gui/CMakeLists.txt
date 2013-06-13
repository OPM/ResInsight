add_python_package( "Python ert_gui" ${PYTHON_INSTALL_PREFIX}/ert_gui "__init__.py;gert_main.py;newconfig.py" True)
add_subdirectory( pages )
add_subdirectory( widgets )
