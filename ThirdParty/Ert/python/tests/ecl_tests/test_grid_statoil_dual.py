#  Copyright (C) 2018  Statoil ASA, Norway.
#
#  The file 'test_grid_statoil_dual.py' is part of ERT - Ensemble based Reservoir Tool.
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
import math

from ecl.util.test import TestAreaContext
from ecl.grid import EclGrid

from tests import EclTest, statoil_test

@statoil_test()
class GridDualTest(EclTest):

    def egrid_file(self):
        return self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")

    def grid_file(self):
        return self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.GRID")

    def test_dual(self):
        with TestAreaContext("python/grid-test/testDual"):
            grid = EclGrid(self.egrid_file())
            self.assertFalse(grid.dualGrid())
            self.assertTrue(grid.getNumActiveFracture() == 0)

            grid2 = EclGrid(self.grid_file())
            self.assertFalse(grid.dualGrid())
            self.assertTrue(grid.getNumActiveFracture() == 0)

            dgrid = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUALPOR_MSW.EGRID"))
            self.assertTrue(dgrid.getNumActive() == dgrid.getNumActiveFracture())
            self.assertTrue(dgrid.getNumActive() == 46118)

            dgrid2 = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUALPOR_MSW.GRID"))
            self.assertTrue(dgrid.getNumActive() == dgrid.getNumActiveFracture())
            self.assertTrue(dgrid.getNumActive() == 46118)
            self.assertTrue(dgrid.equal(dgrid2))


            # The DUAL_DIFF grid has been manipulated to create a
            # situation where some cells are only matrix active, and some
            # cells are only fracture active.
            dgrid = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUAL_DIFF.EGRID"))
            self.assertTrue(dgrid.getNumActive() == 106)
            self.assertTrue(dgrid.getNumActiveFracture() == 105)

            self.assertTrue(dgrid.get_active_fracture_index(global_index=0) == -1)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=2) == -1)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=3) == 0)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=107) == 104)

            self.assertTrue(dgrid.get_active_index(global_index=1) == 1)
            self.assertTrue(dgrid.get_active_index(global_index=105) == 105)
            self.assertTrue(dgrid.get_active_index(global_index=106) == -1)
            self.assertTrue(dgrid.get_global_index1F(2) == 5)

            dgrid.save_EGRID("DUAL_DIFF.EGRID")
            dgrid2 = EclGrid("DUAL_DIFF.EGRID")
            self.assertTrue(dgrid.equal(dgrid2 , verbose = True))
