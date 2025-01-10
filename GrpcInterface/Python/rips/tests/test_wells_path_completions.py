import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_10k(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    project_path = case_root_path + "/well_completions_pytest.rsp"
    project = rips_instance.project.open(path=project_path)

    export_folder = tempfile.gettempdir()

    rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

    case = project.cases()[0]
    case.export_well_path_completions(
        time_step=1,
        well_path_names=["Well-1"],
        file_split="UNIFIED_FILE",
    )


def test_add_well_path_completions(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]

    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test"
    well_path.update()

    # Update the completion settings
    completions_settings = well_path.completion_settings()
    completions_settings.allow_well_cross_flow = True
    completions_settings.auto_well_shut_in = "STOP"
    completions_settings.drainage_radius_for_pi = 1.56
    completions_settings.fluid_in_place_region = 99
    completions_settings.gas_inflow_eq = "R-G"
    completions_settings.group_name_for_export = "TestGroup"
    completions_settings.hydrostatic_density = "AVG"
    completions_settings.msw_liner_diameter = 0.12
    completions_settings.msw_roughness = 4.66
    completions_settings.reference_depth_for_export = 1234
    completions_settings.well_bore_fluid_pvt_table = 33
    completions_settings.well_name_for_export = "TestWellName"
    completions_settings.well_type_for_export = "LIQUID"
    completions_settings.update()  # Commit updates back to ResInsight

    completions_settings_updated = well_path.completion_settings()
    assert completions_settings_updated.allow_well_cross_flow == True
    assert completions_settings_updated.auto_well_shut_in == "STOP"
    assert completions_settings_updated.drainage_radius_for_pi == "1.56"
    assert completions_settings_updated.fluid_in_place_region == 99
    assert completions_settings_updated.gas_inflow_eq == "R-G"
    assert completions_settings_updated.group_name_for_export == "TestGroup"
    assert completions_settings_updated.hydrostatic_density == "AVG"
    assert completions_settings_updated.msw_liner_diameter == 0.12
    assert completions_settings_updated.msw_roughness == 4.66
    assert completions_settings_updated.reference_depth_for_export == "1234"
    assert completions_settings_updated.well_bore_fluid_pvt_table == 33
    assert completions_settings_updated.well_name_for_export == "TestWellName"
    assert completions_settings_updated.well_type_for_export == "LIQUID"

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


def test_add_well_path_fracture_template(rips_instance, initialize_test):

    # Add test for all properties
    # Some properties depend on availablility of other data and is not tested, these tests are commented out

    fracture_template = rips_instance.project.descendants(rips.FractureTemplate)[0]
    fracture_template.azimuth_angle = 23.0
    # fracture_template.beta_factor_type = "FractureBetaFactor"
    fracture_template.conductivity_factor = 12.5
    fracture_template.conductivity_type = "FiniteConductivity"
    fracture_template.d_factor_scale_factor = 1.2
    fracture_template.effective_permeability = 55
    fracture_template.fracture_width = 0.5
    fracture_template.fracture_width_type = "UserDefinedWidth"
    fracture_template.gas_viscosity = 0.1
    fracture_template.height_scale_factor = 1.2
    fracture_template.height_scale_factor = 4
    fracture_template.inertial_coefficient = 0.7
    fracture_template.non_darcy_flow_type = "Computed"
    fracture_template.orientation = "Azimuth"
    fracture_template.perforation_length = 5
    fracture_template.permeability_type = "UserDefinedPermeability"
    fracture_template.relative_gas_density = 0.1
    fracture_template.relative_permeability = 0.2
    fracture_template.user_defined_d_factor = 14
    fracture_template.user_defined_perforation_length = True
    fracture_template.user_description = "my frac name"
    fracture_template.width_scale_factor = 7

    fracture_template.update()

    fracture_template_updated = rips_instance.project.descendants(
        rips.FractureTemplate
    )[0]
    assert fracture_template_updated.azimuth_angle == 23.0
    # assert fracture_template_updated.beta_factor_type == "FractureBetaFactor"
    assert fracture_template_updated.conductivity_factor == 12.5
    assert fracture_template_updated.conductivity_type == "FiniteConductivity"
    assert fracture_template_updated.d_factor_scale_factor == 1.2
    assert fracture_template_updated.effective_permeability == 55
    assert fracture_template_updated.fracture_width == 0.5
    assert fracture_template_updated.fracture_width_type == "UserDefinedWidth"
    assert fracture_template_updated.gas_viscosity == 0.1
    assert fracture_template_updated.height_scale_factor == 4
    assert fracture_template_updated.inertial_coefficient == 0.7
    assert fracture_template_updated.non_darcy_flow_type == "Computed"
    assert fracture_template_updated.orientation == "Azimuth"
    assert fracture_template_updated.perforation_length == 5
    assert fracture_template_updated.permeability_type == "UserDefinedPermeability"
    assert fracture_template_updated.relative_gas_density == 0.1
    assert fracture_template_updated.relative_permeability == 0.2
    assert fracture_template_updated.user_defined_d_factor == 14
    assert fracture_template_updated.user_defined_perforation_length == True
    assert fracture_template_updated.user_description == "my frac name"
    assert fracture_template_updated.width_scale_factor == 7
