#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'region_definition.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import collections
import os.path

from ert.geo import Polyline , XYZIo, GeometryTools
from ert.ecl.faults import Fault, FaultBlockLayer , Layer 
from ert.ecl import EclKW, EclTypeEnum

class RegionDefinition(object):

    def __init__(self , region_id):
        if not isinstance(region_id , int):
            raise TypeError("The region_id input argument must be integer. region_id:%s - invalid" % region_id)
            
        self.region_id = region_id
        self.edges = []
        self.__has_polygon = False

                    

    @staticmethod
    def create(region_id , faults , edge_names):
        regionDef = RegionDefinition( region_id )

        for edge_name in edge_names:
            if isinstance(edge_name , str):
                if faults.hasFault( edge_name ):
                    regionDef.addEdge( faults[ edge_name ] )
                elif os.path.exists( edge_name ):
                    regionDef.addEdge( XYZIo.readXYZFile( edge_name ) )
                else:
                    raise ValueError("The elements in edge_list must be strings with either name of faults or filename with polygon. %s: invalid" % edge_name)
            else:
                raise ValueError("The elements in edge_list must be strings with either name of faults or filename with polygon. %s: invalid" % edge_name)

        return regionDef

    
    def getRegionID(self):
        return self.region_id


    def addEdge(self , edge):
        if isinstance(edge , Polyline):
            self.edges.append( edge )
            self.__has_polygon = True
        elif isinstance(edge , Fault):
            self.edges.append( edge )
        else:
            raise TypeError("Tried to add edge of wrong type; must be Fault or Polyline")



    def __convexHull(self , k):
        point_list = []
        
        if len(self.edges) == 0:
            raise Exception("You must add Fault / Polyline edges first")
            
        for edge in self.edges:
            if isinstance(edge , Fault):
                fault = edge
                layer = fault[k]
                for line in layer:
                    for p in line.getPolyline():
                        point_list.append( (p[0] , p[1]) )
            else:
                poly_line = edge
                for p in poly_line:
                    point_list.append( (p[0] , p[1]) )
        
        return GeometryTools.convexHull( point_list )    
        

    def findInternalBlocks(self , grid , fault_block_layer):
        block_list = []
        raise NotImplementedError
        return block_list
        

    def hasPolygon(self):
        return self.__has_polygon


    @staticmethod
    def splitFaultBlockClosedPolygon( grid , fault_blocks , polygon ):
        """
        Special case code when the region is limited only by one polygon.
        """
        new_fault_blocks = FaultBlockLayer( grid , fault_blocks.getK() )
        new_block = new_fault_blocks.addBlock()

        for block in fault_blocks:
            for p in block:
                if GeometryTools.pointInPolygon( (p.x , p.y) , polygon ):
                    new_block.addCell(p.i , p.j)
        
        return new_fault_blocks
                    
                    


    def splitFaultBlocks(self , grid , fault_blocks ):
        boundingPolygon = Polyline(init_points = grid.getBoundingBox2D())
        boundingPolygon.assertClosed()
        if self.hasPolygon():
            if len(self.edges) == 1:
                return self.splitFaultBlockClosedPolygon( grid , fault_blocks , self.edges[0] )
            else:
                current_fault_block_layer = fault_blocks
                k = fault_blocks.getK()
                for edge in self.edges:
                    if isinstance(edge , Polyline):
                        # Start on a brand new fault block layer.
                        next_fault_block_layer = FaultBlockLayer( grid , k )
                        for block in current_fault_block_layer:
                            if block.containsPolyline(edge):
                                print "Block %d is split due to edge:%s" % (block.getBlockID() , edge.name())
                                sliced = GeometryTools.slicePolygon( boundingPolygon , edge )
                                inside_list = []
                                outside_list = []
                                for p in block:
                                    if GeometryTools.pointInPolygon( (p.x , p.y) , sliced ):
                                        inside_list.append( p )
                                    else:
                                        outside_list.append( p )

                                if len(inside_list) * len(outside_list) == 0:
                                    new_block = next_fault_block_layer.addBlock( )
                                    for p in inside_list:
                                        new_block.addCell(p.i , p.j)

                                    for p in outside_list:
                                        new_block.addCell(p.i , p.j)
                                else:
                                    layer = Layer( grid.getNX() , grid.getNY() )
                                    for p in inside_list:
                                        layer[p.i , p.j] = 1

                                    for p in outside_list:
                                        layer[p.i , p.j] = 2
                                            
                                    next_fault_block_layer.scanLayer( layer )
                            else:
                                next_fault_block_layer.insertBlockContent( block )

                        current_fault_block_layer = next_fault_block_layer
                return current_fault_block_layer
        else:
            return fault_blocks
            
            
            



