set(PYTHON_SOURCES
    __init__.py
    driver.py
    ert_plugin.py
    ert_script.py
    ext_job.py
    ext_joblist.py
    external_ert_script.py
    forward_model.py
    function_ert_script.py
    job.py
    job_status_type_enum.py
    queue.py
    job_queue_manager.py
    workflow.py
    workflow_job.py
    workflow_joblist.py
    workflow_runner.py
)

add_python_package("python.ert.job_queue"  ${PYTHON_INSTALL_PREFIX}/ert/job_queue "${PYTHON_SOURCES}" True)

