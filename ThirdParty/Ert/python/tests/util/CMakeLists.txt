set(TEST_SOURCES
    __init__.py
    test_ctime.py
    test_cstring.py
    test_hash.py
    test_lookup_table.py
    test_matrix.py
    test_rng.py
    test_stat.py
    test_string_list.py
    test_substitution_list.py
    test_thread_pool.py
    test_cthread_pool.py
    test_ui_return.py
    test_vectors.py
    test_version.py
    test_work_area.py
    test_path_context.py
    test_arg_pack.py
    test_spawn.py
)

add_python_package("python.tests.util"  ${PYTHON_INSTALL_PREFIX}/tests/util "${TEST_SOURCES}" False)

addPythonTest(util.ctime util.test_ctime.CTimeTest)
addPythonTest(util.cstring util.test_cstring.CStringTest)
addPythonTest(util.hash util.test_hash.HashTest)
addPythonTest(util.lookup_table util.test_lookup_table.LookupTableTest )
addPythonTest(util.matrix util.test_matrix.MatrixTest )
addPythonTest(util.rng util.test_rng.RngTest )
addPythonTest(util.stat util.test_stat.StatTest )
addPythonTest(util.stringlist util.test_string_list.StringListTest)
addPythonTest(util.tvector util.test_vectors.UtilTest)
addPythonTest(util.ui_return util.test_ui_return.UIReturnTest)
addPythonTest(util.work_area util.test_work_area.WorkAreaTest)
addPythonTest(util.version util.test_version.VersionTest)
addPythonTest(util.path_context util.test_path_context.PathContextTest)
addPythonTest(util.thread_pool util.test_thread_pool.ThreadPoolTest)
addPythonTest(util.cthread_pool util.test_cthread_pool.CThreadPoolTest)
addPythonTest(util.arg_pack util.test_arg_pack.ArgPackTest)
addPythonTest(util.spawn util.test_spawn.SpawnTest)
