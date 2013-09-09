set(PYTHON_SOURCES
    __init__.py
    geo_polygon.py
)

add_python_package("Python ert.geo"  ${PYTHON_INSTALL_PREFIX}/ert/geo "${PYTHON_SOURCES}" True)
