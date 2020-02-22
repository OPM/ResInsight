#  Copyright (C) 2014  Equinor ASA, Norway. 
#   
#  The file 'cpolyline_collection.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 
"""
Create a polygon
"""
import ctypes

from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.util.geometry import CPolyline


class CPolylineCollection(BaseCClass):
    TYPE_NAME = "geo_polygon_collection"

    _alloc_new          = EclPrototype("void*            geo_polygon_collection_alloc(  )" , bind = False)
    _free               = EclPrototype("void             geo_polygon_collection_free( geo_polygon_collection )" )
    _size               = EclPrototype("int              geo_polygon_collection_size( geo_polygon_collection)" )
    _create_polyline    = EclPrototype("geo_polygon_ref  geo_polygon_collection_create_polygon(geo_polygon_collection , char*)" )
    _has_polyline       = EclPrototype("bool             geo_polygon_collection_has_polygon(geo_polygon_collection , char*)" )
    _iget               = EclPrototype("geo_polygon_ref  geo_polygon_collection_iget_polygon(geo_polygon_collection , int)" )
    _get                = EclPrototype("geo_polygon_ref  geo_polygon_collection_get_polygon(geo_polygon_collection , char*)" )
    _add_polyline       = EclPrototype("void             geo_polygon_collection_add_polygon(geo_polygon_collection , geo_polygon , bool)")



    def __init__(self):
        c_ptr = self._alloc_new(  )
        super(CPolylineCollection , self).__init__( c_ptr )
        self.parent_ref = None


    def __contains__(self , name):
        return self._has_polyline(name)


    def __len__(self):
        return self._size( )


    def __iter__(self):
        index = 0

        while index < len(self):
            yield self[index]
            index += 1


    def __getitem__(self , index):
        if isinstance(index , int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                return self._iget( index).setParent( self )
            else:
                raise IndexError("Invalid index:%d - valid range: [0,%d)" % (index , len(self)))
        elif isinstance(index , str):
            if index in self:
                return self._get(index)
            else:
                raise KeyError("No polyline named:%s" % index)
        else:
            raise TypeError("The index argument must be string or integer")


    def shallowCopy(self):
        copy = CPolylineCollection()
        for pl in self:
            copy._add_polyline(pl , False)

        # If we make a shallow copy we must ensure that source, owning
        # all the polyline objects does not go out of scope.
        copy.parent_ref = self
        return copy



    def addPolyline(self , polyline , name = None):
        if not isinstance(polyline , CPolyline):
            polyline = CPolyline( init_points = polyline , name = name)
        else:
            if not name is None:
                raise ValueError("The name keyword argument can only be supplied when add not CPOlyline object")

        name = polyline.getName()
        if name and name in self:
            raise KeyError("The polyline collection already has an object:%s" % name)

        if polyline.isReference():
            self._add_polyline( polyline , False)
        else:
            polyline.convertToCReference( self )
            self._add_polyline( polyline , True)



    def createPolyline(self , name = None):
        if name and name in self:
            raise KeyError("The polyline collection already has an object:%s" % name)

        polyline = self._create_polyline(name)
        polyline.setParent( parent = self )
        return polyline


    def free(self):
        self._free( )
