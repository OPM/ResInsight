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
import six
from unittest import skipIf, skip
import time
import itertools
from numpy import linspace

from ecl.util.util import IntVector
from ecl import EclDataType, EclUnitTypeEnum
from ecl.eclfile import EclKW, EclFile
from ecl.grid import EclGrid
from ecl.grid import EclGridGenerator as GridGen
from ecl.grid.faults import Layer , FaultCollection
from ecl.util.test import TestAreaContext
from tests import EclTest

# This dict is used to verify that corners are mapped to the correct
# cell with respect to containment.
CORNER_HOME = {
        (0, 0, 0) : 0,  (0, 0, 1) : 9,  (0, 0, 2) : 18, (0, 0, 3) : 18,
        (0, 1, 0) : 3,  (0, 1, 1) : 12, (0, 1, 2) : 21, (0, 1, 3) : 21,
        (0, 2, 0) : 6,  (0, 2, 1) : 15, (0, 2, 2) : 24, (0, 2, 3) : 24,
        (0, 3, 0) : 6,  (0, 3, 1) : 15, (0, 3, 2) : 24, (0, 3, 3) : 24,
        (1, 0, 0) : 1,  (1, 0, 1) : 10, (1, 0, 2) : 19, (1, 0, 3) : 19,
        (1, 1, 0) : 4,  (1, 1, 1) : 13, (1, 1, 2) : 22, (1, 1, 3) : 22,
        (1, 2, 0) : 7,  (1, 2, 1) : 16, (1, 2, 2) : 25, (1, 2, 3) : 25,
        (1, 3, 0) : 7,  (1, 3, 1) : 16, (1, 3, 2) : 25, (1, 3, 3) : 25,
        (2, 0, 0) : 2,  (2, 0, 1) : 11, (2, 0, 2) : 20, (2, 0, 3) : 20,
        (2, 1, 0) : 5,  (2, 1, 1) : 14, (2, 1, 2) : 23, (2, 1, 3) : 23,
        (2, 2, 0) : 8,  (2, 2, 1) : 17, (2, 2, 2) : 26, (2, 2, 3) : 26,
        (2, 3, 0) : 8,  (2, 3, 1) : 17, (2, 3, 2) : 26, (2, 3, 3) : 26,
        (3, 0, 0) : 2,  (3, 0, 1) : 11, (3, 0, 2) : 20, (3, 0, 3) : 20,
        (3, 1, 0) : 5,  (3, 1, 1) : 14, (3, 1, 2) : 23, (3, 1, 3) : 23,
        (3, 2, 0) : 8,  (3, 2, 1) : 17, (3, 2, 2) : 26, (3, 2, 3) : 26,
        (3, 3, 0) : 8,  (3, 3, 1) : 17, (3, 3, 2) : 26, (3, 3, 3) : 26
}

def createVolumeTestGridBase(dim, dV, offset=1):
    return [
            GridGen.create_grid(dim, dV, offset=0),
            GridGen.create_grid(dim, dV, offset=offset),
            GridGen.create_grid(dim, dV, offset=offset, irregular_offset=True),
            GridGen.create_grid(dim, dV, offset=offset, concave=True),
            GridGen.create_grid(dim, dV, offset=offset, irregular=True),
            GridGen.create_grid(dim, dV, offset=offset, concave=True, irregular=True),
            GridGen.create_grid(dim, dV, offset=offset, irregular_offset=True, concave=True),
            GridGen.create_grid(dim, dV, offset=0, faults=True),
            GridGen.create_grid(dim, dV, offset=offset, faults=True),
            GridGen.create_grid(dim, dV, escape_origo_shift=(100, 100, 0), scale=2),
            GridGen.create_grid(dim, dV, escape_origo_shift=(100, 100, 0), scale=0.5),
            GridGen.create_grid(dim, dV, escape_origo_shift=(100, 100, 0), translation=(50,50,0)),
            GridGen.create_grid(dim, dV, escape_origo_shift=(100, 100, 0), rotate=True),
            GridGen.create_grid(dim, dV, escape_origo_shift=(100, 100, 0), misalign=True),
            GridGen.create_grid(dim, dV, offset=offset, escape_origo_shift=(100, 100, 0),
                irregular_offset=True, concave=True, irregular=True,
                scale=1.5, translation=(5,5,0), rotate=True,
                misalign=True)
            ]

