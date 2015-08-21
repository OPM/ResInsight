set(TEST_SOURCES
    __init__.py
    ert_shell_test_context.py
    test_ertshell_analysis.py
    test_ertshell_context.py
    test_ertshell_observations.py
    test_ertshell_smoother.py
)

add_python_package("python.tests.gui.ertshell"  ${PYTHON_INSTALL_PREFIX}/tests/gui/ertshell "${TEST_SOURCES}" False)

addPythonTest(gui.ertshell.ertshell_analysis gui.ertshell.test_ertshell_analysis.ErtShellAnalysisModuleTest)
addPythonTest(gui.ertshell.ertshell_context gui.ertshell.test_ertshell_context.ErtShellContextTest)
addPythonTest(gui.ertshell.ertshell_observations gui.ertshell.test_ertshell_observations.ErtShellObservationsTest)
addPythonTest(gui.ertshell.ertshell_smoother gui.ertshell.test_ertshell_smoother.ErtShellSmootherTest)