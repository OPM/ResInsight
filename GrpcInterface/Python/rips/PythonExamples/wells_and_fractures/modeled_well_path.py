# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Create a modeled well path and add well path targets
# The coordinates are based on the Norne case

well_path_coll = resinsight.project.well_path_collection()
well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
well_path.name = "Test Well-1"
well_path.update()

geometry = well_path.well_path_geometry()

reference_point = geometry.reference_point
reference_point[0] = 457196
reference_point[1] = 7322270
reference_point[2] = 2742
geometry.update()  # Commit updates back to ResInsight

# Create the first well target at the reference point
coord = [0, 0, 0]
geometry.append_well_target(coord)

# Append new well targets relative the the reference point
coord = [454.28, 250, -10]
target = geometry.append_well_target(coord)

coord = [1054.28, 250, -50]
target = geometry.append_well_target(coord)

perforation = well_path.append_perforation_interval(3300, 3350, 0.2, 0.76)

# Make some changes to the AICD parameters of the valve template
valve_templates = resinsight.project.valve_templates()
aicd_template = valve_templates.valve_definitions()[0]
aicd_parameters = aicd_template.aicd_parameters()
aicd_parameters.max_flow_rate = 899.43
aicd_parameters.density_calibration_fluid = 45.5
aicd_parameters.update()

# Add a valve to the perforation interval
valve = perforation.add_valve(
    template=aicd_template, start_md=3310, end_md=3340, valve_count=2
)


# Update skin factor of the perforation
perforation_coll = well_path.completions().perforations()
perforation = perforation_coll.perforations()[0]
new_skin_factor = 0.9
print(
    "Changing perforation skin factor from {} to {}.".format(
        perforation.skin_factor, new_skin_factor
    )
)
perforation.skin_factor = new_skin_factor
perforation.update()

# Optionally update the completion settings
completions_settings = well_path.completion_settings()
completions_settings.msw_roughness = 12.34
completions_settings.msw_liner_diameter = 0.2222
completions_settings.well_name_for_export = "file name"
completions_settings.group_name_for_export = "msj"
completions_settings.well_type_for_export = "GAS"
completions_settings.update()  # Commit updates back to ResInsight

# Add diameter roughness intervals for interval-specific configuration
interval1 = completions_settings.add_diameter_roughness_interval(
    start_md=3200, end_md=3300, diameter=0.18, roughness_factor=1.5e-5
)
interval2 = completions_settings.add_diameter_roughness_interval(
    start_md=3300, end_md=3400, diameter=0.16, roughness_factor=2.0e-5
)
print(
    f"Added diameter roughness intervals: {interval1.start_md}-{interval1.end_md}m and {interval2.start_md}-{interval2.end_md}m"
)

# Optionally update the MSW settings
msw_settings = well_path.msw_settings()
msw_settings.custom_values_for_lateral = False
msw_settings.enforce_max_segment_length = False
msw_settings.liner_diameter = 0.152
msw_settings.max_segment_length = 200
msw_settings.pressure_drop = "HF-"
msw_settings.reference_md_type = "GridEntryPoint"
msw_settings.roughness_factor = 1e-05
msw_settings.user_defined_reference_md = 0
msw_settings.update()

# Optionally update the Perforation Non-Darcy settings
non_darcy_parameters = perforation_coll.non_darcy_parameters()
non_darcy_parameters.non_darcy_flow_type = "UserDefined"
non_darcy_parameters.user_defined_d_factor = 1.2345
non_darcy_parameters.update()

# export completions
cases = resinsight.project.cases()

for case in cases:
    print("Case name: ", case.name)
    print("Case id: ", case.id)

    case.export_well_path_completions(
        time_step=0,
        well_path_names=["Test Well-1 Y1"],
        file_split="UNIFIED_FILE",
        include_perforations=True,
        # Replace the following with a valid path
        custom_file_name="f:/scratch/2023-11-02/myfile.myext",
    )
