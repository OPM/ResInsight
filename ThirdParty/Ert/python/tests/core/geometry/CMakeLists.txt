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
)

add_python_package("python.tests.core.geometry" ${PYTHON_INSTALL_PREFIX}/tests/core/geometry "${TEST_SOURCES}" False)

addPythonTest(core.geometry.surface tests.core.geometry.test_surface.SurfaceTest)
addPythonTest(core.geometry.polyline tests.core.geometry.test_polyline.PolylineTest)
addPythonTest(core.geometry.intersection tests.core.geometry.test_intersection.IntersectionTest)
addPythonTest(core.geometry.convex_hull tests.core.geometry.test_convex_hull.ConvexHullTest)
addPythonTest(core.geometry.point_in_polygon tests.core.geometry.test_point_in_polygon.PointInPolygonTest)
addPythonTest(core.geometry.polygon_slicing tests.core.geometry.test_polygon_slicing.PolygonSlicingTest)
addPythonTest(core.geometry.cpolyline tests.core.geometry.test_cpolyline.CPolylineTest)
addPythonTest(core.geometry.cpolyline_collection tests.core.geometry.test_cpolyline_collection.CPolylineCollectionTest)
addPythonTest(core.geometry.geometry_tools tests.core.geometry.test_geometry_tools.GeometryToolsTest )
