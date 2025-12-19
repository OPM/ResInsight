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
    point_x = 5039.84
    point_y = 6303.76
    point_z = 4144.21

    distance, faultname, facename = case.distance_to_closest_fault( point_x, point_y, point_z)

    assert faultname == "Undefined Grid Faults"
    assert facename == "J+"
    assert math.isclose(distance, 533.57, abs_tol=0.1)

    # another test point
    point_x = 4656.43
    point_y = 4713.60
    point_z = 4147.21

    distance, faultname, facename = case.distance_to_closest_fault( point_x, point_y, point_z)

    assert faultname == "Undefined Grid Faults"
    assert facename == "J+"
    assert math.isclose(distance, 225.28, abs_tol=0.1)
