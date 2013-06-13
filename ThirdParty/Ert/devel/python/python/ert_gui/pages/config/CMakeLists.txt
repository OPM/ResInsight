add_python_package( "Python ert_gui.pages.config" ${PYTHON_INSTALL_PREFIX}/ert_gui/pages/config "__init__.py;analysis.py;configpages.py;eclipse.py;ensemble.py;observations.py;plot.py;queuesystem.py;simulation.py;systemenv.py" True)
add_subdirectory( jobs )
add_subdirectory( parameters )
add_subdirectory( simulations )
