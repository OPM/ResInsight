#!/usr/bin/env python
#  Copyright (C) 2017  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

from ecl.grid import Cell, EclGrid
from tests import EclTest
from unittest import skipUnless

class CellTest(EclTest):

    def setUp(self):
        fk = self.createTestPath('local/ECLIPSE/faarikaal/faarikaal1.EGRID')
        self.grid = EclGrid(fk)
        self.cell = self.grid[3455]
        self.actives = [c for c in self.grid if c.active]

    def test_init(self):
        cell = self.grid[0]
        self.assertEqual(0, cell.global_index)

    def test_actives(self):
        self.assertEqual(4160, len(self.actives))

    def test_volumes(self):
        actives = self.actives
        vols = [c.volume for c in actives]
        vmin, vmax = min(vols), max(vols)
        self.assertFloatEqual(vmin, 1332.328921)
        self.assertFloatEqual(vmax, 10104.83204)
        self.assertFloatEqual(actives[2000].dz, 1.68506)

        act_dim = (48.5676, 54.1515, 1.685)  # cell dimension in meter
        cel_dim = self.cell.dimension
        self.assertEqual(3, len(cel_dim))
        for d in range(2):
            self.assertFloatEqual(act_dim[d], cel_dim[d])

    def test_indices(self):
        c = self.cell
        self.assertEqual(3455, c.global_index)
        self.assertEqual(2000, c.active_index)
        self.assertEqual((4,1,82), c.ijk)
        self.assertEqual(c.ijk, (c.i, c.j, c.k))

    def test_coordinates(self):
        c = self.cell
        corners = c.corners
        self.assertEqual(8, len(corners))
        se0 = corners[5] # upper south east
        se0_act = (606900.002, 5202050.07, 5149.26)
        for i in range(3):
            self.assertFloatEqual(se0[i], se0_act[i])

        coordinate = c.coordinate
        coor_act = (606866.883, 5202064.939, 5154.52)

        for i in range(3):
            self.assertFloatEqual(coordinate[i], coor_act[i])

        xyz = c.coordinate
        self.assertIn(xyz, c)

    def test_eq(self):
        c1 = self.cell
        c2 = self.grid[4,1,82]
        c3 = self.grid[1,1,82]
        self.assertEqual(c1,c2)
        self.assertEqual(c2,c1)
        self.assertNotEqual(c1, 'notacell')
        self.assertNotEqual(c1,c3)
        self.assertNotEqual(c3,c1)

    def test_getitem_3(self):
        c = self.cell
        d = self.grid[4,1,82]
        self.assertEqual(c, d)

    def test_validity(self):
        self.assertTrue(self.cell.valid_geometry)
        self.assertTrue(self.cell.valid)

    def test_repr(self):
        c = self.cell
        r = repr(c)
        self.assertTrue(r.startswith('Cell(4, 1, 82, active, '))
        self.assertIn('faarikaal1.EGRID', r)

