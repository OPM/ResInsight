set(TEST_SOURCES
    __init__.py
    test_synthesizer.py
)

add_python_package("python.tests.share" ${PYTHON_INSTALL_PREFIX}/tests/share "${TEST_SOURCES}" False)

addPythonTest(share.test_synthesizer.SynthesizerTest)
