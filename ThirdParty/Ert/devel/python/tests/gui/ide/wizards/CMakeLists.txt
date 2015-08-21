set(TEST_SOURCES
    __init__.py
    test_tree_structure.py
)

add_python_package("python.tests.gui.ide.wizards"  ${PYTHON_INSTALL_PREFIX}/tests/gui/ide/wizards "${TEST_SOURCES}" False)

addPythonTest(gui.ide.wizards.tree_structure tests.gui.ide.wizards.test_tree_structure.TreeStructureTest)