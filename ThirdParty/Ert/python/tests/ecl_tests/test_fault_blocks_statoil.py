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

from ecl import EclDataType
from ecl.eclfile import EclKW
from ecl.grid import EclGrid
from tests import EclTest, statoil_test
from ecl.grid.faults import FaultBlock, FaultBlockLayer

from cwrap import open as copen


@statoil_test()
class FaultBlockTest(EclTest):
    def setUp(self):
        self.grid = EclGrid( self.createTestPath("Statoil/ECLIPSE/Mariner/MARINER.EGRID"))
        fileH = copen( self.createTestPath("Statoil/ECLIPSE/Mariner/faultblock.grdecl") )
        self.kw = EclKW.read_grdecl( fileH , "FAULTBLK" , ecl_type = EclDataType.ECL_INT )



    def test_load(self):
        for k in range(self.grid.getNZ()):
            faultBlocks = FaultBlockLayer(self.grid , k)
            faultBlocks.scanKeyword( self.kw )
            for block in faultBlocks:
                centroid = block.getCentroid()

