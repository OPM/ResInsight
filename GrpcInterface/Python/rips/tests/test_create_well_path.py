import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_add_new_object_for_well_paths(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    my_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    my_well_path.name = "test"
    my_well_path.update()

    geometry = my_well_path.well_path_geometry()
    geometry.add_new_object(rips.WellPathTarget)
    geometry.add_new_object(rips.WellPathTarget)
    geometry.add_new_object(rips.WellPathTarget)
    assert len(geometry.well_path_targets()) == 3

    assert len(well_path_coll.well_paths()) == 1
    my_well_path_duplicate = well_path_coll.well_paths()[0]
    assert my_well_path_duplicate.name == "test"
    geometry_duplicate = my_well_path_duplicate.well_path_geometry()
    assert len(geometry_duplicate.well_path_targets()) == 3

    # Not allowed to add object of unrelated type
    invalid_object = geometry.add_new_object(rips.WellPath)
    assert invalid_object is None


def test_add_well_path_targets(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    my_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    my_well_path.name = "test"
    my_well_path.update()

    geometry = my_well_path.well_path_geometry()

    # Append well target with fixed azimuth
    coord = [2229.10, -833.74, -74.70]
    target = geometry.append_well_target(
        coord, use_fixed_azimuth=True, fixed_azimuth_value=110.1
    )
    assert not target.use_fixed_inclination
    assert target.use_fixed_azimuth
    assert target.azimuth == 110.1
    assert target.inclination == 0.0

    # Append well target with fixed inclination
    coord = [4577.21, -3043.47, -87.15]
    target = geometry.append_well_target(
        coord, use_fixed_inclination=True, fixed_inclination_value=25.6
    )

    assert target.use_fixed_inclination
    assert not target.use_fixed_azimuth
    assert target.azimuth == 0.0
    assert target.inclination == 25.6


def test_duplicate_well_path(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    my_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    my_well_path.name = "test"
    my_well_path.update()

    new_well_path = my_well_path.duplicate()
    assert new_well_path is not None
    assert new_well_path.name != my_well_path.name
