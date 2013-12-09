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
    test_area.py
    tvector.py
    util_func.py
    ui_return.py
    rng.py
)

add_python_package("python.ert.util"  ${PYTHON_INSTALL_PREFIX}/ert/util "${PYTHON_SOURCES}" True)

add_subdirectory(enums)
