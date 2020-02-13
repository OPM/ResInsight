import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_10k(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)
    assert(len(case.grids()) == 2)
    grid = case.grid(index=0)
    dimensions = grid.dimensions()
    assert(dimensions.i == 90)
    assert(dimensions.j == 96)
    assert(dimensions.k == 36)

    cell_centers = grid.cell_centers()
    assert(len(cell_centers) == (dimensions.i * dimensions.j * dimensions.k))
