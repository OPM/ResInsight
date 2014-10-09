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
from ert.ecl import EclGrid,EclKW,EclTypeEnum
from ert.test import ExtendedTestCase



class StatoilFaultTest(ExtendedTestCase):
    def loadGrid(self):
        grid_file   = self.createTestPath("Statoil/ECLIPSE/Faults/grid.grdecl")
        fileH = open(grid_file, "r")
        specgrid = EclKW.read_grdecl(fileH, "SPECGRID", ecl_type=EclTypeEnum.ECL_INT_TYPE, strict=False)
        zcorn = EclKW.read_grdecl(fileH, "ZCORN")
        coord = EclKW.read_grdecl(fileH, "COORD")
        actnum = EclKW.read_grdecl(fileH, "ACTNUM", ecl_type=EclTypeEnum.ECL_INT_TYPE)
        
        return EclGrid.create(specgrid, zcorn, coord, actnum)




    def test_load(self):
        grid = self.loadGrid()
        faults_file = self.createTestPath("Statoil/ECLIPSE/Faults/faults.grdecl")
        faults = FaultCollection( grid , faults_file )
        for fault in faults:
            for layer in fault:
                for fl in layer:
                    fl.verify()
                    



        



        
