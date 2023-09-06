# Load ResInsight Processing Server Client Library
import rips
import tempfile
from os.path import expanduser
from pathlib import Path

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
project = resinsight.project

# Look for input files in the home directory of the user
home_dir = expanduser("~")
elastic_properties_file_path = (Path(home_dir) / "elastic_properties.csv").as_posix()
print("Elastic properties file path:", elastic_properties_file_path)

facies_properties_file_path = (Path(home_dir) / "facies_id.roff").as_posix()
print("Facies properties file path:", facies_properties_file_path)

# Find a case
cases = resinsight.project.cases()
case = cases[1]

# Use the last time step
time_steps = case.time_steps()
time_step = time_steps[len(time_steps) - 1]


# Create stim plan model template
fmt_collection = project.descendants(rips.StimPlanModelTemplateCollection)[0]
stim_plan_model_template = fmt_collection.append_stim_plan_model_template(
    eclipse_case=case,
    time_step=time_step,
    elastic_properties_file_path=elastic_properties_file_path,
    facies_properties_file_path=facies_properties_file_path,
)
stim_plan_model_template.overburden_formation = "Garn"
stim_plan_model_template.overburden_facies = "Shale"
stim_plan_model_template.underburden_formation = "Garn"
stim_plan_model_template.underburden_facies = "Shale"
stim_plan_model_template.overburden_height = 68
stim_plan_model_template.update()
print("Overburden: ", stim_plan_model_template.overburden_formation)


# Set eclipse result for facies definition
eclipse_result = stim_plan_model_template.facies_properties().facies_definition()
eclipse_result.result_type = "INPUT_PROPERTY"
eclipse_result.result_variable = "OPERNUM_1"
eclipse_result.update()

# Set eclipse result for non-net layers
non_net_layers = stim_plan_model_template.non_net_layers()
non_net_layers_result = non_net_layers.facies_definition()
non_net_layers_result.result_type = "STATIC_NATIVE"
non_net_layers_result.result_variable = "NTG"
non_net_layers_result.update()
non_net_layers.formation = "Not"
non_net_layers.facies = "Shale"
non_net_layers.update()

# Add some pressure table items
pressure_table = stim_plan_model_template.pressure_table()
pressure_table.add_pressure(depth=2800.0, initial_pressure=260.0, pressure=261.0)
pressure_table.add_pressure(depth=3000.0, initial_pressure=270.0, pressure=273.0)
pressure_table.add_pressure(depth=3400.0, initial_pressure=274.0, pressure=276.0)
pressure_table.add_pressure(depth=3800.0, initial_pressure=276.0, pressure=280.0)

print("Pressure table ({} items)".format(len(pressure_table.items())))
for item in pressure_table.items():
    print(
        "TDVMSL [m]: {} Initial Pressure: {} Pressure: {}".format(
            item.depth, item.initial_pressure, item.pressure
        )
    )

# Add some scaling factors
elastic_properties = stim_plan_model_template.elastic_properties()
elastic_properties.add_property_scaling(
    formation="Garn", facies="Calcite", property="YOUNGS_MODULUS", scale=1.44
)


well_name = "B-2 H"

# Find a well
well_path = project.well_path_by_name(well_name)
print("well path:", well_path)
stim_plan_model_collection = project.descendants(rips.StimPlanModelCollection)[0]


export_folder = tempfile.gettempdir()

stim_plan_models = []

# Create and export a StimPlan model for each depth
measured_depths = [3200.0, 3400.0, 3600.0]
for measured_depth in measured_depths:
    # Create stim plan model at a give measured depth
    stim_plan_model = stim_plan_model_collection.append_stim_plan_model(
        well_path=well_path,
        measured_depth=measured_depth,
        stim_plan_model_template=stim_plan_model_template,
    )
    stim_plan_models.append(stim_plan_model)

    # Make the well name safer to use as a directory path
    well_name_part = well_name.replace(" ", "_")
    directory_path = Path(export_folder) / "{}_{}".format(
        well_name_part, int(measured_depth)
    )

    # Create the folder
    directory_path.mkdir(parents=True, exist_ok=True)

    print("Exporting fracture model to: ", directory_path)
    stim_plan_model.export_to_file(directory_path=directory_path.as_posix())

    # Create a fracture mode plot
    stim_plan_model_plot_collection = project.descendants(
        rips.StimPlanModelPlotCollection
    )[0]
    stim_plan_model_plot = stim_plan_model_plot_collection.append_stim_plan_model_plot(
        stim_plan_model=stim_plan_model
    )

    print("Exporting fracture model plot to: ", directory_path)
    stim_plan_model_plot.export_snapshot(export_folder=directory_path.as_posix())


print("Setting measured depth and perforation length.")
stim_plan_models[0].measured_depth = 3300.0
stim_plan_models[0].perforation_length = 123.445
stim_plan_models[0].update()
