set(TEST_SOURCES
    __init__.py
    test_mda_es_plugin.py
)

add_python_package("python.tests.plugins" ${PYTHON_INSTALL_PREFIX}/tests/plugins "${TEST_SOURCES}" False)

addPythonTest(plugins.mda_es plugins.test_mda_es_plugin.MDAEnsembleSmootherPluginTest)