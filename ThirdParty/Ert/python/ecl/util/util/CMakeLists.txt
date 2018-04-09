set(PYTHON_SOURCES
    __init__.py
    bool_vector.py
    ctime.py
    double_vector.py
    hash.py
    int_vector.py
    install_abort_signals.py
    lookup_table.py
    rng.py
    stringlist.py
    #substitution_list.py
    thread_pool.py
    time_vector.py
    util_func.py
    vector_template.py
    permutation_vector.py
    version.py
    arg_pack.py
    cwd_context.py
)

add_python_package("python.ecl.util.util"  ${PYTHON_INSTALL_PREFIX}/ecl/util/util "${PYTHON_SOURCES}" True)

