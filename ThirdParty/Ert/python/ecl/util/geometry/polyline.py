import collections
from .geometry_tools import GeometryTools

class Polyline(object):
    def __init__(self, name=None , init_points = None):
        super(Polyline, self).__init__()
        self.__name = name
        self.__points = []
        if init_points:
            self.loadPoints( init_points )

    def __str__(self):
        s = "Polyline:[ "
        for p in self:
            s += "(%s,%s) " % (p[0],p[1])
        s += "]"
        return s

    def getName(self):
        """ @rtype: str """
        return self.__name


    def __iadd__(self , other ):
        for p in other:
            self.__points.append( p )
        return self


    def __add__(self , other ):
        copy = Polyline( init_points = self)
        copy.__iadd__(other)
        return copy


    def __radd__(self , other ):
        copy = Polyline( init_points = other )
        copy.__iadd__(self)
        return copy
            

    def __eq__(self, other):
        if len(self) != len(other):
            return False

        for (p1,p2) in zip(self , other):
            if p1 != p2:
                return False
        
        return True



    def __len__(self):
        return len(self.__points)

    def addPoint(self, x, y, z=None):
        if z is None:
            p = (x, y)
        else:
            p = (x, y, z)
        self.__points.append(p)

    def __getitem__(self, index):
        """ @rtype: tuple of (float, float, float) """
        if index < 0:
            index += len(self)

        if not 0 <= index < len(self):
            raise IndexError("Index:%d invalid must be in range: [0, %d]" % (index, (len(self) - 1)))
        return self.__points[index]

    def isClosed(self):
        first = self[0]
        last = self[-1]

        return first == last


    def assertClosed(self):
        if not self.isClosed():
            first_point = self[0]

            x = first_point[0]
            y = first_point[1]
            try:
                z = first_point[2]
                self.addPoint(x,y,z)
            except IndexError:
                self.addPoint(x,y)
            


            

    def loadPoints(self , points):
        for point in points:
            x = point[0]
            y = point[1]
            try:
                z = point[2]
            except IndexError:
                z = None
            
            self.addPoint(x,y,z)
                
    def intersects(self, other_polyline):
        """
        Test if instances intersects with other polyline.

        @type other_polyline: Polyline or list of tuple of (float, float)
        @rtype: bool
        """
        return GeometryTools.polylinesIntersect( self , other_polyline )


    def __iter__(self):
        index = 0

        while index < len(self):
            yield self[index]
            index += 1


    def unzip2(self):
        x = []
        y = []
        for p in self:
            x.append(p[0])
            y.append(p[1])
        
        return (x,y)


    def unzip(self):
        first_point = self[0]
        x = []
        y = []

        try:
            z = first_point[2]
            z = []
            for p in self:
                x.append(p[0])
                y.append(p[1])
                z.append(p[2])

            return (x,y,z)
        except IndexError:
            for p in self:
                x.append(p[0])
                y.append(p[1])
            
            return (x,y)
            

    def connect(self , target):
        end1 = self[0]
        end2 = self[-1]

        p1 = GeometryTools.nearestPointOnPolyline( end1 , target )
        p2 = GeometryTools.nearestPointOnPolyline( end2 , target )
            
        d1 = GeometryTools.distance( p1 , end1 )
        d2 = GeometryTools.distance( p2 , end2 )

        if d1 < d2:
            return [end1 , p1]
        else:
            return [end2 , p2]

