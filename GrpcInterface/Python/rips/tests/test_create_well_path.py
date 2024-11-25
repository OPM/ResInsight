from rips.generated.generated_classes import (
    ModeledWellPath,
    StimPlanModel,
    WellPathGeometry,
    WellPathTarget,
)
import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_add_new_object_for_well_paths(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]

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
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]

    my_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    my_well_path.name = "test"
    my_well_path.update()

    geometry = my_well_path.well_path_geometry()

    # Append well target with fixed azimuth
    coord = [2229.10, -833.74, -74.70]
    target = geometry.append_well_target(
        coord, use_fixed_azimuth=True, fixed_azimuth_value=110.1
    )
    assert target.use_fixed_inclination == False
    assert target.use_fixed_azimuth == True
    assert target.azimuth == 110.1
    assert target.inclination == 0.0

    # Append well target with fixed inclination
    coord = [4577.21, -3043.47, -87.15]
    target = geometry.append_well_target(
        coord, use_fixed_inclination=True, fixed_inclination_value=25.6
    )

    assert target.use_fixed_inclination == True
    assert target.use_fixed_azimuth == False
    assert target.azimuth == 0.0
    assert target.inclination == 25.6


def test_add_well_path_completions(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]

    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test"
    well_path.update()

    # Update the completion settings
    completions_settings = well_path.completion_settings()
    completions_settings.msw_roughness = 12.34
    completions_settings.msw_liner_diameter = 0.2123
    completions_settings.well_name_for_export = "file name"
    completions_settings.group_name_for_export = "msj"
    completions_settings.well_type_for_export = "GAS"
    completions_settings.update()  # Commit updates back to ResInsight

    completions_settings_updated = well_path.completion_settings()
    assert completions_settings_updated.msw_roughness == 12.34
    assert completions_settings_updated.msw_liner_diameter == 0.2123
    assert completions_settings_updated.well_name_for_export == "file name"
    assert completions_settings_updated.group_name_for_export == "msj"
    assert completions_settings_updated.well_type_for_export == "GAS"

    msw_settings = well_path.msw_settings()
    msw_settings.custom_values_for_lateral = True
    msw_settings.enforce_max_segment_length = True
    msw_settings.liner_diameter = 20.0
    msw_settings.max_segment_length = 123.05
    msw_settings.pressure_drop = "HFA"
    msw_settings.reference_md_type = "UserDefined"
    msw_settings.roughness_factor = 1.3
    msw_settings.user_defined_reference_md = 1234.56
    msw_settings.update()

    msw_settings_updated = well_path.msw_settings()
    assert msw_settings_updated.custom_values_for_lateral == True
    assert msw_settings_updated.enforce_max_segment_length == True
    assert msw_settings_updated.liner_diameter == 20.0
    assert msw_settings_updated.max_segment_length == 123.05
    assert msw_settings_updated.pressure_drop == "HFA"
    assert msw_settings_updated.reference_md_type == "UserDefined"
    assert msw_settings_updated.roughness_factor == 1.3
    assert msw_settings_updated.user_defined_reference_md == 1234.56