def createContainmentTestBase():
    return [
            (3,  GridGen.create_grid((6,6,6), (1,1,1), offset=1)),
            (10, GridGen.create_grid((3,3,3), (1,1,1), offset=1, concave=True)),
            (4,  GridGen.create_grid((10,10,1), (1,1,1), offset=0., misalign=True)),
            (3,
                GridGen.create_grid((6,6,6), (1,1,1), offset=0.,
                    escape_origo_shift=(100, 100, 0),
                    irregular_offset=True, concave=True, irregular=True,
                    scale=1.5, translation=(5,5,0),
                    misalign=True
                    )
                )
            ]

def getMinMaxValue(grid):
    corners = [
                grid.getCellCorner(i, cell)
                    for i in range(8)
                        for cell in range(grid.getGlobalSize())
              ]

    return [(min(values), max(values)) for values in zip(*corners)]

def createWrapperGrid(grid):
    """
    Creates a grid that occupies the same space as the given grid,
    but that consists of a single cell.
    """

    x, y, z = grid.getNX()-1, grid.getNY()-1, grid.getNZ()-1
    corner_pos = [
                    (0, 0, 0), (x, 0, 0), (0, y, 0), (x, y, 0),
                    (0, 0, z), (x, 0, z), (0, y, z), (x, y, z)
                ]

    corners = [
                grid.getCellCorner(i, ijk=pos)
                for i, pos in enumerate(corner_pos)
              ]

    return GridGen.create_single_cell_grid(corners)

def average(points):
    p = six.functools.reduce(
        lambda a, b: (a[0] + b[0], a[1] + b[1], a[2] + b[2]), points)
    return [elem/float(len(points)) for elem in p]

