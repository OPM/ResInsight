set(TEST_SOURCES
    __init__.py
    test_sched.py
)

add_python_package("python.tests.ert.sched" ${PYTHON_INSTALL_PREFIX}/tests/ert/sched "${TEST_SOURCES}" False)

addPythonTest(ert.sched.sched tests.ert.sched.test_sched.SchedFileTest LABELS StatoilData)