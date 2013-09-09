set(PYTHON_SOURCES
    __init__.py
    buffer.py
    ctime.py
    hash.py
    latex.py
    log.py
    lookup_table.py
    matrix.py
    stat.py
    stringlist.py
    substitution_list.py
    tvector.py
    util_func.py

)


add_python_package("Python ert.util" ${PYTHON_INSTALL_PREFIX}/ert/util "${PYTHON_SOURCES}" T)

add_python_package("Python ert.util test" ${PYTHON_INSTALL_PREFIX}/ert/util "test_area.py" F)
