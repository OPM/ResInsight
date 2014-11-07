from ert.geo.geometry_tools import GeometryTools
from ert.geo.polyline import Polyline
from ert.test.extended_testcase import ExtendedTestCase


class PointInPolygonTest(ExtendedTestCase):

    def test_point_in_polygon(self):
        p1 = (0.5, 0.5)
        p2 = (2, 2)
        p3 = (1, 0.5) # on the edge

        poly1 = [(0, 0), (1, 0), (1, 1), (0, 1)] # Not explicitly closed
        poly2 = [(0, 0), (1, 0), (1, 1), (0, 1), (0, 0)] # explicitly closed

        self.assertTrue(GeometryTools.pointInPolygon(p1, poly1))
        self.assertTrue(GeometryTools.pointInPolygon(p1, poly2))

        self.assertFalse(GeometryTools.pointInPolygon(p2, poly1))
        self.assertFalse(GeometryTools.pointInPolygon(p2, poly2))

        self.assertTrue(GeometryTools.pointInPolygon(p3, poly1))


    def test_point_in_polyline(self):
        p1 = (0.5, 0.5)
        p2 = (2, 2)

        poly = Polyline()
        poly.addPoint(0, 0)
        poly.addPoint(1, 0)
        poly.addPoint(1, 1)
        poly.addPoint(0, 1)
        poly.addPoint(0, 0)

        self.assertTrue(GeometryTools.pointInPolygon(p1, poly))
        self.assertTrue(GeometryTools.pointInPolygon(p1, poly))

        self.assertFalse(GeometryTools.pointInPolygon(p2, poly))
        self.assertFalse(GeometryTools.pointInPolygon(p2, poly))


    def test_point_in_strange_polygon(self):
        p1 = (0.5, 0.51)
        p2 = (0.5, 0.49)

        poly = [(0,0), (0, 1), (0.6, 0.5), (0.4, 0.5), (1, 1), (1, 0)]

        self.assertFalse(GeometryTools.pointInPolygon(p1, poly))
        self.assertTrue(GeometryTools.pointInPolygon(p2, poly))


    def test_point_in_polygon_with_3_element_points(self):
        p1 = (0.5, 0.51, 0.2)
        p2 = (0.5, 0.49, 0.1)

        poly = [(0,0,9), (0,1,9), (0.6,0.5), (0.4,0.5,9), (1,1), (1,0,9)]

        self.assertFalse(GeometryTools.pointInPolygon(p1, poly))
        self.assertTrue(GeometryTools.pointInPolygon(p2, poly))