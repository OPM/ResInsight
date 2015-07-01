from ert.geo import GeometryTools
from ert.test.extended_testcase import ExtendedTestCase


class IntersectionTest(ExtendedTestCase):

    def test_intersection(self):

        p1 = (0.0, 0.0)
        p2 = (10.0, 0.0)
        p3 = (5.0, -5.0)
        p4 = (5.0, 5.0)

        self.assertEqual(GeometryTools.lineIntersection(p1, p2, p3, p4), (5.0, 0.0))

        p5 = (0.0, 5.0)
        self.assertEqual(GeometryTools.lineIntersection(p1, p2, p3, p5), (2.5, 0))


        self.assertEqual(GeometryTools.lineIntersection((0.0, 0.0), (1.0, 1.0), (0.0, 1.0), (1.0, 0.0)), (0.5, 0.5))


    def test_coincident(self):
        p1 = (0.0, 0.0)
        p2 = (10.0, 10.0)

        self.assertIsNone( GeometryTools.lineIntersection(p1, p2, p1, p2) )
        

    def test_parallel(self):
        p1 = (0.0, 0.0)
        p2 = (10.0, 0.0)

        p3 = (0.0, 1.0)
        p4 = (10.0, 1.0)

        self.assertIsNone(GeometryTools.lineIntersection(p1, p2, p3, p4))


    def test_intersection_outside_segments(self):
        p1 = (0.0, 0.0)
        p2 = (10.0, 0.0)

        p3 = (-1.0, -1.0)
        p4 = (-1.0, 1.0)

        self.assertIsNone(GeometryTools.lineIntersection(p1, p2, p3, p4))
