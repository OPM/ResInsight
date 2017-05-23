set(TEST_SOURCES
    __init__.py
    test_convex_hull.py
    test_cpolyline.py
    test_cpolyline_collection.py
    test_geometry_tools.py
    test_intersection.py
    test_point_in_polygon.py
    test_polygon_slicing.py
    test_polyline.py
    test_surface.py
    test_geo_pointset.py
    test_geo_region.py
)

add_python_package("python.tests.geometry" ${PYTHON_INSTALL_PREFIX}/tests/geometry "${TEST_SOURCES}" False)

addPythonTest(tests.geometry.test_geo_pointset.GeoPointsetTest)
addPythonTest(tests.geometry.test_geo_region.GeoRegionTest)
addPythonTest(tests.geometry.test_surface.SurfaceTest)
addPythonTest(tests.geometry.test_polyline.PolylineTest)
addPythonTest(tests.geometry.test_intersection.IntersectionTest)
addPythonTest(tests.geometry.test_convex_hull.ConvexHullTest)
addPythonTest(tests.geometry.test_point_in_polygon.PointInPolygonTest)
addPythonTest(tests.geometry.test_polygon_slicing.PolygonSlicingTest)
addPythonTest(tests.geometry.test_cpolyline.CPolylineTest)
addPythonTest(tests.geometry.test_cpolyline_collection.CPolylineCollectionTest)
addPythonTest(tests.geometry.test_geometry_tools.GeometryToolsTest )
