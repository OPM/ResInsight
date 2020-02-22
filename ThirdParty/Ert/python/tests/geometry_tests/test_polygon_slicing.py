from math import sqrt
from ecl.util.geometry.geometry_tools import GeometryTools
from tests import EclTest


class PolygonSlicingTest(EclTest):

    def test_slicing_internal_hull(self):
        polygon = [(2,2),(2,1),(1,1),(1,5),(5,5),(5,4),(4,4)]
        edge =  [(0,0) , (10,0) , (10,10), (0,10) , (0,0)]

        sliced = GeometryTools.slicePolygon(edge , polygon)
        expected = [(2,2),(2,1),(1,1),(1,5),(5,5),(5,4),(4,4),(2.0,4.0),(2,2)]
        self.assertEqual(sliced, expected)



    def test_line_to_ray(self):
        p0 = (0.0, 0.0)
        p1 = (1.0, 1.0)
        p2 = (1.0, 0.0)

        ray = GeometryTools.lineToRay(p0, p1)
        self.assertEqual(ray, (1.0 / sqrt(2.0), 1.0 / sqrt(2.0)))

        ray = GeometryTools.lineToRay(p1, p0)
        self.assertEqual(ray, (-1.0 / sqrt(2.0), -1.0 / sqrt(2.0)))

        ray = GeometryTools.lineToRay(p0, p2)
        self.assertEqual(ray, (1.0, 0.0))


    def test_ray_line_intersection(self):
        p0 = (0.0, 0.0)
        p1 = (0.0, 1.0)
        p2 = (1.0, 1.0)
        p3 = (1.0, 0.0)
        p5 = (2.0, 1.0)

        ray1 = GeometryTools.lineToRay(p0, p2)
        ray2 = GeometryTools.lineToRay(p2, p0)
        ray3 = GeometryTools.lineToRay(p0, p5)

        self.assertEqual((0.5, 0.5), GeometryTools.rayLineIntersection(p0, ray1, p1, p3))
        self.assertIsNone(GeometryTools.rayLineIntersection(p0, ray2, p1, p3)) #ray2 is ray1 reversed (no backwards intersections)

        self.assertEqual((1.0, 0.5), GeometryTools.rayLineIntersection(p0, ray3, p2, p3))
        self.assertIsNone(GeometryTools.rayLineIntersection(p0, ray3, p1, p2))




    def test_slicing_short_line_segment(self):
        p0 = (0.0, 0.0)
        p1 = (0.0, 1.0)
        p2 = (1.0, 1.0)
        p3 = (1.0, 0.0)
        polygon = [p0, p1, p2, p3, p0]
        lp0 = (0.2, 0.5)
        lp1 = (0.4, 0.5)
        line = [lp0, lp1]
    
        result = GeometryTools.slicePolygon(polygon, line)
    
        expected = [(0.0, 0.5), p1, p2, (1.0, 0.5), lp1, lp0, (0.0, 0.5)]
    
        self.assertEqual(result, expected)


    def test_slicing_bendy_line_segments(self):
        p0 = (0.0, 0.0)
        p1 = (0.0, 1.0)
        p2 = (1.0, 1.0)
        p3 = (1.0, 0.0)
        polygon = [p0, p1, p2, p3, p0]

        lp0 = (0.2, 0.5)
        lp1 = (0.4, 0.5)
        lp2 = (0.4, 0.3)
        line = [lp0, lp1, lp2]

        expected = [(0.0, 0.5), p1, p2, p3, (0.4, 0.0), lp2, lp1, lp0, (0.0, 0.5)]

        result = GeometryTools.slicePolygon(polygon, line)
        self.assertEqual(result, expected)

        line = [lp2, lp1, lp0]
        result = GeometryTools.slicePolygon(polygon, line)
        self.assertEqual(result, expected)


    def test_slicing_same_segment(self):
        p0 = (0.0, 0.0)
        p1 = (0.0, 1.0)
        p2 = (1.0, 1.0)
        p3 = (1.0, 0.0)
        polygon = [p0, p1, p2, p3, p0]

        lp0 = (0.2, 0.5)
        lp1 = (0.4, 0.5)
        lp2 = (0.4, 0.3)
        lp3 = (0.2, 0.3)

        line = [lp0, lp1, lp2, lp3]
        result = GeometryTools.slicePolygon(polygon, line)
        expected = [(0.0, 0.5), (0.0, 0.3), lp3, lp2, lp1, lp0, (0.0, 0.5)]
        self.assertEqual(result, expected)

        line = [lp3, lp2, lp1, lp0]
        result = GeometryTools.slicePolygon(polygon, line)
        expected = [(0.0, 0.3), (0.0, 0.5), lp0, lp1, lp2, lp3, (0.0, 0.3)]
        self.assertEqual(result, expected)


    def test_ray_polyline_intersections(self):
        #             /.\
        #              .
        #              .
        #  (4)---------+----(3)
        #     /________:_____:_________
        #     \        .     |
        #         (1)--+----(2)
        #          |   .
        #          |   .
        #         (0)  .

        
        polygon = [(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 2.0), (-1.0, 2.0)]

        p0 = (0.5, 0.0)
        ray0 = (0.0, 1.0)
        intersections0 = GeometryTools.rayPolygonIntersections(p0, ray0, polygon)
        self.assertEqual(intersections0, [(1, (0.5, 1.0)), (3, (0.5, 2.0))])


        p1 = (1.5, 1.5)
        ray1 = (-1.0, 0.0)
        intersections1 = GeometryTools.rayPolygonIntersections(p1, ray1, polygon)
        self.assertEqual(intersections1, [(2, (1, 1.5))])
