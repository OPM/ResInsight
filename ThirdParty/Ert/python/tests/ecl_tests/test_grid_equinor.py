#!/usr/bin/env python
#  Copyright (C) 2011  Equinor ASA, Norway.
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
import math

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

from cwrap import Prototype
from cwrap import open as copen

import time
from ecl import EclDataType, EclUnitTypeEnum
from ecl.eclfile import EclKW, EclFile, openEclFile
from ecl.grid import EclGrid
from ecl.util.util import DoubleVector, IntVector
from ecl.util.test import TestAreaContext
from tests import EclTest, equinor_test


@equinor_test()
class GridTest(EclTest):
    def egrid_file(self):
        return self.createTestPath("Equinor/ECLIPSE/Gurbat/ECLIPSE.EGRID")


    def grid_file(self):
        return self.createTestPath("Equinor/ECLIPSE/Gurbat/ECLIPSE.GRID")


    def grdecl_file(self):
        return self.createTestPath("Equinor/ECLIPSE/Gurbat/include/example_grid_sim.GRDECL")

    def test_loadFromFile(self):
        g1 = EclGrid.loadFromFile( self.egrid_file() )
        g2 = EclGrid.loadFromFile( self.grdecl_file() )

        self.assertTrue( isinstance( g1 , EclGrid ) )
        self.assertTrue( isinstance( g2 , EclGrid ) )


    def test_corner(self):
        grid = EclGrid(self.egrid_file())
        nx = grid.getNX()
        ny = grid.getNY()
        nz = grid.getNZ()

        (x1,y1,z1) = grid.getCellCorner( 0 , ijk = (0,0,0))
        (x2,y2,z2) = grid.getLayerXYZ( 0 , 0 )
        self.assertEqual(x1,x2)
        self.assertEqual(y1,y2)
        self.assertEqual(z1,z2)

        (x1,y1,z1) = grid.getCellCorner( 0 , ijk = (0,1,0))
        (x2,y2,z2) = grid.getLayerXYZ( (nx + 1) , 0 )
        self.assertEqual(x1,x2)
        self.assertEqual(y1,y2)
        self.assertEqual(z1,z2)

        (x1,y1,z1) = grid.getCellCorner( 1 , ijk = (nx - 1,0,0))
        (x2,y2,z2) = grid.getLayerXYZ( nx , 0 )
        self.assertEqual(x1,x2)
        self.assertEqual(y1,y2)
        self.assertEqual(z1,z2)

        (x1,y1,z1) = grid.getCellCorner( 4 , ijk = (0,0,nz-1))
        (x2,y2,z2) = grid.getLayerXYZ( 0 , nz )
        self.assertEqual(x1,x2)
        self.assertEqual(y1,y2)
        self.assertEqual(z1,z2)

        (x1,y1,z1) = grid.getCellCorner( 7 , ijk = (nx-1,ny-1,nz-1))
        (x2,y2,z2) = grid.getLayerXYZ( (nx + 1)*(ny + 1) - 1 , nz )
        self.assertEqual(x1,x2)
        self.assertEqual(y1,y2)
        self.assertEqual(z1,z2)



        with self.assertRaises(IndexError):
            grid.getLayerXYZ( -1 , 0 )

        with self.assertRaises(IndexError):
            grid.getLayerXYZ( (nx + 1)*(ny + 1) , 0 )

        with self.assertRaises(IndexError):
            grid.getLayerXYZ( 0 , -1 )

        with self.assertRaises(IndexError):
            grid.getLayerXYZ( 0 , nz + 1 )


    def test_GRID( self ):
        grid = EclGrid(self.grid_file())
        self.assertTrue(grid)



    def test_EGRID( self ):
        grid = EclGrid(self.egrid_file())
        self.assertTrue(grid)
        dims = grid.getDims()
        self.assertEqual(dims[0] , grid.getNX())
        self.assertEqual(dims[1] , grid.getNY())
        self.assertEqual(dims[2] , grid.getNZ())



    def create(self, filename, load_actnum=True):
        fileH = copen(filename, "r")
        specgrid = EclKW.read_grdecl(fileH, "SPECGRID", ecl_type=EclDataType.ECL_INT, strict=False)
        zcorn = EclKW.read_grdecl(fileH, "ZCORN")
        coord = EclKW.read_grdecl(fileH, "COORD")
        if load_actnum:
            actnum = EclKW.read_grdecl(fileH, "ACTNUM", ecl_type=EclDataType.ECL_INT)
        else:
            actnum = None

        mapaxes = EclKW.read_grdecl(fileH, "MAPAXES")
        grid = EclGrid.create(specgrid, zcorn, coord, actnum, mapaxes=mapaxes)
        return grid


    def test_rect(self):
        with TestAreaContext("python/grid-test/testRect"):
            a1 = 1.0
            a2 = 2.0
            a3 = 3.0
            grid = EclGrid.createRectangular((9, 9, 9), (a1, a2, a3))
            grid.save_EGRID("rect.EGRID")
            grid2 = EclGrid("rect.EGRID")
            self.assertTrue(grid)
            self.assertTrue(grid2)

            (x, y, z) = grid.get_xyz(ijk=(4, 4, 4))
            self.assertAlmostEqualList([x, y, z], [4.5 * a1, 4.5 * a2, 4.5 * a3])

            v = grid.cell_volume(ijk=(4, 4, 4))
            self.assertFloatEqual(v, a1 * a2 * a3)

            z = grid.depth(ijk=(4, 4, 4 ))
            self.assertFloatEqual(z, 4.5 * a3)

            g1 = grid.global_index(ijk=(2, 2, 2))
            g2 = grid.global_index(ijk=(4, 4, 4))
            (dx, dy, dz) = grid.distance(g2, g1)
            self.assertAlmostEqualList([dx, dy, dz], [2 * a1, 2 * a2, 2 * a3])

            self.assertTrue(grid.cell_contains(2.5 * a1, 2.5 * a2, 2.5 * a3, ijk=(2, 2, 2)))

            #ijk = grid.find_cell(1.5 * a1 , 2.5 * a2 , 3.5 * a3)
            #self.assertAlmostEqualList(ijk, [1, 2, 3])


    def test_create(self):
        grid = self.create(self.grdecl_file())
        self.assertTrue(grid)


    def test_grdecl_load(self):
        with self.assertRaises(IOError):
            grid = EclGrid.loadFromGrdecl("/file/does/not/exists")

        with TestAreaContext("python/grid-test/grdeclLoad"):
            with open("grid.grdecl","w") as f:
                f.write("Hei ...")

            with self.assertRaises(ValueError):
                grid = EclGrid.loadFromGrdecl("grid.grdecl")

            actnum = IntVector(default_value = 1 , initial_size = 1000)
            actnum[0] = 0
            g1 = EclGrid.createRectangular((10,10,10) , (1,1,1) , actnum = actnum )
            self.assertEqual( g1.getNumActive() , actnum.elementSum() )
            g1.save_EGRID("G.EGRID")

            with open("grid.grdecl" , "w") as f2:
                f2.write("SPECGRID\n")
                f2.write("  10  10  10  \'F\' /\n")

            with openEclFile("G.EGRID") as f:
                with copen("grid.grdecl" , "a") as f2:

                    coord_kw = f["COORD"][0]
                    coord_kw.write_grdecl( f2 )

                    zcorn_kw = f["ZCORN"][0]
                    zcorn_kw.write_grdecl( f2 )

                    actnum_kw = f["ACTNUM"][0]
                    actnum_kw.write_grdecl( f2 )

            g2 = EclGrid.loadFromGrdecl("grid.grdecl")
            self.assertTrue( g1.equal( g2 ))



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
        with TestAreaContext("python/grid-test/testSave"):
            g1 = EclGrid(self.egrid_file())

            g1.save_EGRID("test.EGRID")
            g2 = EclGrid("test.EGRID")
            self.assertTrue(g1.equal(g2))

            g1.save_GRID("test.GRID")
            g2 = EclGrid("test.GRID")
            self.assertTrue(g1.equal(g2))

            fileH = copen("test.grdecl", "w")
            g1.save_grdecl(fileH)
            fileH.close()
            g2 = self.create("test.grdecl")
            self.assertTrue(g1.equal(g2))


    def test_raise_IO_error(self):
        with self.assertRaises(IOError):
            g = EclGrid("/does/not/exist.EGRID")

    def test_boundingBox(self):
        grid = EclGrid.createRectangular((10,10,10) , (1,1,1))
        with self.assertRaises(ValueError):
            bbox = grid.getBoundingBox2D(layer = -1 )

        with self.assertRaises(ValueError):
            bbox = grid.getBoundingBox2D( layer = 11 )

        bbox = grid.getBoundingBox2D( layer = 10 )
        self.assertEqual( bbox , ((0,0) , (10, 0) , (10 , 10) , (0,10)))


        with self.assertRaises(ValueError):
            grid.getBoundingBox2D( lower_left = (-1,0) )

        with self.assertRaises(ValueError):
            grid.getBoundingBox2D( lower_left = (6,10) )

        bbox = grid.getBoundingBox2D( lower_left = (3,3) )
        self.assertEqual( bbox , ((3,3) , (10,3) , (10,10) , (3,10)))

        with self.assertRaises(ValueError):
            grid.getBoundingBox2D( lower_left = (3,3) , upper_right = (2,2))

        bbox = grid.getBoundingBox2D( lower_left = (3,3) , upper_right = (7,7))
        self.assertEqual( bbox , ((3,3) , (7,3) , (7,7) , (3,7)))


    @skipIf(EclTest.slowTestShouldNotRun(), "Slow test of numActive large memory skipped!")
    def test_num_active_large_memory(self):
        case = self.createTestPath("Equinor/ECLIPSE/Gurbat/ECLIPSE")
        vecList = []
        for i in range(12500):
            vec = DoubleVector()
            vec[81920] = 0
            vecList.append(vec)

        grid1 = EclGrid(case)
        grid2 = EclGrid(case)
        self.assertEqual(grid1.getNumActive(), grid2.getNumActive())
        self.assertEqual(grid1.getNumActive(), 34770)


    def test_no_mapaxes_check_for_nan(self):
        grid_paths = ["Equinor/ECLIPSE/NoMapaxes/ECLIPSE.EGRID", "Equinor/ECLIPSE/NoMapaxes/ECLIPSE.GRID"]

        for grid_path in grid_paths:
            test_grid_path = self.createTestPath(grid_path)
            grid = EclGrid(test_grid_path)

            xyz = grid.get_xyz(ijk=(0, 0, 0))
            self.assertFalse(math.isnan(xyz[0]))
            self.assertFalse(math.isnan(xyz[1]))
            self.assertFalse(math.isnan(xyz[2]))

            xyz = grid.get_xyz(ijk=(1, 1, 1))
            self.assertFalse(math.isnan(xyz[0]))
            self.assertFalse(math.isnan(xyz[1]))
            self.assertFalse(math.isnan(xyz[2]))


    def test_valid_geometry(self):
        grid = EclGrid( self.createTestPath("Equinor/ECLIPSE/GRID_INVALID_CELL/PRED_RESEST_0_R_13_0.GRID"))
        self.assertTrue( grid.validCellGeometry( ijk = (27,0,0)) )
        self.assertFalse( grid.validCellGeometry( ijk = (0,0,0)) )


    def test_volume_kw(self):
        grid = EclGrid(self.egrid_file())
        vol = grid.createVolumeKeyword( )
        self.assertEqual( len(vol) , grid.getNumActive())
        for active_index , volume in enumerate(vol):
            self.assertEqual( volume , grid.cell_volume( active_index = active_index ))

        vol = grid.createVolumeKeyword( active_size = False )
        self.assertEqual( len(vol) , grid.getGlobalSize())
        for global_index , volume in enumerate(vol):
            self.assertEqual( volume , grid.cell_volume( global_index = global_index ))


    def test_lgr_get(self):
        grid = EclGrid(self.createTestPath("Equinor/ECLIPSE/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID"))
        for (nr,name) in [ ( 104 , "LG003017"),
                           (2 , "LG006024"),
                           (  4 , "LG005025"),
                           ( 82 , "LG011029"),
                           (100 , "LG007021"),
                           (110 , "LG003014")]:
            lgr_name = grid.get_lgr( name )
            lgr_nr = grid.get_lgr( nr )

            self.assertEqual( lgr_name , lgr_nr )

        with self.assertRaises(KeyError):
            grid.get_lgr("NO/SUCHLGR")

        with self.assertRaises(KeyError):
            grid.get_lgr(1024)
