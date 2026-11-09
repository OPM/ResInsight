import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
from rips import ModeledWellPath, RipsError


def test_exception_on_object_method(rips_instance, initialize_test):
    name = "the name"
    nx = 12
    ny = 13
    nz = -1

    with pytest.raises(RipsError, match=r"Invalid grid size"):
        rips_instance.project.create_corner_point_grid(name, nx, ny, nz, [], [], [])


def test_non_ascii_text_field(rips_instance, initialize_test):
    name = "Brønn æøå ≈ →"

    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(ModeledWellPath)
    well_path.name = name
    well_path.update()

    assert rips_instance.project.descendants(ModeledWellPath)[0].name == name
