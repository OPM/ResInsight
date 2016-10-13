#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'fault_block.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import ctypes
from cwrap import BaseCClass, CWrapper
from ert.geo import Polyline, GeometryTools , CPolylineCollection
from ert.util import DoubleVector , IntVector
from ert.ecl import ECL_LIB


class FaultBlockCell(object):
    def __init__(self , i,j,k ,x,y,z):

        self.i = i
        self.j = j
        self.k = k

        self.x = x
        self.y = y
        self.z = z

        
    def __str__(self):
        return "(%d,%d)" % (self.i , self.j)



class FaultBlock(BaseCClass):

    def __init__(self , *args , **kwargs):
        raise NotImplementedError("Class can not be instantiated directly!")
        

    def __getitem__(self , index):
        if isinstance(index, int):
            if index < 0:
                index += len(self)
                
            if 0 <= index < len(self):
                x = ctypes.c_double()
                y = ctypes.c_double()
                z = ctypes.c_double()

                i = ctypes.c_int()
                j = ctypes.c_int()
                k = ctypes.c_int()
                
                self.cNamespace().export_cell(self , index , ctypes.byref(i) , ctypes.byref(j) , ctypes.byref(k) , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z))
                return FaultBlockCell( i.value , j.value , k.value , x.value , y.value , z.value )
            else:
                raise IndexError("Index:%d out of range: [0,%d)" % (index , len(self)))
        else:
            raise TypeError("Index:%s wrong type - integer expected")

    def __str__(self):
        return "Block ID: %d" % self.getBlockID()
        

    def __len__(self):
        return self.cNamespace().get_size( self )

    def free(self):
        self.cNamespace().free(self)

    def getCentroid(self):
        xc = self.cNamespace().get_xc( self )
        yc = self.cNamespace().get_yc( self )
        return (xc,yc)


    def countInside(self , polygon):
        """
        Will count the number of points in block which are inside polygon.
        """
        inside = 0
        for p in self:
            if GeometryTools.pointInPolygon( (p.x , p.y) , polygon ):
                inside += 1
        
        return inside 


    def getBlockID(self):
        return self.cNamespace().get_block_id(self)


    def assignToRegion(self , region_id):
        self.cNamespace().assign_to_region(self , region_id)
        

    def getRegionList(self):
        regionList = self.cNamespace().get_region_list(self)
        return regionList.copy()

    def addCell(self, i , j):
        self.cNamespace().add_cell( self , i , j )

    def getGlobalIndexList(self):
        return self.cNamespace().get_global_index_list( self )
        

    def getEdgePolygon(self):
        x_list = DoubleVector()
        y_list = DoubleVector()
        cell_list = IntVector()
        
        self.cNamespace().trace_edge( self , x_list , y_list , cell_list )
        p = Polyline()
        for (x,y) in zip(x_list , y_list):
            p.addPoint(x,y)
        return p


    def containsPolyline(self, polyline):
        """
        Will return true if at least one point from the polyline is inside the block.
        """
        edge_polyline = self.getEdgePolygon()
        for p in polyline:
            if GeometryTools.pointInPolygon( p , edge_polyline ):
                return True
        else:
            edge_polyline.assertClosed()
            return GeometryTools.polylinesIntersect( edge_polyline , polyline )

        
    def getNeighbours(self, polylines = None , connected_only = True):
        """
        Will return a list of FaultBlock instances which are in direct
        contact with this block.
        """
        neighbour_id_list = IntVector()
        if polylines is None:
            polylines = CPolylineCollection()
            
        self.cNamespace().get_neighbours( self , connected_only , polylines , neighbour_id_list )

        parent_layer = self.getParentLayer()
        neighbour_list = []
        for id in neighbour_id_list:
            neighbour_list.append( parent_layer.getBlock( id ))
        return neighbour_list
        

    def getParentLayer(self):
        return self.parent()
        


cwrapper = CWrapper(ECL_LIB)
CWrapper.registerObjectType("fault_block", FaultBlock)

FaultBlock.cNamespace().get_xc                = cwrapper.prototype("double                fault_block_get_xc(fault_block)")
FaultBlock.cNamespace().get_yc                = cwrapper.prototype("double                fault_block_get_yc(fault_block)")
FaultBlock.cNamespace().get_block_id          = cwrapper.prototype("int                   fault_block_get_id(fault_block)")
FaultBlock.cNamespace().get_size              = cwrapper.prototype("int                   fault_block_get_size(fault_block)")
FaultBlock.cNamespace().export_cell           = cwrapper.prototype("void                  fault_block_export_cell(fault_block , int , int* , int* , int* , double* , double* , double*)")
FaultBlock.cNamespace().assign_to_region      = cwrapper.prototype("void                  fault_block_assign_to_region(fault_block , int)")
FaultBlock.cNamespace().get_region_list       = cwrapper.prototype("int_vector_ref        fault_block_get_region_list(fault_block)")
FaultBlock.cNamespace().add_cell              = cwrapper.prototype("void                  fault_block_add_cell(fault_block,  int , int)")
FaultBlock.cNamespace().get_global_index_list = cwrapper.prototype("int_vector_ref        fault_block_get_global_index_list(fault_block)")
FaultBlock.cNamespace().trace_edge            = cwrapper.prototype("void                  fault_block_trace_edge( fault_block, double_vector , double_vector , int_vector)")  
FaultBlock.cNamespace().get_neighbours        = cwrapper.prototype("void                  fault_block_list_neighbours( fault_block , bool , geo_polygon_collection , int_vector)")  

