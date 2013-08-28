set(PYTHON_SOURCES
    __init__.py
    sched_file.py
    history.py
)

add_python_package("Python ert.sched"  ${PYTHON_INSTALL_PREFIX}/ert/sched "${PYTHON_SOURCES}" True)
