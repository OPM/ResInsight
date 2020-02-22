from math import sqrt
import sys
import six

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
            return None

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

        l = six.functools.reduce(keepLeft, points, [])
        u = six.functools.reduce(keepLeft, reversed(points), [])
        l.extend([u[i] for i in six.moves.xrange(1, len(u) - 1)])

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
    def extendToEdge(bounding_polygon, poly_line):
        """
        """
        assert(bounding_polygon.isClosed())
        for p in poly_line:
            if not GeometryTools.pointInPolygon( p , bounding_polygon):
                raise ValueError("The point:%s was not inside bounding polygon")
        
        p1 = poly_line[0]
        ray1 = GeometryTools.lineToRay(poly_line[1], poly_line[0])
        intersection1 = GeometryTools.rayPolygonIntersections(p1, ray1, bounding_polygon)[0] # assume convex
        
        
        p2 = poly_line[-1]
        assert(GeometryTools.pointInPolygon(p2 , bounding_polygon))
                
        ray2 = GeometryTools.lineToRay(poly_line[-2], poly_line[-1])
        intersection2 = GeometryTools.rayPolygonIntersections(p2, ray2, bounding_polygon)
        intersection2 = GeometryTools.rayPolygonIntersections(p2, ray2, bounding_polygon)[0] # assume convex

        return [intersection1[1]] + poly_line + [intersection2[1]]


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
        tmp = GeometryTools.rayPolygonIntersections(p1, ray1, bounding_polygon)
        intersection1 = GeometryTools.rayPolygonIntersections(p1, ray1, bounding_polygon)[0] # assume convex

        p2 = poly_line[-1]
        ray2 = GeometryTools.lineToRay(poly_line[-2], poly_line[-1])
        intersection2 = GeometryTools.rayPolygonIntersections(p2, ray2, bounding_polygon)[0] # assume convex


        # Check for intersection between the polyline extensions on the inside of the bounadary
        internal_intersection = GeometryTools.lineIntersection( p1 , intersection1[1] , p2 , intersection2[1])
        if internal_intersection:
            start_point = poly_line[0]
            return poly_line + [ internal_intersection , start_point]



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
    def rayLineIntersection(point, ray, p1, p2 , flip_ray = False):
        """
        Finds the intersection between the ray starting at point and the line [p1, p2].
        @type point: tuple of (float, float)
        @type ray: tuple of (float, float)
        @type p1: tuple of (float, float)
        @type p2: tuple of (float, float)
        @rtype: tuple of (float, float) or None

        stackoverflow: 563198
        """
        s = (p2[0] - p1[0] , p2[1] - p1[1])
        q = p1
        r = ray
        p = point

        p_m_q = (p[0] - q[0] , p[1] - q[1])
        q_m_p = (q[0] - p[0] , q[1] - p[1])
        r_x_s = r[0] * s[1] - r[1]*s[0]

        q_m_p_x_r = q_m_p[0] * r[1] - q_m_p[1] * r[0]
        q_m_p_x_s = q_m_p[0] * s[1] - q_m_p[1] * s[0]
        
        if abs(r_x_s) < GeometryTools.EPSILON and abs(q_m_p_x_r) < GeometryTools.EPSILON:
            q_m_p_dot_r = q_m_p[0] * r[0] + q_m_p[1] * r[1]
            r_dot_r = r[0] * r[0] + r[1] * r[1]

            p_m_q_dot_s = p_m_q[0] * s[0] + p_m_q[1] * s[1]
            s_dot_s = s[0] * s[0] + s[1] * s[1]

            # Coincident
            if 0 <= q_m_p_dot_r <= r_dot_r:
                return ((p1[0] + p2[0]) / 2 , (p1[1] + p2[1]) / 2)

            # Coincident
            if 0 <= p_m_q_dot_s <= s_dot_s:
                return ((p1[0] + p2[0]) / 2 , (p1[1] + p2[1]) / 2)

            return None
            
            
        if abs(r_x_s) < GeometryTools.EPSILON:
            # Parallell
            return None


        t = 1.0 * q_m_p_x_s / r_x_s
        u = 1.0 * q_m_p_x_r / r_x_s

        if t >= 0 and 0 <= u <= 1:
            x = p[0] + t*r[0]
            y = p[1] + t*r[1]
            
            return x,y

        if flip_ray:
            return GeometryTools.rayLineIntersection( point , (-ray[0] , -ray[1]) , p1 , p2 , False)
        else:
            return None




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


    @staticmethod
    def distance(p1,p2):
        if len(p1) != len(p2):
            raise ValueError("Different lenght of objects")
        
        sqr_distance = 0
        for x1,x2 in zip(p1,p2):
            sqr_distance += (x1 - x2) * (x1 - x2)
            
        return sqrt( sqr_distance )
    

    @staticmethod
    def joinPolylines(polyline1 , polyline2):
        """The shortest straight line connecting polyline1 and polyline2.

        The joinPolylines function does not extend the polylines with
        a ray from the end, only the length of the straight line
        connecting the various endpoints is considered. If the two
        polylines already intersect the function returns None.
        """


        if len(polyline1) < 1:
            raise ValueError("Length of polyline must be >= 1")

        if len(polyline2) < 1:
            raise ValueError("Length of polyline must be >= 1")

        if GeometryTools.polylinesIntersect( polyline1 , polyline2):
            return None
            
        p0 = polyline1[0]
        p1 = polyline1[-1]
        pa = polyline2[0]
        pb = polyline2[-1]

        d_list = [ (GeometryTools.distance( p0 , pa ), [p0 , pa]),
                   (GeometryTools.distance( p0 , pb ), [p0 , pb]),
                   (GeometryTools.distance( p1 , pa ), [p1 , pa]),
                   (GeometryTools.distance( p1 , pb ), [p1 , pb]) ]

        d_list.sort( key = lambda x: x[0])
        return d_list[0][1]


    @staticmethod
    def connectPolylines( polyline , target_polyline):
        if GeometryTools.polylinesIntersect( polyline , target_polyline ):
            return None

        if len(polyline) < 2:
            raise ValueError("Polyline must have at least two points")

        d_list = []
        
        p0 = polyline[-1]
        p1 = polyline[-2]
        ray = GeometryTools.lineToRay( p1 , p0 )
        for (index , p) in GeometryTools.rayPolygonIntersections( p0 , ray , target_polyline):
            d_list.append( (GeometryTools.distance( p0 , p) , [p0 , p]) )
        
        p0 = polyline[0]
        p1 = polyline[1]
        ray = GeometryTools.lineToRay( p1 , p0 )
        for (index , p) in GeometryTools.rayPolygonIntersections( p0 , ray , target_polyline):
            d_list.append( (GeometryTools.distance( p0 , p) , [p0 , p]) )

        if len(d_list) == 0:
            raise ValueError("Polyline %s can not be extended to %s" % (polyline.getName() , target_polyline.getName()))

        d_list.sort( key = lambda x: x[0])
        return d_list[0][1]



    @staticmethod
    def nearestPointOnPolyline( p , polyline ):
        if len(polyline) > 1:
            d_list = [ GeometryTools.distance( p  , pi ) for pi in polyline ]
            index0 = d_list.index( min(d_list) )
            p0 = polyline[index0]
            dist0 = d_list[index0]

            dist1 = sys.float_info.max
            dist2 = sys.float_info.max
            intercept1 = None
            intercept2 = None

            index1 = None
            index2 = None
            if index0 > 0:
                index1 = index0 - 1

            if index0 < len(polyline) - 1:
                index2 = index0 + 1

            if not index1 is None:
                p1 = polyline[index1] 
                dy1 = p1[1] - p0[1]
                dx1 = p1[0] - p0[0]
                intercept1 = GeometryTools.rayLineIntersection( p , (dy1 , -dx1) , p0 , p1 , True)
                if intercept1:
                    dist1 = GeometryTools.distance( intercept1 , p )


            if not index2 is None:
                p2 = polyline[index2]
                dy2 = p2[1] - p0[1]
                dx2 = p2[0] - p0[0]
                intercept2 = GeometryTools.rayLineIntersection( p , (dy2 , -dx2) , p0 , p2 , True)
                if intercept2:
                    dist2 = GeometryTools.distance( intercept2 , p )                


            point_list = [ p0 , intercept1 , intercept2 ]
            d_list = [ dist0 , dist1 , dist2 ]
            index = d_list.index( min(d_list) )

            
            return point_list[index]
        else:
            raise ValueError("Polyline must have len() >= 2")

