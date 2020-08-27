import unittest
import sys
import numpy as np
import datetime

from opm.io.ecl import EGrid
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path



class TestEGrid(unittest.TestCase):

    def test_ijk_active_and_global_indices(self):

        with self.assertRaises(ValueError):
            EGrid("/file/that/does_not_exists")

        grid1 = EGrid(test_path("data/9_EDITNNC.EGRID"))

        self.assertEqual(grid1.active_cells, 2794)

        nI,nJ,nK = grid1.dimension
        tot_ant_cells = nI*nJ*nK

        self.assertEqual( nI, 13)
        self.assertEqual( nJ, 22)
        self.assertEqual( nK, 11)

        i,j,k = grid1.ijk_from_global_index(0)

        self.assertEqual( i, 0)
        self.assertEqual( j, 0)
        self.assertEqual( k, 0)

        i,j,k = grid1.ijk_from_global_index(1000)

        self.assertEqual( i, 12)
        self.assertEqual( j, 10)
        self.assertEqual( k, 3)

        self.assertEqual( grid1.global_index(12, 10, 3), 1000 )

        i,j,k = grid1.ijk_from_global_index(tot_ant_cells - 1)

        self.assertEqual( i, nI -1 )
        self.assertEqual( j, nJ -1 )
        self.assertEqual( k, nK -1 )

        with self.assertRaises(ValueError):
            i,j,k = grid1.ijk_from_global_index(tot_ant_cells)

        i,j,k = grid1.ijk_from_active_index( 1000 )

        self.assertEqual( i, 1 )
        self.assertEqual( j, 15 )
        self.assertEqual( k, 3 )

        self.assertEqual( grid1.active_index(1, 15, 3), 1000 )


    def test_coordinates(self):

        Xref=[2899.45166015625, 2999.390869140625, 2899.45166015625, 2999.390869140625,
              2899.4176237656716, 2999.3568089317187, 2899.417623015281, 2999.356808099622]
        Yref=[2699.973388671875, 2699.973388671875, 2799.969482421875, 2799.969482421875,
              2699.9818918149376, 2699.9818918149376, 2799.978009571257, 2799.9780095915808]
        Zref=[2565.301025390625, 2568.791015625, 2564.42822265625, 2567.918212890625,
              2575.29443359375, 2578.784423828125, 2574.421875, 2577.911865234375]

        grid1 = EGrid(test_path("data/9_EDITNNC.EGRID"))

        X1, Y1, Z1 = grid1.xyz_from_ijk(9, 7, 0)

        for n in range(0,8):
            self.assertAlmostEqual(Xref[n], X1[n], 8)
            self.assertAlmostEqual(Yref[n], Y1[n], 8)
            self.assertAlmostEqual(Zref[n], Z1[n], 8)

        actInd = grid1.active_index(9, 7, 0);
        X2, Y2, Z2 = grid1.xyz_from_active_index(actInd)

        for n in range(0,8):
            self.assertAlmostEqual(Xref[n], X2[n], 8)
            self.assertAlmostEqual(Yref[n], Y2[n], 8)
            self.assertAlmostEqual(Zref[n], Z2[n], 8)


    def test_cell_volume(self):

        grid1 = EGrid(test_path("data/9_EDITNNC.EGRID"))

        nI,nJ,nK = grid1.dimension
        tot_ant_cells = nI*nJ*nK

        celVolAll = grid1.cellvolumes()

        self.assertTrue(isinstance(celVolAll, np.ndarray))
        self.assertEqual(len(celVolAll), tot_ant_cells)

        self.assertTrue(min(celVolAll) > 0.0)

        mask =[0]*tot_ant_cells

        for k in range(nK):
            globInd=grid1.global_index(0,0,k)
            mask[globInd] = 1

        celVol = grid1.cellvolumes(mask)

        self.assertTrue(min(celVol) == 0.0)
        self.assertEqual(np.count_nonzero(celVol) , nK)


if __name__ == "__main__":

    unittest.main()

