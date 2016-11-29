set (TEST_SOURCES
    __init__.py
    test_schedule.py)

add_python_package("python.tests.ecl_state.schedule" "${PYTHON_INSTALL_PREFIX}/tests/ecl_state/schedule" "${TEST_SOURCES}" False)

addPythontest( python.test_schedule tests.ecl_state.schedule.test_schedule.ScheduleTest )
