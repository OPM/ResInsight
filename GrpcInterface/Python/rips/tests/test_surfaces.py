import sys
import os
import tempfile
import pytest
from pathlib import Path

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_create_and_export_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    surface = surface_collection.new_surface(case, 5)
    assert surface

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        path = Path(tmpdirname, "mysurface.ts")
        print("Temporary folder: ", path.as_posix())

        fname = surface.export_to_file(path.as_posix())
        assert len(fname.values) == 1

        assert path.exists()


def test_create_regular_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    bbox = c.reservoir_boundingbox()
    depth = bbox.max_z - ((bbox.max_z - bbox.min_z) / 2.0)

    origin_x = bbox.min_x
    origin_y = bbox.min_y
    name = "test surface name"
    nx = 20
    ny = 10
    increment_x = (bbox.max_x - bbox.min_x) / float(nx)
    increment_y = (bbox.max_y - bbox.min_y) / float(ny)
    rotation = 23.4

    s = surface_collection.new_regular_surface(
        name=name,
        origin_x=origin_x,
        origin_y=origin_y,
        depth=-depth,
        nx=nx,
        ny=ny,
        increment_x=increment_x,
        increment_y=increment_y,
        rotation=rotation,
    )

    assert s.origin_x == origin_x
    assert s.origin_y == origin_y
    assert s.increment_x == increment_x
    assert s.increment_y == increment_y
    assert s.rotation == rotation
    assert s.nx == nx
    assert s.ny == ny

    with pytest.raises(rips.RipsError, match="Failed to set depth property."):
        s.set_property_as_depth(name="non_existing_property")

    with pytest.raises(
        rips.RipsError, match="Failed to set property: incorrect dimensions."
    ):
        s.set_property("property_too_big", [i for i in range(nx * ny * 2)])

    # Sucessfully set the depth property
    s.set_property("depth", [i for i in range(nx * ny)])
    s.set_property_as_depth("depth")

    # Change the dimensions of the surface: should invalidate the properties
    s.nx = 30
    s.update()

    # 'depth' property is now gone
    with pytest.raises(rips.RipsError, match="Failed to set depth property."):
        s.set_property_as_depth(name="depth")


def test_create_regular_surface_invalid_values(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    with pytest.raises(rips.RipsError, match="Invalid nx"):
        surface_collection.new_regular_surface(nx=-1)

    with pytest.raises(rips.RipsError, match="Invalid ny"):
        surface_collection.new_regular_surface(ny=-1)

    with pytest.raises(rips.RipsError, match="Invalid increment X. Must be positive."):
        surface_collection.new_regular_surface(increment_x=-1.0)

    with pytest.raises(rips.RipsError, match="Invalid increment Y. Must be positive."):
        surface_collection.new_regular_surface(increment_y=-1.0)

    with pytest.raises(rips.RipsError, match="Invalid rotation."):
        surface_collection.new_regular_surface(rotation=-1.0)
