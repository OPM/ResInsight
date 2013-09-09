#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
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
import time
from unittest2 import skipIf
from ert.ecl import EclTypeEnum, EclKW, EclGrid
from ert.util import DoubleVector
from ert.util.test_area import TestAreaContext
from ert_tests import ExtendedTestCase


class GridTest(ExtendedTestCase):
    def egrid_file(self):
        return self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")


    def grid_file(self):
        return self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.GRID")


    def grdecl_file(self):
        return self.createTestPath("Statoil/ECLIPSE/Gurbat/include/example_grid_sim.GRDECL")

    def test_GRID( self ):
        grid = EclGrid(self.grid_file())
        self.assertTrue(grid)


    def test_EGRID( self ):
        grid = EclGrid(self.egrid_file())
        self.assertTrue(grid)


    def create(self, filename, load_actnum=True):
        fileH = open(filename, "r")
        specgrid = EclKW.read_grdecl(fileH, "SPECGRID", ecl_type=EclTypeEnum.ECL_INT_TYPE, strict=False)
        zcorn = EclKW.read_grdecl(fileH, "ZCORN")
        coord = EclKW.read_grdecl(fileH, "COORD")
        if load_actnum:
            actnum = EclKW.read_grdecl(fileH, "ACTNUM", ecl_type=EclTypeEnum.ECL_INT_TYPE)
        else:
            actnum = None

        mapaxes = EclKW.read_grdecl(fileH, "MAPAXES")
        grid = EclGrid.create(specgrid, zcorn, coord, actnum, mapaxes=mapaxes)
        return grid


    def test_rect(self):
        #work_area = TestArea("python/grid-test/testRect", True)
        with TestAreaContext("python/grid-test/testRect", True):
            a1 = 1.0
            a2 = 2.0
            a3 = 3.0
            grid = EclGrid.create_rectangular((9, 9, 9), (a1, a2, a3))
            grid.save_EGRID("rect.EGRID")
            grid2 = EclGrid("rect.EGRID")
            self.assertTrue(grid)
            self.assertTrue(grid2)

            (x, y, z) = grid.get_xyz(ijk=(4, 4, 4))
            self.assertAlmostEqualList([x, y, z], [4.5 * a1, 4.5 * a2, 4.5 * a3])

            v = grid.cell_volume(ijk=(4, 4, 4))
            self.assertAlmostEqualScaled(v, a1 * a2 * a3)

            z = grid.depth(ijk=(4, 4, 4 ))
            self.assertAlmostEqualScaled(z, 4.5 * a3)

            g1 = grid.global_index(ijk=(2, 2, 2))
            g2 = grid.global_index(ijk=(4, 4, 4))
            (dx, dy, dz) = grid.distance(g2, g1)
            self.assertAlmostEqualList([dx, dy, dz], [2 * a1, 2 * a2, 2 * a3])

            self.assertTrue(grid.cell_contains(2.5 * a1, 2.5 * a2, 2.5 * a3, ijk=(2, 2, 2)))

            ijk = grid.find_cell(1.5 * a1, 2.5 * a2, 3.5 * a3)
            self.assertAlmostEqualList(ijk, [1, 2, 3])


    def test_create(self):
        grid = self.create(self.grdecl_file())
        self.assertTrue(grid)


    def test_ACTNUM(self):
        g1 = self.create(self.grdecl_file())
        g2 = self.create(self.grdecl_file(), load_actnum=False)
        self.assertTrue(g1.equal(g2))


    def test_time(self):
        t0 = time.clock()
        g1 = EclGrid(self.egrid_file())
        t1 = time.clock()
        t = t1 - t0
        self.assertTrue(t < 1.0)


    def test_save(self):
        #work_area = TestArea("python/grid-test/testSave", True)
        with TestAreaContext("python/grid-test/testSave", True):
            g1 = EclGrid(self.egrid_file())

            g1.save_EGRID("test.EGRID")
            g2 = EclGrid("test.EGRID")
            self.assertTrue(g1.equal(g2))

            g1.save_GRID("test.GRID")
            g2 = EclGrid("test.GRID")
            self.assertTrue(g1.equal(g2))

            fileH = open("test.grdecl", "w")
            g1.save_grdecl(fileH)
            fileH.close()
            g2 = self.create("test.grdecl")
            self.assertTrue(g1.equal(g2))

    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow test of coarse grid skipped!")
    def test_coarse(self):
        #work_area = TestArea("python/grid-test/testCoarse")
        with TestAreaContext("python/grid-test/testCoarse"):
            testGRID = True
            g1 = EclGrid(self.createTestPath("Statoil/ECLIPSE/LGCcase/LGC_TESTCASE2.EGRID"))

            g1.save_EGRID("LGC.EGRID")
            g2 = EclGrid("LGC.EGRID")
            self.assertTrue(g1.equal(g2, verbose=True))

            if testGRID:
                g1.save_GRID("LGC.GRID")
                g3 = EclGrid("LGC.GRID")
                self.assertTrue(g1.equal(g3, verbose=True))

            self.assertTrue(g1.coarse_groups() == 3384)


    def test_raise_IO_error(self):
        with self.assertRaises(IOError):
            g = EclGrid("/does/not/exist.EGRID")


    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow test of dual grid skipped!")
    def test_dual(self):
        #work_area = TestArea("python/grid-test/testDual", True)
        with TestAreaContext("python/grid-test/testDual", True):
            grid = EclGrid(self.egrid_file())
            self.assertFalse(grid.dual_grid)
            self.assertTrue(grid.nactive_fracture == 0)

            grid2 = EclGrid(self.grid_file())
            self.assertFalse(grid.dual_grid)
            self.assertTrue(grid.nactive_fracture == 0)

            dgrid = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUALPOR_MSW.EGRID"))
            self.assertTrue(dgrid.nactive == dgrid.nactive_fracture)
            self.assertTrue(dgrid.nactive == 46118)

            dgrid2 = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUALPOR_MSW.GRID"))
            self.assertTrue(dgrid.nactive == dgrid.nactive_fracture)
            self.assertTrue(dgrid.nactive == 46118)
            self.assertTrue(dgrid.equal(dgrid2))


            # The DUAL_DIFF grid has been manipulated to create a
            # situation where some cells are only matrix active, and some
            # cells are only fracture active.
            dgrid = EclGrid(self.createTestPath("Statoil/ECLIPSE/DualPoro/DUAL_DIFF.EGRID"))
            self.assertTrue(dgrid.nactive == 106)
            self.assertTrue(dgrid.nactive_fracture == 105)

            self.assertTrue(dgrid.get_active_fracture_index(global_index=0) == -1)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=2) == -1)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=3) == 0)
            self.assertTrue(dgrid.get_active_fracture_index(global_index=107) == 104)

            self.assertTrue(dgrid.get_active_index(global_index=1) == 1)
            self.assertTrue(dgrid.get_active_index(global_index=105) == 105)
            self.assertTrue(dgrid.get_active_index(global_index=106) == -1)
            self.assertTrue(dgrid.get_global_index1F(2) == 5)

            dgrid.save_GRID("DUAL_DIFF.GRID")
            dgrid2 = EclGrid("DUAL_DIFF.GRID")
            self.assertTrue(dgrid.equal(dgrid2))

    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow test of nactive large memory skipped!")
    def test_nactive_large_memory(self):
        case = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE")
        vecList = []
        for i in range(12500):
            vec = DoubleVector()
            vec[81920] = 0
            vecList.append(vec)

        grid1 = EclGrid(case)
        grid2 = EclGrid(case)
        self.assertEqual(grid1.nactive, grid2.nactive)
        self.assertEqual(grid1.nactive, 34770)