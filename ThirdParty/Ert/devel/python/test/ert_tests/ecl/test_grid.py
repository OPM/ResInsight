#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_grid.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os.path
from unittest import skipIf
import time

from ert.ecl import EclGrid
from ert.ecl.faults import Layer , FaultCollection
from ert.test import ExtendedTestCase , TestAreaContext


# This test class should only have test cases which do not require
# external test data. Tests involving Statoil test data are in the
# test_grid_statoil module.

class GridTest(ExtendedTestCase):
    
    def test_dims(self):
        grid = EclGrid.createRectangular( (10,20,30) , (1,1,1) )
        self.assertEqual( grid.getNX() , 10 )
        self.assertEqual( grid.getNY() , 20 )
        self.assertEqual( grid.getNZ() , 30 )
        self.assertEqual( grid.getGlobalSize() , 30*10*20 )

    def test_node_pos(self):
        grid = EclGrid.createRectangular( (10,20,30) , (1,1,1) )
        with self.assertRaises(IndexError):
            grid.getNodePos(-1,0,0)

        with self.assertRaises(IndexError):
            grid.getNodePos(11,0,0)

        p0 = grid.getNodePos(0,0,0)
        self.assertEqual( p0 , (0,0,0))

        p7 = grid.getNodePos(10,20,30)
        self.assertEqual( p7 , (10,20,30))

    
    def test_truncated_file(self):
        grid = EclGrid.createRectangular( (10,20,30) , (1,1,1) )
        with TestAreaContext("python/ecl_grid/truncated"):
            grid.save_EGRID( "TEST.EGRID")

            size = os.path.getsize( "TEST.EGRID")
            with open("TEST.EGRID" , "r+") as f:
                f.truncate( size / 2 )

            with self.assertRaises(IOError):
                EclGrid("TEST.EGRID")
