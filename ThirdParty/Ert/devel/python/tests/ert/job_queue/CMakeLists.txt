set(TEST_SOURCES
    __init__.py
    test_ert_plugin.py
    test_ert_script.py
    test_function_ert_script.py
    test_job_queue.py
    test_workflow.py
    test_workflow_job.py
    test_workflow_joblist.py
    test_workflow_runner.py
    workflow_common.py
)

add_python_package("python.tests.ert.job_queue" ${PYTHON_INSTALL_PREFIX}/tests/ert/job_queue "${TEST_SOURCES}" False)

addPythonTest(ert.job_queue.job_status_type_enum tests.ert.job_queue.test_job_queue.JobQueueTest)
addPythonTest(ert.job_queue.workflow_job tests.ert.job_queue.test_workflow_job.WorkflowJobTest)
addPythonTest(ert.job_queue.workflow tests.ert.job_queue.test_workflow.WorkflowTest)
addPythonTest(ert.job_queue.workflow_joblist tests.ert.job_queue.test_workflow_joblist.WorkflowJoblistTest)
addPythonTest(ert.job_queue.ert_plugin tests.ert.job_queue.test_ert_plugin.ErtPluginTest)
addPythonTest(ert.job_queue.ert_script tests.ert.job_queue.test_ert_script.ErtScriptTest)
addPythonTest(ert.job_queue.function_ert_script tests.ert.job_queue.test_function_ert_script.FunctionErtScriptTest)
addPythonTest(ert.job_queue.workflow_runner tests.ert.job_queue.test_workflow_runner.WorkflowRunnerTest)