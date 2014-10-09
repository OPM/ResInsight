from ert.geo.geometry_tools import GeometryTools
from ert.test.extended_testcase import ExtendedTestCase


class ConvexHullTest(ExtendedTestCase):

    def test_ccw(self):
        p1 = (0, 0)
        p2 = (1, 0)
        p3 = (0, 1)
        p4 = (0, 2)


        self.assertTrue(GeometryTools.ccw(p1, p2, p3) > 0) # Counter-clockwise
        self.assertTrue(GeometryTools.ccw(p1, p3, p2) < 0) # Clockwise
        self.assertTrue(GeometryTools.ccw(p1, p3, p4) == 0) # Colinear


    def test_convex_hull(self):
        points = [(0, 0), (0, 1), (1, 1), (1, 0), (1, 1), (0.5, 0.5), (0.25, 0.25), (0.5, 1.25), (0.5, 0.75)]
        result = GeometryTools.convexHull(points)
        self.assertEqual(result, [(0, 0), (0, 1), (0.5, 1.25), (1, 1), (1, 0)])


        points = [(0, -0.5), (0, 0.5), (-0.5, 0), (0.5, 0), (0, 0), (0.5, 0.5)]
        result = GeometryTools.convexHull(points)
        self.assertEqual(result, [(-0.5, 0), (0, 0.5), (0.5, 0.5), (0.5, 0), (0, -0.5)])