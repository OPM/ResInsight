set(PYTHON_SOURCES
    __init__.py
    ccsp.py
    distribution.py
    ensemble.py
    gaussian_kde.py
    histogram.py
    history.py
    observations.py
    plot_tools.py
    refcase.py
    statistics.py
)

add_python_package("python.ert_gui.plottery.plots" ${PYTHON_INSTALL_PREFIX}/ert_gui/plottery/plots "${PYTHON_SOURCES}" True)

