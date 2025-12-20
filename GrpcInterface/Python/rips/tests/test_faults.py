import sys
import os
import math
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_faultDistance(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )

    # a test point
    point_x = 4817.84
    point_y = 5204.76
    point_z = 4137.21

    faultname, distance, facename = case.distance_to_closest_fault(
        point_x, point_y, point_z
    )

    # Fault name is unstable between grid readers, so we skip this check
    # assert faultname == "Undefined Grid Faults"

    assert facename == "I+"
    assert math.isclose(distance, 53.84, abs_tol=0.1)
