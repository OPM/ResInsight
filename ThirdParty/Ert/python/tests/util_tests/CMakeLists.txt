set(TEST_SOURCES
    __init__.py
    test_ctime.py
    test_hash.py
    test_lookup_table.py
    test_rng.py
    test_string_list.py
    #test_substitution_list.py
    test_thread_pool.py
    test_vectors.py
    test_version.py
    test_work_area.py
    test_path_context.py
    test_spawn.py
)

add_python_package("python.tests.util_tests"  ${PYTHON_INSTALL_PREFIX}/tests/util_tests "${TEST_SOURCES}" False)

addPythonTest(tests.util_tests.test_ctime.CTimeTest)
addPythonTest(tests.util_tests.test_hash.HashTest)
addPythonTest(tests.util_tests.test_lookup_table.LookupTableTest )
addPythonTest(tests.util_tests.test_rng.RngTest )
addPythonTest(tests.util_tests.test_string_list.StringListTest)
addPythonTest(tests.util_tests.test_vectors.UtilTest)
addPythonTest(tests.util_tests.test_work_area.WorkAreaTest)
addPythonTest(tests.util_tests.test_version.VersionTest)
addPythonTest(tests.util_tests.test_path_context.PathContextTest)
addPythonTest(tests.util_tests.test_thread_pool.ThreadPoolTest)
addPythonTest(tests.util_tests.test_spawn.SpawnTest)
#addPythonTest(tests.util.test_substitution_list.SubstitutionListTest)
