set(PYTHON_SOURCES
    __init__.py
    driver.py
    ext_job.py
    ext_joblist.py
    forward_model.py
    job.py
    job_status_type_enum.py
    queue.py
)

add_python_package("Python ert.job_queue"  ${PYTHON_INSTALL_PREFIX}/ert/job_queue "${PYTHON_SOURCES}" True)