# This test class should only have test cases which do not require
# external test data. Tests involving Statoil test data are in the
# test_grid_statoil module.
class GridTest(EclTest):

    def test_oom_grid(self):
        nx = 2000
        ny = 2000
        nz = 2000

        with self.assertRaises(MemoryError):
            grid = GridGen.createRectangular( (nx,ny,nz), (1,1,1))



    def test_posXYEdge(self):
        nx = 10
        ny = 11
        grid = GridGen.createRectangular( (nx,ny,1) , (1,1,1) )
        self.assertEqual( grid.findCellCornerXY(0,0,0)  , 0 )
        self.assertEqual( grid.findCellCornerXY(nx,0,0) , nx)
        self.assertEqual( grid.findCellCornerXY(0 , ny , 0) , (nx + 1 ) * ny )
        self.assertEqual( grid.findCellCornerXY(nx,ny,0) , (nx + 1 ) * (ny + 1) - 1)

        self.assertEqual( grid.findCellCornerXY(0.25,0,0)  , 0 )
        self.assertEqual( grid.findCellCornerXY(0,0.25,0)  , 0 )

        self.assertEqual( grid.findCellCornerXY(nx - 0.25,0,0)  , nx )
        self.assertEqual( grid.findCellCornerXY(nx , 0.25,0)  , nx )

        self.assertEqual( grid.findCellCornerXY(0 , ny - 0.25, 0) , (nx + 1 ) * ny )
        self.assertEqual( grid.findCellCornerXY(0.25 , ny , 0) , (nx + 1 ) * ny )

        self.assertEqual( grid.findCellCornerXY(nx -0.25 ,ny,0) , (nx + 1 ) * (ny + 1) - 1)
        self.assertEqual( grid.findCellCornerXY(nx , ny - 0.25,0) , (nx + 1 ) * (ny + 1) - 1)


    def test_dims(self):
        grid = GridGen.createRectangular( (10,20,30) , (1,1,1) )
        self.assertEqual( grid.getNX() , 10 )
        self.assertEqual( grid.getNY() , 20 )
        self.assertEqual( grid.getNZ() , 30 )
        self.assertEqual( grid.getGlobalSize() , 30*10*20 )

        self.assertEqual( grid.getDims() , (10,20,30,6000) )

    def test_create(self):
        with self.assertRaises(ValueError):
            grid = GridGen.createRectangular( (10,20,30) , (1,1,1) , actnum = [0,1,1,2])

        with self.assertRaises(ValueError):
            grid = GridGen.createRectangular( (10,20,30) , (1,1,1) , actnum = IntVector(initial_size = 10))
        grid = GridGen.createRectangular( (10,20,30) , (1,1,1) ) # actnum=None -> all active
        self.assertEqual( grid.getNumActive( ) , 30*20*10)
        actnum = IntVector(default_value = 1 , initial_size = 6000)
        actnum[0] = 0
        actnum[1] = 0
        grid = GridGen.createRectangular( (10,20,30) , (1,1,1) , actnum = actnum)
        self.assertEqual( grid.getNumActive( ) , 30*20*10 - 2)

    def test_all_iters(self):
        fk = self.createTestPath('local/ECLIPSE/faarikaal/faarikaal1.EGRID')
        grid = EclGrid(fk)
        cell = grid[3455]
        self.assertEqual(3455, cell.global_index)
        cell = grid[(4,1,82)]
        self.assertEqual(3455, cell.global_index)
        self.assertEqual(grid.cell(global_index=3455),
                         grid.cell(active_index=2000))
        self.assertEqual(grid.cell(global_index=3455),
                         grid.cell(i=4, j=1, k=82))

        na = grid.get_num_active()
        self.assertEqual(na, 4160)
        cnt = 0
        for c in grid.cells(active=True):
            cnt += 1
            self.assertTrue(c.active)
        self.assertEqual(cnt, 4160)

        cnt = len([c for c in grid.cells()])
        self.assertEqual(cnt, len(grid))


    def test_repr_and_name(self):
        grid = GridGen.createRectangular((2,2,2), (10,10,10), actnum=[0,0,0,0,1,1,1,1])
        pfx = 'EclGrid('
        rep = repr(grid)
        self.assertEqual(pfx, rep[:len(pfx)])
        self.assertEqual(type(rep), type(''))
        self.assertEqual(type(grid.getName()), type(''))
        with TestAreaContext("python/ecl_grid/repr"):
            grid.save_EGRID("CASE.EGRID")
            g2 = EclGrid("CASE.EGRID")
            r2 = repr(g2)
            self.assertEqual(pfx, r2[:len(pfx)])
            self.assertEqual(type(r2), type(''))
            self.assertEqual(type(g2.getName()), type(''))

    def test_node_pos(self):
        grid = GridGen.createRectangular( (10,20,30) , (1,1,1) )
        with self.assertRaises(IndexError):
            grid.getNodePos(-1,0,0)

        with self.assertRaises(IndexError):
            grid.getNodePos(11,0,0)

        p0 = grid.getNodePos(0,0,0)
        self.assertEqual( p0 , (0,0,0))

        p7 = grid.getNodePos(10,20,30)
        self.assertEqual( p7 , (10,20,30))


    # The broken file was previously handled by the ecl_file_open() call internally
    # in the ecl_grid implementation. That will now not fail for a broken file, and then
    # the grid class needs to do more/better checking itself.
    @skip("Needs better error checking inside in the ecl_grid")
    def test_truncated_file(self):
        grid = GridGen.createRectangular( (10,20,30) , (1,1,1) )
        with TestAreaContext("python/ecl_grid/truncated"):
            grid.save_EGRID( "TEST.EGRID")

            size = os.path.getsize( "TEST.EGRID")
            with open("TEST.EGRID" , "r+") as f:
                f.truncate( size / 2 )

            with self.assertRaises(IOError):
                EclGrid("TEST.EGRID")

    def test_posXY1(self):
        nx = 4
        ny = 1
        nz = 1
        grid = GridGen.createRectangular( (nx,ny,nz) , (1,1,1) )
        (i,j) = grid.findCellXY( 0.5 , 0.5, 0 )
        self.assertEqual(i , 0)
        self.assertEqual(j , 0)

        (i,j) = grid.findCellXY( 3.5 , 0.5, 0 )
        self.assertEqual(i , 3)
        self.assertEqual(j , 0)


    def test_init_ACTNUM(self):
        nx = 10
        ny = 23
        nz = 7
        grid = GridGen.createRectangular( (nx,ny,nz) , (1,1,1) )
        actnum = grid.exportACTNUM()

        self.assertEqual( len(actnum) , nx*ny*nz )
        self.assertEqual( actnum[0] , 1 )
        self.assertEqual( actnum[nx*ny*nz - 1] , 1 )

        actnum_kw = grid.exportACTNUMKw( )
        self.assertEqual(len(actnum_kw) , len(actnum))
        for a1,a2 in zip(actnum, actnum_kw):
            self.assertEqual(a1, a2)


    def test_posXY(self):
        nx = 10
        ny = 23
        nz = 7
        grid = GridGen.createRectangular( (nx,ny,nz) , (1,1,1) )
        with self.assertRaises(IndexError):
            grid.findCellXY( 1 , 1, -1 )

        with self.assertRaises(IndexError):
            grid.findCellXY( 1 , 1, nz + 1 )

        with self.assertRaises(ValueError):
            grid.findCellXY(15 , 78 , 2)


        i,j = grid.findCellXY( 1.5 , 1.5 , 2 )
        self.assertEqual(i , 1)
        self.assertEqual(j , 1)


        for i in range(nx):
            for j in range(ny):
                p = grid.findCellXY(i + 0.5 , j+ 0.5 , 0)
                self.assertEqual( p[0] , i )
                self.assertEqual( p[1] , j )

        c = grid.findCellCornerXY( 0.10 , 0.10 , 0 )
        self.assertEqual(c , 0)

        c = grid.findCellCornerXY( 0.90 , 0.90 , 0 )
        self.assertEqual( c , (nx + 1) + 1 )

        c = grid.findCellCornerXY( 0.10 , 0.90 , 0 )
        self.assertEqual( c , (nx + 1) )

        c = grid.findCellCornerXY( 0.90 , 0.90 , 0 )
        self.assertEqual( c , (nx + 1) + 1 )

        c = grid.findCellCornerXY( 0.90 , 0.10 , 0 )
        self.assertEqual( c , 1 )

    def test_compressed_copy(self):
        nx = 10
        ny = 10
        nz = 10
        grid = GridGen.createRectangular( (nx,ny,nz) , (1,1,1) )
        kw1 = EclKW("KW" , 1001 , EclDataType.ECL_INT )
        with self.assertRaises(ValueError):
            cp = grid.compressedKWCopy( kw1 )


    def test_dxdydz(self):
        nx = 10
        ny = 10
        nz = 10
        grid = GridGen.createRectangular( (nx,ny,nz) , (2,3,4) )

        (dx,dy,dz) = grid.getCellDims( active_index = 0 )
        self.assertEqual( dx , 2 )
        self.assertEqual( dy , 3 )
        self.assertEqual( dz , 4 )

    def test_numpy3D(self):
        nx = 10
        ny = 7
        nz = 5
        grid = GridGen.createRectangular((nx,ny,nz) , (1,1,1))
        kw = EclKW( "SWAT" , nx*ny*nz , EclDataType.ECL_FLOAT )
        numpy_3d = grid.create3D( kw )

    def test_len(self):
        nx = 10
        ny = 11
        nz = 12
        actnum = EclKW( "ACTNUM" , nx*ny*nz , EclDataType.ECL_INT )
        actnum[0] = 1
        actnum[1] = 1
        actnum[2] = 1
        actnum[3] = 1

        grid = GridGen.createRectangular( (nx,ny,nz) , (1,1,1), actnum = actnum)
        self.assertEqual( len(grid) , nx*ny*nz )
        self.assertEqual( grid.getNumActive( ) , 4 )

    def test_export(self):
        dims = (3, 3, 3)
        coord = GridGen.create_coord(dims, (1,1,1))
        zcorn = GridGen.create_zcorn(dims, (1,1,1), offset=0)

        grid = EclGrid.create(dims, zcorn, coord, None)

        self.assertEqual(zcorn, grid.export_zcorn())
        self.assertEqual(coord, grid.export_coord())

    def test_output_units(self):
        n = 10
        a = 1
        grid = GridGen.createRectangular( (n,n,n), (a,a,a))

        with TestAreaContext("python/ecl_grid/units"):
            grid.save_EGRID( "CASE.EGRID" , output_unit = EclUnitTypeEnum.ECL_FIELD_UNITS )
            f = EclFile("CASE.EGRID")
            g = f["GRIDUNIT"][0]
            self.assertEqual( g[0].strip( ) , "FEET" )
            g2 = EclGrid("CASE.EGRID")
            self.assertFloatEqual( g2.cell_volume( global_index = 0 ) , 3.28084*3.28084*3.28084)


            grid.save_EGRID( "CASE.EGRID" )
            f = EclFile("CASE.EGRID")
            g = f["GRIDUNIT"][0]
            self.assertEqual( g[0].strip( ) , "METRES" )

            grid.save_EGRID( "CASE.EGRID" , output_unit = EclUnitTypeEnum.ECL_LAB_UNITS)
            f = EclFile("CASE.EGRID")
            g = f["GRIDUNIT"][0]
            self.assertEqual( g[0].strip() , "CM" )
            g2 = EclGrid("CASE.EGRID")
            self.assertFloatEqual( g2.cell_volume( global_index = 0 ) , 100*100*100 )

    def test_volume(self):
        dim     = (10,10,10)
        dV      = (2,2,2)

        grids = createVolumeTestGridBase(dim, dV)
        for grid in grids:
            tot_vol = createWrapperGrid(grid).cell_volume(0)
            cell_volumes = [grid.cell_volume(i) for i in range(grid.getGlobalSize())]
            self.assertTrue(min(cell_volumes) >= 0)
            self.assertFloatEqual(sum(cell_volumes), tot_vol)

    def test_unique_containment(self):
        test_base = createContainmentTestBase()

        for steps_per_unit, grid in test_base:
            wgrid = createWrapperGrid(grid)

            (xmin, xmax), (ymin, ymax), (zmin, zmax) = getMinMaxValue(wgrid)
            x_space = linspace(xmin-1, xmax+1, (xmax-xmin+2)*steps_per_unit+1)
            y_space = linspace(ymin-1, ymax+1, (ymax-ymin+2)*steps_per_unit+1)
            z_space = linspace(zmin-1, zmax+1, (zmax-zmin+2)*steps_per_unit+1)

            for x, y, z in itertools.product(x_space, y_space, z_space):
                hits = [
                            grid.cell_contains(x, y, z, i)
                            for i in range(grid.getGlobalSize())
                        ].count(True)

                self.assertIn(hits, [0, 1])

                expected = 1 if wgrid.cell_contains(x, y, z, 0) else 0
                self.assertEqual(
                        expected,
                        hits,
                        'Expected %d for (%g,%g,%g), got %d' % (expected, x, y, z, hits)
                        )

    def test_cell_corner_containment(self):
        n = 4
        d = 10
        grid = GridGen.createRectangular( (n, n, n), (d, d, d))

        for x, y, z in itertools.product(range(0, n*d+1, d), repeat=3):
            self.assertEqual(
                    1,
                    [grid.cell_contains(x, y, z, i) for i in range(n**3)].count(True)
                    )

    def test_cell_corner_containment_compatability(self):
        grid = GridGen.createRectangular( (3,3,3), (1,1,1) )

        for x, y, z in itertools.product(range(4), repeat=3):
            for i in range(27):
                if grid.cell_contains(x, y, z, i):
                    self.assertEqual(
                            CORNER_HOME[(x,y,z)],
                            i
                            )

    def test_cell_face_containment(self):
        n = 4
        d = 10
        grid = GridGen.createRectangular( (n, n, n), (d, d, d))

        for x, y, z in itertools.product(range(d//2, n*d, d), repeat=3):
            for axis, direction in itertools.product(range(3), [-1, 1]):
                p = [x, y, z]
                p[axis] = p[axis] + direction*d/2
                self.assertEqual(
                        1,
                        [grid.cell_contains(p[0], p[1], p[2], i) for i in range(n**3)].count(True)
                    )

    # This test generates a cell that is concave on ALL 6 sides
    def test_concave_cell_containment(self):
        points = [
            (5, 5, 5),
            (20, 10, 10),
            (10, 20, 10),
            (25, 25, 5),
            (10, 10, 20),
            (25, 5, 25),
            (5, 25, 25),
            (20, 20, 20)
            ]

        grid = GridGen.create_single_cell_grid(points)

        assertPoint = lambda p : self.assertTrue(
                grid.cell_contains(p[0], p[1], p[2], 0)
                )

        assertNotPoint = lambda p : self.assertFalse(
                grid.cell_contains(p[0], p[1], p[2], 0)
                )

        # Cell center
        assertPoint(average(points));

        # "Side" center
        assertNotPoint(average(points[0:4:]))
        assertNotPoint(average(points[4:8:]))
        assertNotPoint(average(points[1:8:2]))
        assertNotPoint(average(points[0:8:2]))
        assertNotPoint(average(points[0:8:4] + points[1:8:4]))
        assertNotPoint(average(points[2:8:4] + points[3:8:4]))

        # Corners
        for p in points:
            assertPoint(p)

        # Edges
        edges = ([(i, i+1) for i in range(0, 8, 2)] +
                 [(i, i+2) for i in [0, 1, 4, 5]] +
                 [(i, i+4) for i in range(4)] +
                 [(1,2), (2,7), (1,7), (4,7), (2,4), (4,1)])
        for a,b in edges:
            assertPoint(average([points[a], points[b]]))

        # Epsilon inside from corners
        middle_point = average(points)
        for p in points:
            assertPoint(average(20*[p] + [middle_point]))

        # Espilon outside
        middle_point[2] = 0
        for p in points[0:4:]:
            assertNotPoint(average(20*[p] + [middle_point]))

        middle_point[2] = 30
        for p in points[4:8:]:
            assertNotPoint(average(20*[p] + [middle_point]))

    # This test generates a cell that is strictly convex on ALL 6 sides
    def test_concvex_cell_containment(self):
        points = [
            (10, 10, 10),
            (25, 5, 5),
            (5, 25, 5),
            (20, 20, 10),
            (5, 5, 25),
            (20, 10, 20),
            (10, 20, 20),
            (25, 25, 25)
            ]

        grid = GridGen.create_single_cell_grid(points)

        assertPoint = lambda p : self.assertTrue(
                grid.cell_contains(p[0], p[1], p[2], 0)
                )

        assertNotPoint = lambda p : self.assertFalse(
                grid.cell_contains(p[0], p[1], p[2], 0)
                )

        # Cell center
        assertPoint(average(points));

        # "Side" center
        assertPoint(average(points[0:4:]))
        assertPoint(average(points[4:8:]))
        assertPoint(average(points[1:8:2]))
        assertPoint(average(points[0:8:2]))
        assertPoint(average(points[0:8:4] + points[1:8:4]))
        assertPoint(average(points[2:8:4] + points[3:8:4]))

        # Corners
        for p in points:
            assertPoint(p)

        # Edges
        edges = ([(i, i+1) for i in range(0, 8, 2)] +
                 [(i, i+2) for i in [0, 1, 4, 5]] +
                 [(i, i+4) for i in range(4)] +
                 [(1,2), (2,7), (1,7), (4,7), (2,4), (4,1)])
        for a,b in edges:
            assertPoint(average([points[a], points[b]]))

        # Epsilon inside from corners
        middle_point = average(points)
        for p in points:
            assertPoint(average(20*[p] + [middle_point]))

        # Espilon outside
        middle_point[2] = 0
        for p in points[0:4:]:
            assertNotPoint(average(20*[p] + [middle_point]))

        middle_point[2] = 30
        for p in points[4:8:]:
            assertNotPoint(average(20*[p] + [middle_point]))
