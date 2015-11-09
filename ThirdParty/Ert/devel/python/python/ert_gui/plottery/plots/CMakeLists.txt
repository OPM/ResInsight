set(PYTHON_SOURCES
    __init__.py
    ensemble.py
    gaussian_kde.py
    histogram.py
    observations.py
    overview.py
    plot_tools.py
    refcase.py
    statistics.py
)

add_python_package("python.ert_gui.plottery.plots" ${PYTHON_INSTALL_PREFIX}/ert_gui/plottery/plots "${PYTHON_SOURCES}" True)

