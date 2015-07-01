set(TEST_SOURCES
    __init__.py
    test_plot_data.py
)

add_python_package("python.tests.ert.enkf.plot" ${PYTHON_INSTALL_PREFIX}/tests/ert/enkf/plot "${TEST_SOURCES}" False)

addPythonTest(ert.enkf.plot.plot_block_data tests.ert.enkf.plot.test_plot_data.PlotDataTest LABELS StatoilData)