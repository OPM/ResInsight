import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_10k(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)
    assert(case.grid_count() == 2)
    grid = case.grid(index=0)
    dimensions = grid.dimensions()
    assert(dimensions.i == 90)
    assert(dimensions.j == 96)
    assert(dimensions.k == 36)
