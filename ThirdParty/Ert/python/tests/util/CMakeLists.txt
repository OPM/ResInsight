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
    #test_substitution_list.py
    test_thread_pool.py
    test_cthread_pool.py
    test_ui_return.py
    test_vectors.py
    test_version.py
    test_work_area.py
    test_path_context.py
    test_path_fmt.py
    test_arg_pack.py
    test_spawn.py
    test_log.py
)

add_python_package("python.tests.util"  ${PYTHON_INSTALL_PREFIX}/tests/util "${TEST_SOURCES}" False)

addPythonTest(tests.util.test_ctime.CTimeTest)
addPythonTest(tests.util.test_cstring.CStringTest)
addPythonTest(tests.util.test_hash.HashTest)
addPythonTest(tests.util.test_lookup_table.LookupTableTest )
addPythonTest(tests.util.test_matrix.MatrixTest )
addPythonTest(tests.util.test_rng.RngTest )
addPythonTest(tests.util.test_stat.StatTest )
addPythonTest(tests.util.test_string_list.StringListTest)
addPythonTest(tests.util.test_vectors.UtilTest)
addPythonTest(tests.util.test_ui_return.UIReturnTest)
addPythonTest(tests.util.test_path_fmt.PathFmtTest)
addPythonTest(tests.util.test_work_area.WorkAreaTest)
addPythonTest(tests.util.test_version.VersionTest)
addPythonTest(tests.util.test_path_context.PathContextTest)
addPythonTest(tests.util.test_thread_pool.ThreadPoolTest)
addPythonTest(tests.util.test_cthread_pool.CThreadPoolTest)
addPythonTest(tests.util.test_arg_pack.ArgPackTest)
addPythonTest(tests.util.test_spawn.SpawnTest)
#addPythonTest(tests.util.test_substitution_list.SubstitutionListTest)
addPythonTest(tests.util.test_log.LogTest)
