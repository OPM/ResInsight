#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_fault_blocks.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.ecl import EclGrid, EclTypeEnum , EclKW , EclRegion
from ert.test import ExtendedTestCase , TestAreaContext
from ert.ecl.faults import FaultBlock, FaultBlockLayer, FaultBlockCell


class FaultBlockTest(ExtendedTestCase):
    def setUp(self):
        self.grid = EclGrid.create_rectangular( (10,10,10) , (1,1,1) )
        self.kw = EclKW.create( "FAULTBLK" , self.grid.size , EclTypeEnum.ECL_INT_TYPE )
        self.kw.assign( 1 )

        reg = EclRegion( self.grid , False )

        for k in range(self.grid.getNZ()):
            reg.clear( )
            reg.select_kslice( k , k )
            self.kw.assign( k , mask = reg )
            self.kw[ k * self.grid.getNX() * self.grid.getNY() + 7] = 177
            

            
    def test_fault_block(self):
        grid = EclGrid.create_rectangular( (5,5,1) , (1,1,1) )
        kw = EclKW.create( "FAULTBLK" , grid.size , EclTypeEnum.ECL_INT_TYPE )
        kw.assign( 0 )
        for j in range(1,4):
            for i in range(1,4):
                g = i + j*grid.getNX()
                kw[g] = 1

        layer = FaultBlockLayer( grid , 0 )
        layer.scanKeyword( kw )
        block = layer[1]

        self.assertEqual( (2.50 , 2.50) , block.getCentroid() )
        self.assertEqual( len(block) , 9)
        self.assertEqual( layer , block.getParentLayer() )

        
    def test_neighbours(self):

        with TestAreaContext("python/fault_block_layer/neighbour") as work_area:
            with open("kw.grdecl","w") as fileH:
                fileH.write("FAULTBLK \n")
                fileH.write("1 1 1 0 0\n")
                fileH.write("1 2 2 0 3\n")
                fileH.write("4 2 2 3 3\n")
                fileH.write("4 4 4 0 0\n")
                fileH.write("4 4 4 0 5\n")
                fileH.write("/\n")

            kw = EclKW.read_grdecl(open("kw.grdecl") , "FAULTBLK" , ecl_type = EclTypeEnum.ECL_INT_TYPE)
        
        grid = EclGrid.create_rectangular( (5,5,1) , (1,1,1) )
        layer = FaultBlockLayer( grid , 0 )

        layer.loadKeyword( kw )
        block1 = layer.getBlock( 1 )
        block2 = layer.getBlock( 2 )
        block3 = layer.getBlock( 3 )
        block4 = layer.getBlock( 4 )
        block5 = layer.getBlock( 5 )
        self.assertEqual( block1.getParentLayer() , layer )

        #Expected: 1 -> {2,4}, 2 -> {1,3,4}, 3 -> {2}, 4 -> {1,2}, 5-> {}
                
        neighbours = block1.getNeighbours()
        self.assertEqual( len(neighbours) , 2)
        self.assertTrue( block2 in neighbours )
        self.assertTrue( block4 in neighbours )

        neighbours = block2.getNeighbours()
        self.assertEqual( len(neighbours) , 3)
        self.assertTrue( block1 in neighbours )
        self.assertTrue( block3 in neighbours )
        self.assertTrue( block4 in neighbours )
                
        neighbours = block3.getNeighbours()
        self.assertEqual( len(neighbours) , 1)
        self.assertTrue( block2 in neighbours )

        neighbours = block4.getNeighbours()
        self.assertEqual( len(neighbours) , 2)
        self.assertTrue( block1 in neighbours )
        self.assertTrue( block2 in neighbours )

        neighbours = block5.getNeighbours()
        self.assertEqual( len(neighbours) , 0)




    def test_fault_block_edge(self):
        grid = EclGrid.create_rectangular( (5,5,1) , (1,1,1) )
        kw = EclKW.create( "FAULTBLK" , grid.size , EclTypeEnum.ECL_INT_TYPE )
        kw.assign( 0 )
        for j in range(1,4):
            for i in range(1,4):
                g = i + j*grid.getNX()
                kw[g] = 1

        layer = FaultBlockLayer( grid , 0 )
        #with self.assertRaises:
        #    layer.getEdgePolygon( )
        


    def test_fault_block_layer(self):
        with self.assertRaises(ValueError):
            layer = FaultBlockLayer( self.grid , -1 )

        with self.assertRaises(ValueError):
            layer = FaultBlockLayer( self.grid , self.grid.size  )
            
        layer = FaultBlockLayer( self.grid , 1 )
        self.assertEqual( 1 , layer.getK() )

        kw = EclKW.create( "FAULTBLK" , self.grid.size , EclTypeEnum.ECL_FLOAT_TYPE )
        with self.assertRaises(ValueError):
            layer.scanKeyword( kw )

        layer.scanKeyword( self.kw )
        self.assertEqual( 2 , len(layer) )

        with self.assertRaises(TypeError):
            ls = layer["JJ"]

        l = []
        for blk in layer:
            l.append( blk )
        self.assertEqual( len(l) , 2 )

        l0 = layer[0]
        l1 = layer[1]
        self.assertTrue( isinstance(l1 , FaultBlock ))
        l0.getCentroid()
        l1.getBlockID()

        with self.assertRaises(IndexError):
            l2 = layer[2]

            
        self.assertEqual( True , 1 in layer)
        self.assertEqual( True , 2 in layer)
        self.assertEqual( False , 77 in layer)
        self.assertEqual( False , 177 in layer)

        l1 = layer.getBlock( 1 )
        self.assertTrue( isinstance(l1 , FaultBlock ))
        
        with self.assertRaises(KeyError):
            l =layer.getBlock(66)

        with self.assertRaises(KeyError):
            layer.deleteBlock(66)

        layer.deleteBlock(2)
        self.assertEqual( 1 , len(layer))
        blk = layer[0]
        self.assertEqual( blk.getBlockID() , 1 ) 

        with self.assertRaises(KeyError):
            layer.addBlock(1)
            
        blk2 = layer.addBlock(2)
        self.assertEqual( len(layer) , 2 ) 
        
        blk3 = layer.addBlock()
        self.assertEqual( len(layer) , 3 ) 
        

        layer.addBlock(100)
        layer.addBlock(101)
        layer.addBlock(102)
        layer.addBlock(103)

        layer.deleteBlock(2)
        blk1 = layer.getBlock( 103 )
        blk2 = layer[-1]
        self.assertEqual( blk1.getBlockID() , blk2.getBlockID() )

        fault_block = layer[0]
        fault_block.assignToRegion( 2 )
        self.assertEqual( [2] , list(fault_block.getRegionList()))

        fault_block.assignToRegion( 2 )
        self.assertEqual( [2] , list(fault_block.getRegionList()))

        fault_block.assignToRegion( 3 )
        self.assertEqual( [2,3] , list(fault_block.getRegionList()))

        fault_block.assignToRegion( 1 )
        self.assertEqual( [1,2,3] , list(fault_block.getRegionList()))

        fault_block.assignToRegion( 2 )
        self.assertEqual( [1,2,3] , list(fault_block.getRegionList()))



    def test_fault_block_layer_export(self):
        layer = FaultBlockLayer( self.grid , 1 )
        kw1 = EclKW.create( "FAULTBLK" , self.grid.size + 1 , EclTypeEnum.ECL_INT_TYPE )
        with self.assertRaises(ValueError):
            layer.exportKeyword( kw1 )

        kw2 = EclKW.create( "FAULTBLK" , self.grid.size , EclTypeEnum.ECL_FLOAT_TYPE )
        with self.assertRaises(TypeError):
            layer.exportKeyword(kw2)

            


