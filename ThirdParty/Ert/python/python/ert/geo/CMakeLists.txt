set(PYTHON_SOURCES
    __init__.py
    cpolyline.py
    cpolyline_collection.py
    geometry_tools.py
    geo_pointset.py
    geo_region.py
    polyline.py
    xyz_io.py
    surface.py
)

add_python_package("python.ert.geo"  ${PYTHON_INSTALL_PREFIX}/ert/geo "${PYTHON_SOURCES}" True)
