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
from ecl.ecl.faults import FaultCollection, Fault, FaultLine, FaultSegment
from ecl.ecl import EclGrid, EclKW, EclDataType
from ecl.test import ExtendedTestCase



class StatoilFaultTest(ExtendedTestCase):
    def loadGrid(self):
        grid_file   = self.createTestPath("Statoil/ECLIPSE/Faults/grid.grdecl")
        fileH = open(grid_file, "r")
        specgrid = EclKW.read_grdecl(fileH, "SPECGRID", ecl_type=EclDataType.ECL_INT, strict=False)
        zcorn = EclKW.read_grdecl(fileH, "ZCORN")
        coord = EclKW.read_grdecl(fileH, "COORD")
        actnum = EclKW.read_grdecl(fileH, "ACTNUM", ecl_type=EclDataType.ECL_INT)
        
        return EclGrid.create(specgrid, zcorn, coord, actnum)




    def test_load(self):
        grid = self.loadGrid()
        faults_file = self.createTestPath("Statoil/ECLIPSE/Faults/faults.grdecl")
        faults = FaultCollection( grid , faults_file )
        for fault in faults:
            for layer in fault:
                for fl in layer:
                    fl.verify()
                    


    def test_splitLine2(self):
        grid = self.loadGrid( )
        f = Fault(grid , "DF41_C")

#                         179   180   181
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                             |                                                            
#  78                         |                                                   
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                             | 
#  77                         |
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                             | 
#  76                         |
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                             |
#  75                         |
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#            
#  74        
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#            
#  73        
#           o     o     o     o-----o     o     o     o     o     o     o     o     o     o
#                                   |
#  72                               |
#           o     o     o     o-----o     o     o     o     o     o     o     o     o     o
#            
#  71        
#           o     o     o     o-----o     o     o     o     o     o     o     o     o     o
#                                   |
#  70                               |
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                                   |
#  69                               |
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#            
#  68        
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                                   
#  67        
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#            
#  66        
#           o     o     o     o     o     o     o     o     o     o     o     o     o     o
#                                   |   
#  65                               |
#           o     o     o     o-----o     o     o     o     o     o     o     o     o     o


        f.addRecord( 179,  179 ,    77  ,  78  ,     0  , 42 ,   'X'  )
        f.addRecord( 179,  179 ,    75  ,  76  ,     0  , 41 ,   'X'  )
        f.addRecord( 180,  180 ,    72  ,  72  ,     0  , 41 ,   'X'  )
        f.addRecord( 180,  180 ,    72  ,  72  ,     0  , 41 ,   'Y'  )
        f.addRecord( 180,  180 ,    72  ,  72  ,     0  , 41 ,   'Y-' )
                                                     
        f.addRecord( 180,  180 ,    70  ,  70  ,     0  , 42 ,   'Y'  )
        f.addRecord( 180,  180 ,    69  ,  70  ,     0  , 42 ,   'X'  )
        f.addRecord( 180,  180 ,    65  ,  65  ,     0  , 42 ,   'X'  )
        f.addRecord( 180,  180 ,    65  ,  65  ,     0  , 42 ,   'Y-' )
        
        
        ij_polyline = f.getIJPolyline( 19 )
        ij_list = [(180, 79), (180, 77), (180, 75),
                   (180, 73), (181, 73), (181, 72), (180, 72),
                   (180, 71), (181, 71), (181, 69),
                   (181, 66), (181, 65), (180, 65)]

        self.assertEqual(ij_polyline , ij_list)
        
        



        
