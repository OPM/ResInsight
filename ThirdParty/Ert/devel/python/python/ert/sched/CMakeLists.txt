set(PYTHON_SOURCES
    __init__.py
    history.py
    history_source_enum.py
    sched_file.py
)

add_python_package("Python ert.sched"  ${PYTHON_INSTALL_PREFIX}/ert/sched "${PYTHON_SOURCES}" True)
