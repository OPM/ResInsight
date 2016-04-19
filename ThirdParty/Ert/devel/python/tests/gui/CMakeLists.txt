set(TEST_SOURCES
    __init__.py
    test_model.py
    test_observable.py
)

add_python_package("python.tests.gui"  ${PYTHON_INSTALL_PREFIX}/tests/gui "${TEST_SOURCES}" False)

addPythonTest(gui.model tests.gui.test_model.ModelTest)
addPythonTest(gui.observable tests.gui.test_observable.ObservableTest)

add_subdirectory(ertshell)
add_subdirectory(ide)
add_subdirectory(plottery)


