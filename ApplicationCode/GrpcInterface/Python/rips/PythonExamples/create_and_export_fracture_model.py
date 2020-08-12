# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
project = resinsight.project

# Find a well
well_path = project.well_path_by_name("Well-1")
print("well path:", well_path)
fracture_model_collection = project.descendants(rips.FractureModelCollection)[0]

# Create fracture model at a give measured depth
measured_depth = 4100.0
elastic_properties_file_path = "/home/resinsight/stimplan/complete_dataset_2020-06-22/Elastic_Template_CSV_file-with-biot.csv"
fracture_model = fracture_model_collection.new_fracture_model(well_path=well_path, measured_depth=measured_depth, elastic_properties_file_path=elastic_properties_file_path)





cases = resinsight.project.cases()
case = cases[0]

# Use the last time step
time_steps = case.time_steps()
time_step = time_steps[len(time_steps) - 1]


fracture_model_plot_collection = project.descendants(rips.FractureModelPlotCollection)[0]
fracture_model_plot = fracture_model_plot_collection.new_fracture_model_plot(eclipse_case=case, fracture_model=fracture_model, time_step=time_step)

