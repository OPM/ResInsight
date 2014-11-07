#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_faults.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.ecl.faults import FaultCollection, Fault, FaultLine, FaultSegment
from ert.ecl import EclGrid
from ert.test import ExtendedTestCase
from ert.geo import Polyline

class FaultTest(ExtendedTestCase):
    def setUp(self):
        self.faults1 = self.createTestPath("local/ECLIPSE/FAULTS/fault1.grdecl")
        self.faults2 = self.createTestPath("local/ECLIPSE/FAULTS/fault2.grdecl")
        self.grid = EclGrid.create_rectangular( (151,100,50) , (1,1,1))
        
        
    def test_empty_collection(self):
        faults = FaultCollection(self.grid)
        self.assertEqual(0 , len(faults))

        self.assertFalse( faults.hasFault("FX") )
        
        with self.assertRaises(TypeError):
            f = faults[ [] ]

        with self.assertRaises(KeyError):
            f = faults["FX"]

        with self.assertRaises(IndexError):
            f = faults[0]


    def test_splitLine(self):
        faults = FaultCollection(self.grid)
        with self.assertRaises(ValueError):
            # Not slash terminated
            t = faults.splitLine("'F1'             149  149     29   29      1   43    'Y'")

        with self.assertRaises(ValueError):
            # Not integer
            t = faults.splitLine("'F1'             149  149     29   29      1   43X    'Y' /")

        with self.assertRaises(ValueError):
            # Missing item
            t = faults.splitLine("'F1'             149     29   29      1   43    'Y' /")

        with self.assertRaises(ValueError):
            # Quote fuckup
            t = faults.splitLine("'F1             149     149 29   29      1   43    'X' /")


    def test_empty_fault( self ):
        f = Fault(self.grid , "NAME")
        self.assertEqual("NAME" , f.getName())
        
        with self.assertRaises(KeyError):
            g = f["Key"]

        with self.assertRaises(KeyError):
            g = f[0]

        self.assertEqual( len(f) , 0 )


    def test_empty_faultLine(self):
        fl = FaultLine(self.grid , 10)
        self.assertEqual( 10 , fl.getK())
        self.assertEqual( 0 , len(fl) )
        
        with self.assertRaises(TypeError):
            f = fl[ "Test" ]

        with self.assertRaises(IndexError):
            f = fl[0]


    def test_faultLine(self):
        fl = FaultLine(self.grid , 10)
        S1 = FaultSegment(0 , 10)
        S2 = FaultSegment(10 , 20)
        fl.tryAppend( S1 )
        fl.tryAppend( S2 )
        fl.verify()
        S3 = FaultSegment(20 , 30)
        fl.tryAppend( S3 )
        fl.verify()
        #---
        fl = FaultLine(self.grid , 10)
        S1 = FaultSegment(0 , 10)
        S2 = FaultSegment(20 , 10)
        fl.tryAppend( S1 )
        self.assertTrue( fl.tryAppend( S2 ) ) 
        fl.verify()
        #---
        fl = FaultLine(self.grid , 10)
        S1 = FaultSegment(10 , 0)
        S2 = FaultSegment(20 , 10)
        fl.tryAppend( S1 )
        fl.tryAppend( S2 )
        fl.verify()
        #---
        fl = FaultLine(self.grid , 10)
        S1 = FaultSegment(10 , 0)
        S2 = FaultSegment(10 , 20)
        fl.tryAppend( S1 )
        fl.tryAppend( S2 )
        fl.verify()

        fl = FaultLine(self.grid , 10)
        S1 = FaultSegment(10 , 0)
        S2 = FaultSegment(10 , 20)
        fl.tryAppend( S1 )
        fl.tryAppend( S2 )
        S3 = FaultSegment(40 , 30)
        self.assertTrue( fl.tryAppend(S3) == False )
        self.assertEqual( len(fl) , 2 )
            
        pl = fl.getPolyline( )
        self.assertIsInstance( pl , Polyline )
        self.assertEqual( len(pl) , len(fl) + 1 )

        S3 = FaultSegment(20 , 30)
        fl.tryAppend( S3 )
        pl = fl.getPolyline( )
        self.assertIsInstance( pl , Polyline )
        self.assertEqual( len(pl) , len(fl) + 1 )




    def test_load(self):
        faults = FaultCollection(self.grid , self.faults1)
        self.assertEqual( 3 , len(faults))
        faults.load( self.faults2 )
        self.assertEqual( 7 , len(faults))
        fault1 = faults["F1"]
        layer8 = fault1[8]
        self.assertEqual( len(layer8) , 1 ) 
    
    
    def test_iter(self):
        faults = FaultCollection(self.grid , self.faults1 , self.faults2)
        self.assertEqual( 7 , len(faults))
        c = 0
        for f in faults:
            c += 1
        self.assertEqual( c , len(faults))
    
    
    
    def test_fault(self):
        f = Fault(self.grid , "NAME")
    
        with self.assertRaises(ValueError):
            # Invalid face
            f.addRecord( 10 , 10 , 11 , 11 , 1 , 43 , "F")
            
    
        with self.assertRaises(ValueError):
            # Invalid coordinates
            f.addRecord( -1 , 10 , 11 , 11 , 1 , 43 , "X")
    
        with self.assertRaises(ValueError):
            # Invalid coordinates
            f.addRecord( 10000 , 10 , 11 , 11 , 1 , 43 , "X")
    
        with self.assertRaises(ValueError):
            # Invalid coordinates
            f.addRecord( 10 , 9 , 11 , 11 , 1 , 43 , "X")
    
    
        with self.assertRaises(ValueError):
            # Invalid coordinates
            f.addRecord( 10 , 9 , 11 , 11 , 1 , 43 , "X")
    
        with self.assertRaises(ValueError):
            # Invalid coordinates/face combination
            f.addRecord( 10 , 11 , 11 , 11 , 1 , 43 , "X")
    
        with self.assertRaises(ValueError):
            # Invalid coordinates/face combination
            f.addRecord( 10 , 11 , 11 , 12 , 1 , 43 , "Y")
    
        f.addRecord(10 , 10 , 0 , 10 , 1 , 10 , "X")
    
    
    def test_segment(self ):
        s0 = FaultSegment(0 , 10)
        self.assertEqual(s0.getC1() , 0 )
        self.assertEqual(s0.getC2() , 10 )
    
        s0.swap()
        self.assertEqual(s0.getC1() , 10 )
        self.assertEqual(s0.getC2() , 0 )
        
    
    
    def test_fault_line(self ):
        faults = FaultCollection(self.grid , self.faults1 , self.faults2)
        for fault in faults:
            for layer in fault:
                for fl in layer:
                    fl.verify()



                    

    def test_neighbour_cells(self):
        nx = 10
        ny = 8
        nz = 7
        grid = EclGrid.create_rectangular( (nx , ny , nz) , (1,1,1) )
        faults_file = self.createTestPath("local/ECLIPSE/FAULTS/faults_nb.grdecl")
        faults = FaultCollection( grid , faults_file )

        fault = faults["FY"]
        self.assertEqual(len(fault),1)
        fault_layer = fault[0]

        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(0, nx) , (1,nx + 1), (2,nx+2) , (3,nx + 3) , (4,nx+4)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )
        
        fl2 = fault_layer[1]
        nb_cells2 = fl2.getNeighborCells()
        true_nb_cells2 = [(6, nx+6) , (7,nx + 7), (8 , nx+8) , (9,nx + 9)]
        self.assertListEqual( nb_cells2 , true_nb_cells2 )
                               
        nb_cells = fault_layer.getNeighborCells()
        self.assertListEqual( nb_cells , true_nb_cells1  + true_nb_cells2)

        
        fault = faults["FY0"]
        fault_layer = fault[0]
        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(-1,0) , (-1,1), (-1,2)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )


        fault = faults["FYNY"]
        fault_layer = fault[0]
        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(nx * (ny - 1) , -1), (nx * (ny - 1) + 1 , -1), (nx * (ny - 1) + 2, -1)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )

        fault = faults["FX"]
        fault_layer = fault[0]
        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(0,1) , (nx , nx+1) , (2*nx , 2*nx + 1)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )


        fault = faults["FX0"]
        fault_layer = fault[0]
        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(-1 , 0) , (-1 , nx) , (-1 , 2*nx)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )

        fault = faults["FXNX"]
        fault_layer = fault[0]
        fl1 = fault_layer[0]
        nb_cells1 = fl1.getNeighborCells()
        true_nb_cells1 = [(nx -1 , -1) , (2*nx -1 , -1) , (3*nx - 1 , -1)]
        self.assertListEqual( nb_cells1 , true_nb_cells1 )

