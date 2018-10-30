#  Copyright (C) 2018  Statoil ASA, Norway.
#
#  The file 'test_grid_statoil_large_case.py' is part of ERT - Ensemble based Reservoir Tool.
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
class GridLargeCaseTest(EclTest):

    def test_large_case(self):
        grdecl_file = self.createTestPath("Statoil/ECLIPSE/1.6.0_issueGrdecl/test_aug2016_gridOnly.grdecl")
        grid = EclGrid.loadFromGrdecl( grdecl_file )
