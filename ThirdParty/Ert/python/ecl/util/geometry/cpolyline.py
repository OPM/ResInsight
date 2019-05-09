#  Copyright (C) 2011 Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
#  A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
"""
Create a polygon
"""
import ctypes
import os.path

from cwrap import BaseCClass
from ecl import EclPrototype
from .geometry_tools import GeometryTools


class CPolyline(BaseCClass):
    TYPE_NAME = "geo_polygon"

    _alloc_new          = EclPrototype("void*           geo_polygon_alloc( char* )" , bind = False)
    _fread_alloc_irap   = EclPrototype("geo_polygon_obj geo_polygon_fload_alloc_irap( char* )" , bind = False)
    _add_point          = EclPrototype("void     geo_polygon_add_point( geo_polygon , double , double )")
    _add_point_front    = EclPrototype("void     geo_polygon_add_point_front( geo_polygon , double , double )")
    _free               = EclPrototype("void     geo_polygon_free( geo_polygon )")
    _size               = EclPrototype("int      geo_polygon_get_size( geo_polygon )")
    _iget_xy            = EclPrototype("void     geo_polygon_iget_xy( geo_polygon , int , double* , double* )")
    _segment_intersects = EclPrototype("bool     geo_polygon_segment_intersects( geo_polygon , double , double, double , double)")
    _get_name           = EclPrototype("char*    geo_polygon_get_name( geo_polygon  )")
    _set_name           = EclPrototype("void     geo_polygon_set_name( geo_polygon , char*  )")
    _segment_length     = EclPrototype("double   geo_polygon_get_length( geo_polygon)")
    _equal              = EclPrototype("bool     geo_polygon_equal( geo_polygon , geo_polygon )")


    def __init__(self, name = None , init_points = ()):
        c_ptr = self._alloc_new( name )
        super(CPolyline , self).__init__( c_ptr )
        for (xc, yc) in init_points:
            self.addPoint(xc, yc)


    @classmethod
    def createFromXYZFile(cls , filename , name = None):
        if not os.path.isfile(filename):
            raise IOError("No such file:%s" % filename)

        polyline = cls._fread_alloc_irap( filename )
        if not name is None:
            polyline._set_name( name )
        return polyline

    def __str__(self):
        name = self.getName()
        if name:
            str = "%s [" % name
        else:
            str = "["

        for index,p in enumerate(self):
            str += "(%g,%g)" % p
            if index < len(self) - 1:
                str += ","
        str += "]"
        return str

    def __repr__(self):
        return str(self)


    def __len__(self):
        return self._size()


    def __getitem__(self , index):
        if not isinstance(index,int):
            raise TypeError("Index argument must be integer. Index:%s invalid" % index)

        if index < 0:
            index += len(self)

        if 0 <= index < len(self):
            x = ctypes.c_double()
            y = ctypes.c_double()
            self._iget_xy( index , ctypes.byref(x) , ctypes.byref(y) )

            return (x.value , y.value)
        else:
            raise IndexError("Invalid index:%d valid range: [0,%d)" % (index , len(self)))


    def segmentIntersects(self, p1 , p2):
        return self._segment_intersects(p1[0] , p1[1] , p2[0] , p2[1])


    def intersects(self , polyline):
        if len(self) > 1:
            for index,p2 in enumerate(polyline):
                if index == 0:
                    continue

                p1 = polyline[index - 1]
                if self.segmentIntersects(p1 , p2):
                    return True
        return False


    def __iadd__(self , other ):
        for p in other:
            self.addPoint( p[0] , p[1] )
        return self


    def __add__(self , other ):
        copy = CPolyline( init_points = self)
        copy.__iadd__(other)
        return copy


    def __radd__(self , other ):
        copy = CPolyline( init_points = other )
        copy.__iadd__(self)
        return copy

    def __eq__(self , other):
        if super(CPolyline , self).__eq__( other ):
            return True
        else:
            return self._equal( other )


    def segmentLength(self):
        if len(self) == 0:
            raise ValueError("Can not measure length of zero point polyline")

        return self._segment_length( )

    def extendToBBox(self , bbox , start = True):
        if start:
            p0 = self[1]
            p1 = self[0]
        else:
            p0 = self[-2]
            p1 = self[-1]

        ray_dir = GeometryTools.lineToRay(p0,p1)
        intersections = GeometryTools.rayPolygonIntersections( p1 , ray_dir , bbox)
        if intersections:
            p2 = intersections[0][1]
            if self.getName():
                name = "Extend:%s" % self.getName()
            else:
                name = None

            return CPolyline( name = name , init_points = [(p1[0] , p1[1]) , p2])
        else:
            raise ValueError("Logical error - must intersect with bounding box")


    def addPoint( self, xc, yc , front = False):
        if front:
            self._add_point_front(xc, yc)
        else:
            self._add_point(xc, yc)


    def getName(self):
        return self._get_name( )


    def free(self):
        self._free( )


    def unzip(self):
        x_list = [ ]
        y_list = [ ]
        for x,y in self:
            x_list.append(x)
            y_list.append(y)

        return (x_list , y_list)


    def unzip2(self):
        return self.unzip()


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
