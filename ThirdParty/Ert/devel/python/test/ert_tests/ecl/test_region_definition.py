#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_region_definition.py' is part of ERT - Ensemble based Reservoir Tool.
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
try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import time
from ert.ecl.faults import FaultCollection, Fault, FaultLine, FaultSegment, RegionDefinition, FaultBlockLayer
from ert.ecl import EclGrid, EclKW, EclTypeEnum
from ert.test import ExtendedTestCase
from ert.geo import Polyline



class RegionDefinitionTest(ExtendedTestCase):
    def setUp(self):
        self.grid = EclGrid.create_rectangular( (16,16,1) , (1,1,1) )
        self.poly1 = Polyline(init_points = [(0,0) , (0,4) , (5,5) , (0,5)])
        self.poly2 = Polyline(init_points = [(11,11) , (16,12) , (16,16) , (12,16)])
        self.fault_block_kw = EclKW.create( "FAULTBLK" , self.grid.getGlobalSize() , EclTypeEnum.ECL_INT_TYPE )
        self.fault_block_kw.assign( 0 )
        self.faults = FaultCollection( self.grid )
        for j in range(4):
            for i in range(4):
                g1 = i + j*self.grid.getNX()
                g2 = i + 12 + (j + 12)* self.grid.getNX()
                
                self.fault_block_kw[g1] = 1
                self.fault_block_kw[g2] = 2

        self.fault_blocks = []
        for k in range(self.grid.getNZ()):
            block = FaultBlockLayer( self.grid , k)
            block.scanKeyword( self.fault_block_kw )
            self.fault_blocks.append( block )


    
    def test_add_edge(self):
        createRegion = RegionDefinition( 1 )
        with self.assertRaises(TypeError):
            createRegion.addEdge("Poly1")

        with self.assertRaises(TypeError):
            createRegion.addEdge(112)
            

    def test_update(self):
        reg1 = RegionDefinition( 1 )
        reg2 = RegionDefinition( 2 )

        self.assertTrue( not reg1.hasPolygon() )

        reg1.addEdge( self.poly1 )
        reg2.addEdge( self.poly2 )
        
        self.assertTrue( reg1.hasPolygon() )
        

        region_kw = EclKW.create( "REGIONS" , self.grid.getGlobalSize() , EclTypeEnum.ECL_INT_TYPE )
        region_kw.assign( 0 )
        block_list1 = []
        block_list2 = []

        with self.assertRaises(NotImplementedError):
            block_list1 = reg1.findInternalBlocks(self.grid , reg1.splitFaultBlocks(self.grid , self.fault_blocks[0]))

        with self.assertRaises(NotImplementedError):
            block_list2 = reg2.findInternalBlocks(self.grid , reg2.splitFaultBlocks(self.grid , self.fault_blocks[0]))

        if block_list1:
            for block in block_list1:
                region_id = reg1.getRegionID()
                block.assignToRegion( region_id )
                for g in block.getGlobalIndexList():
                    region_kw[g] = region_id

        if block_list2:
            for block in block_list2:
                region_id = reg2.getRegionID()
                block.assignToRegion( region_id )
                for g in block.getGlobalIndexList():
                    region_kw[g] = region_id
                
        for j in range(4):
            for i in range(4):
                g1 = i + j*self.grid.getNX()
                g2 = i + 12 + (j + 12)* self.grid.getNX()
    
                #self.assertEqual(region_kw[g1] , 1)
                #self.assertEqual(region_kw[g2] , 2)
                
        #self.assertEqual( region_kw.sum() , 16 * 3 )
        
