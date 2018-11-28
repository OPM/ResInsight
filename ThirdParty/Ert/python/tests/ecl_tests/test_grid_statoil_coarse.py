#  Copyright (C) 2018  Statoil ASA, Norway.
#
#  The file 'test_grid_statoil_coarse.py' is part of ERT - Ensemble based Reservoir Tool.
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
class GridCoarceTest(EclTest):

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
