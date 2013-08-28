set(PYTHON_SOURCES
    __init__.py
    driver.py
    job.py
    queue.py
    ext_job.py
    ext_joblist.py
    forward_model.py
)

add_python_package("Python ert.job_queue" ${PYTHON_INSTALL_PREFIX}/ert/job_queue "${PYTHON_SOURCES}" True)
