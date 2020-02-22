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

add_python_package("python.tests.geometry_tests" ${PYTHON_INSTALL_PREFIX}/tests/geometry_tests "${TEST_SOURCES}" False)

addPythonTest(tests.geometry_tests.test_geo_pointset.GeoPointsetTest)
addPythonTest(tests.geometry_tests.test_geo_region.GeoRegionTest)
addPythonTest(tests.geometry_tests.test_surface.SurfaceTest)
addPythonTest(tests.geometry_tests.test_polyline.PolylineTest)
addPythonTest(tests.geometry_tests.test_intersection.IntersectionTest)
addPythonTest(tests.geometry_tests.test_convex_hull.ConvexHullTest)
addPythonTest(tests.geometry_tests.test_point_in_polygon.PointInPolygonTest)
addPythonTest(tests.geometry_tests.test_polygon_slicing.PolygonSlicingTest)
addPythonTest(tests.geometry_tests.test_cpolyline.CPolylineTest)
addPythonTest(tests.geometry_tests.test_cpolyline_collection.CPolylineCollectionTest)
addPythonTest(tests.geometry_tests.test_geometry_tools.GeometryToolsTest )
