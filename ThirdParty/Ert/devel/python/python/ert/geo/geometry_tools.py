from math import sqrt


class GeometryTools(object):
    EPSILON = 0.000001

    @staticmethod
    def lineIntersection(p1, p2, p3, p4):
        """
        Finds intersection between line segments. Returns None if no intersection found.
        Algorithm provided by Paul Bourke

        @type p1: tuple of (float, float)
        @type p2: tuple of (float, float)
        @type p3: tuple of (float, float)
        @type p4: tuple of (float, float)
        @rtype: tuple of (float, float)
        """

        denominator = (p4[1] - p3[1]) * (p2[0] - p1[0]) - (p4[0] - p3[0]) * (p2[1] - p1[1])
        numerator_a = (p4[0] - p3[0]) * (p1[1] - p3[1]) - (p4[1] - p3[1]) * (p1[0] - p3[0])
        numerator_b = (p2[0] - p1[0]) * (p1[1] - p3[1]) - (p2[1] - p1[1]) * (p1[0] - p3[0])

        # coincident?
        if abs(numerator_a) < GeometryTools.EPSILON and abs(numerator_b) < GeometryTools.EPSILON and abs(denominator) < GeometryTools.EPSILON:
            x = (p1[0] + p2[0]) / 2.0
            y = (p1[1] + p2[1]) / 2.0
            return x, y

        # parallel?
        if abs(denominator) < GeometryTools.EPSILON:
            return None


        # intersection along the segments?
        mua = numerator_a / denominator
        mub = numerator_b / denominator

        if mua < 0.0 or mua > 1.0 or mub < 0.0 or mub > 1.0:
            return None

        x = p1[0] + mua * (p2[0] - p1[0])
        y = p1[1] + mua * (p2[1] - p1[1])
        return x, y

    @staticmethod
    def polylinesIntersect(polyline1 , polyline2):
        """Test if the polylines polyline1 and polyline2 intersect.
        
        The input arguments must be either Polyline instances[1], or a
        list of (float,float) tuples. The method performs a super
        naive n^2 check and should not be used for large polyline objects.

        @type polyline1: Polyline or list of tuple of (float, float)
        @type polyline2: Polyline or list of tuple of (float, float)

        [1]: The z - coordinate will be ignored.

        """

        for index1 in range(len(polyline1) - 1):
            p1 = polyline1[index1]
            p2 = polyline1[index1 + 1]
            for index2 in range(len(polyline2) - 1):
                p3 = polyline2[index2]
                p4 = polyline2[index2 + 1]

                if GeometryTools.lineIntersection(p1,p2,p3,p4):
                    return True
        
        return False




    @staticmethod
    def ccw(p1, p2, p3):
        """
        Three points are a counter-clockwise turn if ccw > 0, clockwise if
        ccw < 0, and collinear if ccw = 0 because ccw is a determinant that
        gives the signed area of the triangle formed by p1, p2 and p3.

        @type p1: tuple of (float, float)
        @type p2: tuple of (float, float)
        @type p3: tuple of (float, float)
        @rtype: float
        """
        return (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0])


    @staticmethod
    def convexHull(points):
        """
        Given a list of points finds the convex hull
        @type points: list of tuple of (float, float)
        @rtype: list of tuple of (float, float)
        """
        points = sorted(points)

        def keepLeft(hull, r):
            while len(hull) > 1 and GeometryTools.ccw(hull[-2], hull[-1], r) > 0:
                hull.pop()

            if len(hull) == 0 or hull[-1] != r:
                hull.append(r)

            return hull

        l = reduce(keepLeft, points, [])
        u = reduce(keepLeft, reversed(points), [])
        l.extend([u[i] for i in xrange(1, len(u) - 1)])

        return l


    @staticmethod
    def pointInPolygon(p, polygon):
        """
        Finds out if a point is inside a polygon or not
        @type p: tuple of (float, float)
        @type polygon: Polyline or list of tuple of (float, float)
        @rtype: bool
        """
        x = p[0]
        y = p[1]
        n = len(polygon)

        inside = False

        p1x, p1y = polygon[0][0:2]
        for index in range(n + 1):
            p2x, p2y = polygon[index % n][0:2]

            if min(p1y, p2y) < y <= max(p1y, p2y):
                if x <= max(p1x, p2x):
                    if p1y != p2y:
                        xints = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x

                    if p1x == p2x or x <= xints:
                        inside = not inside

            p1x, p1y = p2x, p2y

        return inside




    @staticmethod
    def slicePolygon(bounding_polygon, poly_line):
        """
        This algorithm extends the end-points of the line and find intersections between the line
        and the enclosing polygon. The result is a polygon sliced by the extended line.

        The enclosing polygon must be convex, closed and completely enclose the line.

        @type bounding_polygon: Polyline or list of tuple of (float, float)
        @type poly_line:  Polyline or list of tuple of (float, float)
        @rtype: list of tuple of (float, float)
        """

        p1 = poly_line[0]
        ray1 = GeometryTools.lineToRay(poly_line[1], poly_line[0])
        intersection1 = GeometryTools.rayPolygonIntersections(p1, ray1, bounding_polygon)[0] # assume convex

        p2 = poly_line[-1]
        ray2 = GeometryTools.lineToRay(poly_line[-2], poly_line[-1])
        intersection2 = GeometryTools.rayPolygonIntersections(p2, ray2, bounding_polygon)[0] # assume convex


        if intersection2[0] < intersection1[0]:
            intersection1, intersection2 = intersection2, intersection1
            poly_line = list(reversed(poly_line))

        result = [intersection1[1]]

        for index in range(intersection1[0] + 1, intersection2[0] + 1):
            result.append(bounding_polygon[index])

        result.append(intersection2[1])

        for point in reversed(poly_line):
            result.append(point)

        result.append(intersection1[1])

        return result



    @staticmethod
    def lineToRay(p0, p1):
        """
        Converts a line segment to a unit vector starting at p0 pointing towards p1.
        @type p0: tuple of (float, float)
        @type p1: tuple of (float, float)
        @rtype: tuple of (float, float)
        """

        x = p1[0] - p0[0]
        y = p1[1] - p0[1]

        length = sqrt(x * x + y * y)

        return x / length, y / length


    @staticmethod
    def rayLineIntersection(point, ray, p1, p2):
        """
        Finds the intersection between the ray starting at point and the line [p1, p2].
        @type point: tuple of (float, float)
        @type ray: tuple of (float, float)
        @type p1: tuple of (float, float)
        @type p2: tuple of (float, float)
        @rtype: tuple of (float, float) or None
        """
        denominator = ray[1] * (p2[0] - p1[0]) - ray[0] * (p2[1] - p1[1])
        numerator_a = ray[0] * (p1[1] - point[1]) - ray[1] * (p1[0] - point[0])
        numerator_b = (p2[0] - p1[0]) * (p1[1] - point[1]) - (p2[1] - p1[1]) * (p1[0] - point[0])

        # coincident?
        if abs(numerator_a) < GeometryTools.EPSILON and abs(numerator_b) < GeometryTools.EPSILON and abs(denominator) < GeometryTools.EPSILON:
            x = (p1[0] + p2[0]) / 2.0
            y = (p1[1] + p2[1]) / 2.0
            return x, y

        # parallel?
        if abs(denominator) < GeometryTools.EPSILON:
            return None


        # intersection along the segments?
        mua = numerator_a / denominator
        mub = numerator_b / denominator

        # for rays mub can be larger than 1.0
        if mua < 0.0 or mua > 1.0 or mub < 0.0:
            return None

        x = p1[0] + mua * (p2[0] - p1[0])
        y = p1[1] + mua * (p2[1] - p1[1])
        return x, y

    @staticmethod
    def rayPolygonIntersections(point, ray, polygon):
        """
        Finds all intersections along the ray with the polygon.
        The returned value is a tuple containing the line segment in the polygon and the intersection coordinate.

        @type point: tuple of (float, float)
        @type ray: tuple of (float, float)
        @type polygon: Polyline or [tuple of (float, float)]
        @rtype: list of tuple of (int, tuple of (float, float))
        """
        results = []
        for index in range(len(polygon) - 1):
            lp1 = polygon[index]
            lp2 = polygon[index + 1]

            intersection = GeometryTools.rayLineIntersection(point, ray, lp1, lp2)
            if intersection is not None:
                results.append((index, intersection))

        return results
