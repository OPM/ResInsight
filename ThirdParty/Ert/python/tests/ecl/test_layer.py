#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_layer.py' is part of ERT - Ensemble based Reservoir Tool.
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

from unittest import skipIf
import time

from ecl.util import IntVector
from ecl.ecl import EclGrid
from ecl.geo import CPolyline
from ecl.ecl.faults import Layer , FaultCollection
from ecl.test import ExtendedTestCase , TestAreaContext


class LayerTest(ExtendedTestCase):
    def setUp(self):
        pass


    def test_create_layer(self):
        layer = Layer(10,10)
        self.assertTrue( isinstance( layer , Layer ))


    def test_add_cell(self):
        layer = Layer(10,10)
        with self.assertRaises(ValueError):
            layer[100,100] = 199

        with self.assertRaises(ValueError):
            layer[100,"X"] = 199

        with self.assertRaises(ValueError):
            layer[100] = 199

        layer[5,5] = 88
        self.assertEqual(layer[5,5] , 88)
        


    def test_contact(self):
        nx = 20
        ny = 10
        layer = Layer(nx,ny)
        grid = EclGrid.createRectangular( (nx,ny,1) , (1,1,1) )
        
        with self.assertRaises(IndexError):
            layer.cellContact( (-1,0),(1,1) )

        with self.assertRaises(IndexError):
            layer.cellContact( (20,0),(1,1) )


        self.assertFalse( layer.cellContact((0,0) , (2,0)) )
        self.assertFalse( layer.cellContact((1,0) , (1,0)) )

        self.assertTrue( layer.cellContact((0,0) , (1,0)) )
        self.assertTrue( layer.cellContact((1,0) , (0,0)) )
        
        self.assertTrue( layer.cellContact((0,0) , (0,1)) )
        self.assertTrue( layer.cellContact((0,1) , (0,0)) )

        self.assertFalse( layer.cellContact((0,0) , (1,1)) )
        self.assertFalse( layer.cellContact((1,1) , (0,0)) )
        
        self.assertTrue( layer.cellContact((4,0) , (5,0)) )
        self.assertTrue( layer.cellContact((0,4) , (0,5)) )
        
        with TestAreaContext("Layer/barrier"):
            with open("faults.grdecl" , "w") as f:
                f.write("FAULTS\n")
                f.write("\'FX\'   5   5   1   10   1   1  'X'  /\n")
                f.write("\'FY\'   1   10   5   5   1   1  'Y'  /\n")
                f.write("/")
                
            faults = FaultCollection( grid , "faults.grdecl")
            
        layer.addFaultBarrier( faults["FX"] , 0 )
        self.assertFalse( layer.cellContact((4,0) , (5,0)) )

        layer.addFaultBarrier( faults["FY"] , 0 )
        self.assertFalse( layer.cellContact((0,4) , (0,5)) )

        self.assertFalse( layer.cellContact((9,4) , (9,5)) )
        self.assertTrue( layer.cellContact((10,4) , (10,5)) )
        
    def test_get_barrier(self):
        layer = Layer(10,10)
        self.assertFalse( layer.leftBarrier(5,5) )
        self.assertFalse( layer.bottomBarrier(5,5) )

        layer.addIJBarrier([(1,1),(2,1),(2,2)])
        self.assertTrue( layer.bottomBarrier(1,1) )
        self.assertTrue( layer.leftBarrier(2,1) )
        
        
    
    def test_fault_barrier(self):
        nx = 120
        ny = 60
        nz = 43
        grid = EclGrid.createRectangular( (nx , ny , nz) , (1,1,1) )
        with TestAreaContext("python/faults/line_order"):
            with open("faults.grdecl" , "w") as f:
                f.write("""FAULTS
\'F\'              105  107     50   50      1   43    \'Y\'    / 
\'F\'              108  108     50   50      1   43    \'X\'    /
\'F\'              108  108     50   50     22   43    \'Y\'    /
\'F\'              109  109     49   49      1   43    \'Y\'    /
\'F\'              110  110     49   49      1   43    \'X\'    /
\'F\'              111  111     48   48      1   43    \'Y\'    /
/
""")                
            with open("faults.grdecl") as f:
                faults = FaultCollection( grid , "faults.grdecl" )


        # Fault layout:                
        #
        # +---+---+---+---+
        #                 |
        #                 +---+   +  
        #                         |
        #                         +---+ 

        
        fault = faults["F"]
        layer = Layer(nx,ny)
        fault_pairs = [((104,49),(104,50)),
                       ((105,49),(105,50)),
                       ((106,49),(106,50)),
                       ((107,49),(108,49)),
                       ((107,49),(107,50)),
                       ((108,48),(108,49)),
                       ((109,48),(110,48)),
                       ((110,47),(110,48))]
        gap_pair = ((109,48),(109,49))


        for p1,p2 in fault_pairs:
            self.assertTrue(layer.cellContact( p1 , p2 ))

        p1,p2 = gap_pair
        self.assertTrue(layer.cellContact( p1 , p2 ))


        layer.addFaultBarrier(fault , 30 , link_segments = False)
        for p1,p2 in fault_pairs:
            self.assertFalse(layer.cellContact( p1 , p2 ))
        p1,p2 = gap_pair
        self.assertTrue(layer.cellContact( p1 , p2 ))

        layer.addFaultBarrier(fault , 30)
        p1,p2 = gap_pair
        self.assertFalse(layer.cellContact( p1 , p2 ))


    def test_contact2(self):
        nx = 10
        ny = 10
        layer = Layer(nx,ny)
        grid = EclGrid.createRectangular( (nx,ny,1) , (1,1,1) )

        # Too short
        with self.assertRaises(ValueError):
            layer.addIJBarrier( [(1,5)] )

        # Out of range
        with self.assertRaises(ValueError):
            layer.addIJBarrier( [(10,15),(5,5)] )

        # Out of range
        with self.assertRaises(ValueError):
            layer.addIJBarrier( [(7,7),(-5,5)] )
            
        # Must have either i1 == i2 or j1 == j2
        with self.assertRaises(ValueError):
            layer.addIJBarrier( [(7,8),(6,5)] )

        p1 = (0 , 4)
        p2 = (0 , 5)
        self.assertTrue(layer.cellContact( p1 , p2 ))
        layer.addIJBarrier( [(0,5) , (nx , 5)] )
        self.assertFalse(layer.cellContact( p1 , p2 ))



    def test_update_connected(self):
        nx = 10
        ny = 10
        layer = Layer(nx,ny)

        layer[0,0] = 100
        self.assertEqual( layer[0,0], 100 )
        layer.clearCells()
        self.assertEqual( layer[0,0], 0 )
        self.assertEqual( layer.cellSum( ) , 0 )
        
        with self.assertRaises(ValueError):
            layer.updateConnected( (10,10) , 10 )

        layer[0,0] = 77
        with self.assertRaises(ValueError):
            layer.updateConnected( (0,0) , 10 , org_value = 0)

        layer.updateConnected( (0,0) , 10 )
        self.assertEqual( 10 , layer.cellSum() )

        layer[0,0] = 0
        layer.updateConnected( (0,0) , 3 )
        self.assertEqual( nx*ny*3 , layer.cellSum() )

        layer.addIJBarrier( [(5,0), (5,10)] )
        layer.clearCells( )
        self.assertEqual( 0 , layer.cellSum( ) )
        layer.updateConnected( (0,0) , 1 )
                
        self.assertEqual( 50 , layer.cellSum( ) )
        self.assertEqual( layer[4,0] , 1 )
        self.assertEqual( layer[5,0] , 0 )

        layer = Layer(nx,ny)
        layer.addIJBarrier( [(5,0), (5,5)] )
        layer.updateConnected( (0,0) , 1 )
        self.assertEqual( 100 , layer.cellSum( ) )
        
        
    def test_matching(self):
        d = 10
        layer = Layer(d,d)
        
        for i in range(d):
            layer[i,i] = 10

        cell_list = layer.cellsEqual( 1 )
        self.assertEqual( cell_list , [] )
        
        cell_list = layer.cellsEqual( 10 )
        self.assertEqual( cell_list , [ (i,i) for i in range(d)] )
        
    
    def test_add_polyline_barrier(self):
        d = 10
        layer = Layer(d,d)
        grid = EclGrid.createRectangular( (d,d,1) , (1,1,1) )
        pl = CPolyline( init_points = [(0 , 0) , (d/2 , d/2) , (d,d)])
        layer.addPolylineBarrier( pl , grid , 0)
        for i in range(d):
            self.assertTrue( layer.bottomBarrier(i,i) )
            if i < (d - 1):
                self.assertTrue( layer.leftBarrier(i+1,i) )
                

    def test_active(self):
        d = 10
        layer = Layer(d,d)
        with self.assertRaises( ValueError ):
            layer.activeCell(d+1,d+2)
            
        self.assertTrue( layer.activeCell(1,2) )

        grid = EclGrid.createRectangular( (d,d+1,1) , (1,1,1) )
        with self.assertRaises( ValueError ):
            layer.updateActive( grid , 0 )

        grid = EclGrid.createRectangular( (d,d,1) , (1,1,1) )
        with self.assertRaises( ValueError ):
             layer.updateActive( grid , 10 )
            
        actnum = IntVector( initial_size = d*d*1 , default_value = 1)
        actnum[0] = 0
        grid = EclGrid.createRectangular( (d,d,1) , (1,1,1) , actnum = actnum)
        layer.updateActive( grid , 0 )
        self.assertTrue( layer.activeCell(1,2) )
        self.assertFalse( layer.activeCell(0,0) )


    def test_assign(self):
        layer = Layer(10,5)
        self.assertEqual( layer.cellSum() , 0 )

        layer.assign(10)
        self.assertEqual( layer.cellSum() , 500 )
        


    def test_count_equal(self):
        layer = Layer(10,10)
        self.assertEqual( 100 , layer.countEqual( 0 ))
        self.assertEqual( 0 , layer.countEqual( 1 ))

        layer[3,3] = 3
        self.assertEqual( 1 , layer.countEqual( 3 ))
