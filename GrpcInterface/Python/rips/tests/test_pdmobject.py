import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
from rips import RipsError


def test_exception_on_object_method(rips_instance, initialize_test):
    name = "the name"
    nx = 12
    ny = 13
    nz = -1

    with pytest.raises(RipsError, match=r"Invalid grid size"):
        rips_instance.project.create_corner_point_grid(name, nx, ny, nz, [], [], [])
