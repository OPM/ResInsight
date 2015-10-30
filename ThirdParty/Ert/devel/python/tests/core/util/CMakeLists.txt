set(TEST_SOURCES
    __init__.py
    test_ctime.py
    test_hash.py
    #test_latex.py
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
)

add_python_package("python.tests.core.util"  ${PYTHON_INSTALL_PREFIX}/tests/core/util "${TEST_SOURCES}" False)

addPythonTest(core.util.ctime core.util.test_ctime.CTimeTest)
addPythonTest(core.util.hash core.util.test_hash.HashTest)
addPythonTest(core.util.lookup_table core.util.test_lookup_table.LookupTableTest )
addPythonTest(core.util.matrix core.util.test_matrix.MatrixTest )
addPythonTest(core.util.rng core.util.test_rng.RngTest )
addPythonTest(core.util.stat core.util.test_stat.StatTest )
addPythonTest(core.util.stringlist core.util.test_string_list.StringListTest)
addPythonTest(core.util.tvector core.util.test_vectors.UtilTest)
addPythonTest(core.util.ui_return core.util.test_ui_return.UIReturnTest)
addPythonTest(core.util.work_area core.util.test_work_area.WorkAreaTest)
addPythonTest(core.util.version core.util.test_version.VersionTest)
addPythonTest(core.util.path_context core.util.test_path_context.PathContextTest)
addPythonTest(core.util.thread_pool core.util.test_thread_pool.ThreadPoolTest)
addPythonTest(core.util.cthread_pool core.util.test_cthread_pool.CThreadPoolTest)
addPythonTest(core.util.arg_pack core.util.test_arg_pack.ArgPackTest)

#add_test( NAME python.tests.ert.util.latex
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} core.util.test_latex.LatexTest )
