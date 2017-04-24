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

addPythonTest(geometry.geo_pointset tests.geometry.test_geo_pointset.GeoPointsetTest)
addPythonTest(geometry.geo_region tests.geometry.test_geo_region.GeoRegionTest)
addPythonTest(geometry.surface tests.geometry.test_surface.SurfaceTest)
addPythonTest(geometry.polyline tests.geometry.test_polyline.PolylineTest)
addPythonTest(geometry.intersection tests.geometry.test_intersection.IntersectionTest)
addPythonTest(geometry.convex_hull tests.geometry.test_convex_hull.ConvexHullTest)
addPythonTest(geometry.point_in_polygon tests.geometry.test_point_in_polygon.PointInPolygonTest)
addPythonTest(geometry.polygon_slicing tests.geometry.test_polygon_slicing.PolygonSlicingTest)
addPythonTest(geometry.cpolyline tests.geometry.test_cpolyline.CPolylineTest)
addPythonTest(geometry.cpolyline_collection tests.geometry.test_cpolyline_collection.CPolylineCollectionTest)
addPythonTest(geometry.geometry_tools tests.geometry.test_geometry_tools.GeometryToolsTest )
