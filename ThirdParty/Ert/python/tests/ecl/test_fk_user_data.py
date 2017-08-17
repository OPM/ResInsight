#!/usr/bin/env python
#  Copyright (C) 2017  Statoil ASA, Norway. 
#   
#  The file 'test_ecl_cell_containment.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ecl.ecl import EclGrid
from ecl.test import ExtendedTestCase, TestAreaContext

class FKTest(ExtendedTestCase):

    def test_cell_containment(self):

        grid_location = "local/ECLIPSE/faarikaal/faarikaal%d.EGRID"
        well_location = "local/ECLIPSE/faarikaal/faarikaal%d.txt"

        for i in range(1, 8):
            grid_file = self.createTestPath(grid_location % i)
            well_file = self.createTestPath(well_location % i)

            grid = EclGrid(grid_file)

            # Load well data
            with open(well_file, "r") as f:
                lines = [line.split() for line in f.readlines()]

            points = [map(float, line[:3:]) for line in lines]
            exp_cells = [tuple(map(int, line[3::])) for line in lines]

            msg = "Expected point %s to be in cell %s, was in %s."
            for point, exp_cell in zip(points, exp_cells):
                reported_cell = grid.find_cell(*point)
                self.assertEqual(
                        exp_cell,
                        reported_cell,
                        msg % (str(point), str(exp_cell), str(reported_cell))
                        )
