#!/usr/bin/env python
#  Copyright (C) 2017  Statoil ASA, Norway. 
#   
#  The file 'test_grid_generator.py' is part of ERT - Ensemble based Reservoir Tool.
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

from itertools import product as prod
import operator, random, numpy

from ecl.ecl import EclGrid, EclKW, EclDataType
from ecl.ecl import EclGridGenerator as GridGen
from ecl.test import ExtendedTestCase, TestAreaContext


def generate_ijk_bounds(dims):
    ibounds = [(l, u) for l in range(dims[0]) for u in range(l, dims[0])]
    jbounds = [(l, u) for l in range(dims[1]) for u in range(l, dims[1])]
    kbounds = [(l, u) for l in range(dims[2]) for u in range(l, dims[2])]
    return prod(ibounds, jbounds, kbounds)

def decomposition_preserving(ijk_bound):
    return sum(zip(*ijk_bound)[0])%2 is 0

class GridGeneratorTest(ExtendedTestCase):

    def setUp(self):
        self.test_base = [
            (
                list(GridGen.create_coord((4,4,4), (1,1,1))),
                list(GridGen.create_zcorn((4,4,4), (1,1,1), offset=0)),
                GridGen.create_grid((4,4,4), (1,1,1), offset=0)
            ),
            (
                list(
                    GridGen.create_coord((5,5,5), (1,1,1),
                    translation=(10,10,0))
                    ),
                list(
                    GridGen.create_zcorn((5,5,5), (1,1,1), offset=0.5,
                    irregular_offset=True, concave=True, irregular=True)
                    ),
                GridGen.create_grid(
                    (5,5,5), (1,1,1), offset=0.5,
                    irregular=True, irregular_offset=True, concave=True,
                    translation=(10,10,0)
                    )
            )
            ]

    def test_extract_grid_decomposition_change(self):
        dims = (4,4,4)
        zcorn = list(GridGen.create_zcorn(dims, (1,1,1), offset=0))
        coord = list(GridGen.create_coord(dims, (1,1,1)))

        ijk_bounds = generate_ijk_bounds(dims)
        for ijk_bounds in ijk_bounds:
            if decomposition_preserving(ijk_bounds):
                GridGen.extract_subgrid_data(dims, coord, zcorn, ijk_bounds)
            else:
                with self.assertRaises(ValueError):
                    GridGen.extract_subgrid_data(dims, coord, zcorn, ijk_bounds)

            GridGen.extract_subgrid_data(dims,
                                 coord, zcorn,
                                 ijk_bounds,
                                 decomposition_change=True)

    def test_extract_grid_invalid_bounds(self):
        dims = (3,3,3)
        zcorn = list(GridGen.create_zcorn(dims, (1,1,1), offset=0))
        coord = list(GridGen.create_coord(dims, (1,1,1)))

        with self.assertRaises(ValueError):
            GridGen.extract_subgrid_data(dims, coord, zcorn, ((-1,0), (2,2), (2,2)))

        with self.assertRaises(ValueError):
            GridGen.extract_subgrid_data(dims, coord, zcorn, ((1,6), (2,2), (2,2)))
                
        with self.assertRaises(ValueError):
            GridGen.extract_subgrid_data(dims, coord, zcorn, ((1,2), (2,0), (2,2)))

    def test_extract_grid_slice_spec(self):
        dims = (4,4,4)
        zcorn = list(GridGen.create_zcorn(dims, (1,1,1), offset=0))
        coord = list(GridGen.create_coord(dims, (1,1,1)))

        ijk_bounds = generate_ijk_bounds(dims)
        for ijk in ijk_bounds:
            ijk = list(ijk)
            for i in range(3):
                if len(set(ijk[i])) == 1:
                    ijk[i] = ijk[i][0]

            GridGen.extract_subgrid_data(dims,
                                 coord, zcorn,
                                 ijk,
                                 decomposition_change=True)

    def assertSubgrid(self, grid, subgrid, ijk_bound):
        sijk_space = prod(*[range(d) for d in subgrid.getDims()[:-1:]])
        for sijk in sijk_space:
            gijk = tuple([a+b for a, b in zip(sijk, zip(*ijk_bound)[0])])

            self.assertEqual(
                [subgrid.getCellCorner(i, ijk=sijk) for i in range(8)],
                [grid.getCellCorner(i, ijk=gijk) for i in range(8)]
                )

            self.assertEqual(grid.active(ijk=gijk), subgrid.active(ijk=sijk))

    def test_validate_cells(self):
        for coord, zcorn, grid in self.test_base:
            grid_dims = grid.getDims()[:-1:]
            ijk_bounds = generate_ijk_bounds(grid_dims)
            for ijk_bound in ijk_bounds:
                if not decomposition_preserving(ijk_bound):
                    continue

                sub_dims = tuple([u-l+1 for l, u in ijk_bound])
                sub_coord, sub_zcorn, _ = GridGen.extract_subgrid_data(
                                                    grid_dims,
                                                    coord,
                                                    zcorn,
                                                    ijk_bound
                                                    )

                subgrid = EclGrid.create(sub_dims, sub_zcorn, sub_coord, None)
                self.assertEqual(sub_dims, subgrid.getDims()[:-1:])
                self.assertSubgrid(grid, subgrid, ijk_bound)

    def test_actnum_extraction(self):
        dims = (4,4,4)

        coord = GridGen.create_coord(dims, (1,1,1))
        zcorn = GridGen.create_zcorn(dims, (1,1,1), offset=0)

        actnum = EclKW("ACTNUM", reduce(operator.mul, dims), EclDataType.ECL_INT)
        random.seed(1337)
        for i in range(len(actnum)):
            actnum[i] = random.randint(0, 1)

        grid = EclGrid.create(dims, zcorn, coord, actnum)

        ijk_bounds = generate_ijk_bounds(dims)
        for ijk_bound in ijk_bounds:
            if not decomposition_preserving(ijk_bound):
                continue

            sub = GridGen.extract_subgrid_data(
                                         dims,
                                         coord,
                                         zcorn,
                                         ijk_bound,
                                         actnum=actnum
                                       )

            sub_coord, sub_zcorn, sub_actnum = sub
            sub_dims = tuple([u-l+1 for l, u in ijk_bound])
            subgrid = EclGrid.create(sub_dims, sub_zcorn, sub_coord, sub_actnum)
            self.assertEqual(sub_dims, subgrid.getDims()[:-1:])
            self.assertSubgrid(grid, subgrid, ijk_bound)

    def test_translation(self):
        dims = (3,3,3)

        coord = GridGen.create_coord(dims, (1,1,1))
        zcorn = GridGen.create_zcorn(dims, (1,1,1), offset=0)
        grid = EclGrid.create(dims, zcorn, coord, None)

        ijk_bound = [(0, d-1) for d in dims]
        translation = (1, 2, 3)
        sub_coord, sub_zcorn, _ = GridGen.extract_subgrid_data(
                                                        dims,
                                                        coord,
                                                        zcorn,
                                                        ijk_bound,
                                                        translation=translation
                                                       )

        tgrid = EclGrid.create(dims, sub_zcorn, sub_coord, None)
        self.assertEqual(grid.getGlobalSize(), tgrid.getGlobalSize())

        for gi in range(grid.getGlobalSize()):
            translation = numpy.array(translation)
            corners = [grid.getCellCorner(i, gi) for i in range(8)]
            corners = [tuple(numpy.array(c)+translation) for c in corners]

            tcorners = [tgrid.getCellCorner(i, gi) for i in range(8)]

            self.assertEqual(corners, tcorners)

    def test_subgrid_extration(self):
        for _, _, grid in self.test_base[:-1:]:
            grid_dims = grid.getDims()[:-1:]
            ijk_bounds = generate_ijk_bounds(grid_dims)
            for ijk_bound in ijk_bounds:
                if not decomposition_preserving(ijk_bound):
                    continue

                sub_dims = tuple([u-l+1 for l, u in ijk_bound])
                subgrid = GridGen.extract_subgrid(grid, ijk_bound)

                self.assertEqual(sub_dims, subgrid.getDims()[:-1:])
                self.assertSubgrid(grid, subgrid, ijk_bound)

    def test_subgrid_translation(self):
        grid = GridGen.create_grid((4,4,4), (1,1,1), offset=0.5,
                    irregular=True, irregular_offset=True, concave=True,
                    translation=(10,10,0))

        # Create grid with MAPAXES
        mapaxes = EclKW("MAPAXES", 6, EclDataType.ECL_FLOAT)
        for i, val in enumerate([1200, 1400, 2500, 2500, 3700, 4000]):
            mapaxes[i] = val

        grid = EclGrid.create(
                grid.getDims(),
                grid.export_zcorn(),
                grid.export_coord(),
                None,
                mapaxes=mapaxes
                )

        for translation in [
                (0,0,0),
                (10, 10, 100),
                (-1, -1, -1)
                ]:
            subgrid = GridGen.extract_subgrid(
                                        grid,
                                        ((0,3), (0,3), (0,3)),
                                        translation=translation
                                        )

            self.assertEqual(grid.getDims(), subgrid.getDims())

            translation = numpy.array(translation)
            for gindex in range(grid.getGlobalSize()):
                grid_corners = [
                                grid.getCellCorner(i, global_index=gindex)
                                for i in range(8)
                              ]

                subgrid_corners = [
                                subgrid.getCellCorner(i, global_index=gindex)
                                for i in range(8)
                                ]

                subgrid_corners = [
                                list(numpy.array(corner) - translation)
                                for corner in subgrid_corners
                                ]

                for gc, sc in zip(grid_corners, subgrid_corners):
                    self.assertAlmostEqualList(
                            gc,
                            sc,
                            msg="Failed to translate corners correctly." +
                                "Expected %s, was %s." % (gc, sc),
                            tolerance=10e-10
                            )
