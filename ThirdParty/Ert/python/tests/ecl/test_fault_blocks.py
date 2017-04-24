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
from __future__ import print_function
from unittest import skipIf
import warnings

from ert.ecl import EclGrid, EclKW , EclRegion, EclDataType
from ert.ecl.faults import FaultBlock, FaultBlockLayer, FaultBlockCell,FaultCollection
from ert.geo import Polyline , CPolylineCollection
from ert.test import ExtendedTestCase , TestAreaContext


class FaultBlockTest(ExtendedTestCase):
    def setUp(self):
        self.grid = EclGrid.createRectangular( (10,10,10) , (1,1,1) )
        self.kw = EclKW( "FAULTBLK" , self.grid.getGlobalSize() , EclDataType.ECL_INT )
        self.kw.assign( 1 )

        reg = EclRegion( self.grid , False )

        for k in range(self.grid.getNZ()):
            reg.clear( )
            reg.select_kslice( k , k )
            self.kw.assign( k , mask = reg )
            self.kw[ k * self.grid.getNX() * self.grid.getNY() + 7] = 177
            

            
    def test_fault_block(self):
        grid = EclGrid.createRectangular( (5,5,1) , (1,1,1) )
        kw = EclKW( "FAULTBLK" , grid.getGlobalSize() , EclDataType.ECL_INT )
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

    def test_get_ijk(self):
        with TestAreaContext("python/fault_block_layer/neighbour") as work_area:
            with open("kw.grdecl","w") as fileH:
                fileH.write("FAULTBLK \n")
                fileH.write("1 1 1 0 0\n")
                fileH.write("1 2 2 0 3\n")
                fileH.write("4 2 2 3 3\n")
                fileH.write("4 4 4 0 0\n")
                fileH.write("4 4 4 0 5\n")
                fileH.write("/\n")

            kw = EclKW.read_grdecl(open("kw.grdecl") , "FAULTBLK" , ecl_type = EclDataType.ECL_INT)
        
        grid = EclGrid.createRectangular( (5,5,1) , (1,1,1) )
        layer = FaultBlockLayer( grid , 0 )
        layer.loadKeyword( kw )

        block = layer[0,0]
        self.assertEqual( block.getBlockID() , 1 )

        block = layer[2,2]
        self.assertEqual( block.getBlockID() , 2 )

        with self.assertRaises(ValueError):
            layer[3,3]

        with self.assertRaises(IndexError):
            layer[5,5]

                
        
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

            kw = EclKW.read_grdecl(open("kw.grdecl") , "FAULTBLK" , ecl_type = EclDataType.ECL_INT)
        
        grid = EclGrid.createRectangular( (5,5,1) , (1,1,1) )
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

        


    def test_neighbours2(self):
        nx = 8
        ny = 8
        nz = 1
        grid = EclGrid.createRectangular( (nx , ny , nz) , (1,1,1) )
        layer = FaultBlockLayer( grid , 0 )
        with TestAreaContext("python/FaultBlocks/neighbours"):
            with open("faultblock.grdecl","w") as fileH:
                fileH.write("FAULTBLK \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("3 3 3 3 2 2 2 2 \n")
                fileH.write("3 3 3 3 2 2 2 2 \n")
                fileH.write("3 3 3 3 2 2 2 2 \n")
                fileH.write("3 3 3 3 2 2 2 2 \n")
                fileH.write("/\n")
                
            kw = EclKW.read_grdecl(open("faultblock.grdecl") , "FAULTBLK" , ecl_type = EclDataType.ECL_INT)
            with open("faults.grdecl" , "w") as f:
                f.write("FAULTS\n")
                f.write("\'FY\'   1   4   4   4   1   1  'Y'  /\n")
                f.write("\'FX\'   4   4   1   8   1   1  'X'  /\n")
                f.write("/")
            
            faults = FaultCollection( grid , "faults.grdecl")
        layer.loadKeyword( kw )
        b1 = layer.getBlock( 1 )
        b2 = layer.getBlock( 2 )
        b3 = layer.getBlock( 3 )

        nb = b1.getNeighbours()
        self.assertTrue( b2 in nb )
        self.assertTrue( b3 in nb )
        
        polylines1 = CPolylineCollection()
        p1 = polylines1.createPolyline(name="P1")
        p1.addPoint(4,0)
        p1.addPoint(4,4)
        p1.addPoint(4,8)
        nb = b1.getNeighbours( polylines = polylines1 )
        self.assertFalse( b2 in nb )
        self.assertTrue( b3 in nb )


        polylines2 = CPolylineCollection()
        p1 = polylines2.createPolyline(name="P2")
        p1.addPoint(0,4)
        p1.addPoint(4,4)
        nb = b1.getNeighbours( polylines = polylines2 )
        self.assertTrue( b2 in nb )
        self.assertFalse( b3 in nb )

        

        layer.addFaultBarrier( faults["FY"] )
        nb = b1.getNeighbours()
        self.assertTrue( b2 in nb )
        self.assertFalse( b3 in nb )
        
        layer.addFaultBarrier( faults["FX"] )
        nb = b1.getNeighbours()
        self.assertEqual( len(nb) , 0 )


    def test_neighbours3(self):
        nx = 8
        ny = 8
        nz = 1
        grid = EclGrid.createRectangular( (nx , ny , nz) , (1,1,1) )
        layer = FaultBlockLayer( grid , 0 )
        with TestAreaContext("python/FaultBlocks/neighbours"):
            with open("faultblock.grdecl","w") as fileH:
                fileH.write("FAULTBLK \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("/\n")
                
            kw = EclKW.read_grdecl(open("faultblock.grdecl") , "FAULTBLK" , ecl_type = EclDataType.ECL_INT)
            with open("faults.grdecl" , "w") as f:
                f.write("FAULTS\n")
                f.write("\'FX\'   4   4   1   4   1   1  'X'  /\n")
                f.write("\'FX\'   5   5   5   8   1   1  'X'  /\n")
                f.write("/")
            
            faults = FaultCollection( grid , "faults.grdecl")
        layer.loadKeyword( kw )
        b1 = layer.getBlock( 1 )
        b2 = layer.getBlock( 2 )

        nb = b1.getNeighbours()
        self.assertTrue( b2 in nb )
        
        layer.addFaultBarrier( faults["FX"] , link_segments = False)
        nb = b1.getNeighbours()
        self.assertTrue( b2 in nb )



        



    def test_fault_block_edge(self):
        grid = EclGrid.createRectangular( (5,5,1) , (1,1,1) )
        kw = EclKW( "FAULTBLK" , grid.getGlobalSize() , EclDataType.ECL_INT )
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
            layer = FaultBlockLayer( self.grid , self.grid.getGlobalSize()  )
            
        layer = FaultBlockLayer( self.grid , 1 )
        self.assertEqual( 1 , layer.getK() )

        kw = EclKW( "FAULTBLK" , self.grid.getGlobalSize() , EclDataType.ECL_FLOAT )
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


    def test_add_polyline_barrier1(self):
        grid = EclGrid.createRectangular( (4,1,1) , (1,1,1) )
        layer = FaultBlockLayer( self.grid , 0 )
        polyline = Polyline( init_points = [ (1.99 , 0.001) , (2.01 , 0.99)])
        
        points = [((1,0) , (2,0))]
        
        geo_layer = layer.getGeoLayer()
        for p1,p2 in points:
            self.assertTrue(geo_layer.cellContact( p1 , p2 ))
            
        layer.addPolylineBarrier( polyline )
        for p1,p2 in points:
            print(p1,p2)
            self.assertFalse(geo_layer.cellContact( p1 , p2 ))



    def test_add_polyline_barrier2(self):
        grid = EclGrid.createRectangular( (10,10,1) , (1,1,1) )
        layer = FaultBlockLayer( self.grid , 0 )
        polyline = Polyline( init_points = [ (0.1 , 0.9) , (8.9,0.9) , (8.9,8.9) ])
        
        points = [((0,0) , (0,1)),
                  ((2,0) , (2,1)),
                  ((4,0) , (4,1)),
                  ((6,0) , (6,1)),
                  ((8,0) , (8,1)),
                  #
                  ((8,1) , (9,1)),
                  ((8,3) , (9,3)),
                  ((8,5) , (9,5)),
                  ((8,7) , (9,7))]
        
        geo_layer = layer.getGeoLayer()
        for p1,p2 in points:
            self.assertTrue(geo_layer.cellContact( p1 , p2 ))
            
        layer.addPolylineBarrier( polyline )
        for p1,p2 in points:
            print(p1,p2)
            self.assertFalse(geo_layer.cellContact( p1 , p2 ))


        



    def test_fault_block_layer_export(self):
        layer = FaultBlockLayer( self.grid , 1 )
        kw1 = EclKW( "FAULTBLK" , self.grid.getGlobalSize() + 1 , EclDataType.ECL_INT )
        with self.assertRaises(ValueError):
            layer.exportKeyword( kw1 )

        kw2 = EclKW( "FAULTBLK" , self.grid.getGlobalSize() , EclDataType.ECL_FLOAT )
        with self.assertRaises(TypeError):
            layer.exportKeyword(kw2)

            
    def test_internal_blocks(self):
        nx = 8
        ny = 8
        nz = 1
        grid = EclGrid.createRectangular( (nx , ny , nz) , (1,1,1) )
        layer = FaultBlockLayer( grid , 0 )
        with TestAreaContext("python/FaultBlocks/internal_blocks"):
            with open("faultblock.grdecl","w") as fileH:
                fileH.write("FAULTBLK \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 4 4 1 2 5 5 2 \n")
                fileH.write("1 4 4 1 2 5 5 2 \n")
                fileH.write("1 1 1 1 2 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("1 1 3 1 1 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("1 1 1 1 1 2 2 2 \n")
                fileH.write("/\n")


            kw = EclKW.read_grdecl(open("faultblock.grdecl") , "FAULTBLK" , ecl_type = EclDataType.ECL_INT)
            with open("faults.grdecl" , "w") as f:
                f.write("FAULTS\n")
                f.write("\'FX\'   4   4   1   4   1   1  'X'  /\n")
                f.write("\'FX\'   5   5   4   4   1   1  'Y'  /\n")
                f.write("\'FX\'   5   5   5   8   1   1  'X'  /\n")
                f.write("/")
            
            faults = FaultCollection( grid , "faults.grdecl")

        layer.loadKeyword( kw )
        layer.addFaultBarrier( faults["FX"] )
        b1 = layer.getBlock( 1 )
        b2 = layer.getBlock( 2 )
        b3 = layer.getBlock( 3 )
        b4 = layer.getBlock( 4 )
        b5 = layer.getBlock( 5 )


        nb = b1.getNeighbours()
        for b in nb:
            print('Block:%d' % b.getBlockID())

        self.assertTrue( len(nb) == 2 )
        self.assertTrue( b3 in nb )
        self.assertTrue( b4 in nb )

        nb = b2.getNeighbours()
        self.assertTrue( len(nb) == 1 )
        self.assertTrue( b5 in nb )
