# Load ResInsight Processing Server Client Library
import rips
import tempfile
from os.path import expanduser
from pathlib import Path

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
project = resinsight.project

# Create fracture model template
home_dir = expanduser("~")
elastic_properties_file_path = (Path(home_dir) / "elastic_properties.csv").as_posix()
print("Elastic properties file path:", elastic_properties_file_path)

facies_properties_file_path = (Path(home_dir) / "facies_id.roff").as_posix()
print("Facies properties file path:", facies_properties_file_path)

fmt_collection = project.descendants(rips.FractureModelTemplateCollection)[0]
fracture_model_template = fmt_collection.new_fracture_model_template(elastic_properties_file_path=elastic_properties_file_path,
                                                                     facies_properties_file_path=facies_properties_file_path)
fracture_model_template.overburden_formation = "Garn"
fracture_model_template.overburden_facies = "Shale"
fracture_model_template.underburden_formation = "Garn"
fracture_model_template.underburden_facies = "Shale"
fracture_model_template.overburden_height = 68
fracture_model_template.update()
print("Overburden: ", fracture_model_template.overburden_formation)


# Set eclipse result for facies definition
eclipse_result = fracture_model_template.facies_properties().facies_definition()
eclipse_result.result_type = "INPUT_PROPERTY"
eclipse_result.result_variable = "OPERNUM_1"
eclipse_result.update()

# Set eclipse result for non-net layers
non_net_layers = fracture_model_template.non_net_layers()
non_net_layers_result = non_net_layers.facies_definition()
non_net_layers_result.result_type = "STATIC_NATIVE"
non_net_layers_result.result_variable = "NTG"
non_net_layers_result.update()
non_net_layers.formation = "Not"
non_net_layers.facies = "Shale"
non_net_layers.update()


# Add some scaling factors
elastic_properties = fracture_model_template.elastic_properties()
elastic_properties.add_property_scaling(formation="Garn", facies="Calcite", property="YOUNGS_MODULUS", scale=1.44)


# Find a well
well_path = project.well_path_by_name("B-2 H")
print("well path:", well_path)
fracture_model_collection = project.descendants(rips.FractureModelCollection)[0]

# Find a case
cases = resinsight.project.cases()
case = cases[0]

# Use the last time step
time_steps = case.time_steps()
time_step = time_steps[len(time_steps) - 1]

# Create fracture model at a give measured depth
measured_depth = 3200.0
fracture_model = fracture_model_collection.new_fracture_model(eclipse_case=case,
                                                              time_step=time_step,
                                                              well_path=well_path,
                                                              measured_depth=measured_depth,
                                                              fracture_model_template=fracture_model_template)

export_folder = tempfile.gettempdir()

print("Exporting fracture model to: ", export_folder)
fracture_model.export_to_file(directory_path=export_folder)

# Create a fracture mode plot
fracture_model_plot_collection = project.descendants(rips.FractureModelPlotCollection)[0]
fracture_model_plot = fracture_model_plot_collection.new_fracture_model_plot(fracture_model=fracture_model)

print("Exporting fracture model plot to: ", export_folder)
fracture_model_plot.export_snapshot(export_folder=export_folder)

print("Setting measured depth and perforation length.")
fracture_model.measured_depth = 3300.0
fracture_model.perforation_length = 123.445
fracture_model.update()
