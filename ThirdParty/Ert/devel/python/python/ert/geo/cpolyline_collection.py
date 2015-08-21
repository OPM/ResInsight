#  Copyright (C) 2014  Statoil ASA, Norway. 
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

from ert.cwrap import BaseCClass, CWrapper
from ert.geo import ERT_GEOMETRY_LIB , CPolyline


class CPolylineCollection(BaseCClass):
    def __init__(self):
        c_ptr = CPolylineCollection.cNamespace().alloc_new(  )
        super(CPolylineCollection , self).__init__( c_ptr )
        self.parent_ref = None
        

    def __contains__(self , name):
        return CPolylineCollection.cNamespace().has_polyline(self , name)
    

    def __len__(self):
        return CPolylineCollection.cNamespace().size(self)

    
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
                return CPolylineCollection.cNamespace().iget(self , index).setParent( self )
            else:
                raise IndexError("Invalid index:%d - valid range: [0,%d)" % (index , len(self)))
        elif isinstance(index , str):
            if index in self:
                return CPolylineCollection.cNamespace().get(self , index)
            else:
                raise KeyError("No polyline named:%s" % index)
        else:
            raise TypeError("The index argument must be string or integer")


    def shallowCopy(self):
        copy = CPolylineCollection()
        for pl in self:
            CPolylineCollection.cNamespace().add_polyline(copy , pl , False)
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
            CPolylineCollection.cNamespace().add_polyline(self , polyline , False)
        else:
            polyline.convertToCReference( self )
            CPolylineCollection.cNamespace().add_polyline(self , polyline , True)



    def createPolyline(self , name = None):
        if name and name in self:
            raise KeyError("The polyline collection already has an object:%s" % name)
            
        polyline = CPolylineCollection.cNamespace().create_polyline(self , name)
        return polyline


    def free(self):
        CPolylineCollection.cNamespace().free(self)

    


#################################################################

cwrapper = CWrapper(ERT_GEOMETRY_LIB)
cwrapper.registerObjectType("geo_polygon_collection", CPolylineCollection)

CPolylineCollection.cNamespace().alloc_new          = cwrapper.prototype("c_void_p         geo_polygon_collection_alloc(  )")
CPolylineCollection.cNamespace().free               = cwrapper.prototype("void             geo_polygon_collection_free( geo_polygon_collection )" )
CPolylineCollection.cNamespace().size               = cwrapper.prototype("int              geo_polygon_collection_size( geo_polygon_collection)" )
CPolylineCollection.cNamespace().create_polyline    = cwrapper.prototype("geo_polygon_ref  geo_polygon_collection_create_polygon(geo_polygon_collection , char*)" )
CPolylineCollection.cNamespace().has_polyline       = cwrapper.prototype("bool             geo_polygon_collection_has_polygon(geo_polygon_collection , char*)" )
CPolylineCollection.cNamespace().iget               = cwrapper.prototype("geo_polygon_ref  geo_polygon_collection_iget_polygon(geo_polygon_collection , int)" )
CPolylineCollection.cNamespace().get                = cwrapper.prototype("geo_polygon_ref  geo_polygon_collection_get_polygon(geo_polygon_collection , char*)" )
CPolylineCollection.cNamespace().add_polyline       = cwrapper.prototype("void             geo_polygon_collection_add_polygon(geo_polygon_collection , geo_polygon , bool)")
