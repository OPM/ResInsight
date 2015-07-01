set(TEST_SOURCES
    __init__.py
    test_plot_metrics_tracker.py
)

add_python_package("python.tests.gui.plot" ${PYTHON_INSTALL_PREFIX}/tests/gui/plot "${TEST_SOURCES}" False)

addPythonTest(gui.plot.plot_metrics_tracker tests.gui.plot.test_plot_metrics_tracker.PlotMetricTrackerTest)