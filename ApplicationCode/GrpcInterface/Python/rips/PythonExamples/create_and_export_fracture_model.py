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

measured_depth = 4100.0
fracture_model = fracture_model_collection.new_fracture_model(well_path=well_path, measured_depth=measured_depth)

elastic_properties = rips.ElasticProperties()
elastic_properties.file_path = "/home/resinsight/stimplan/complete_dataset_2020-06-22/Elastic_Template_CSV_file-with-biot.csv"
elastic_properties.update()

fracture_model.elastic_properties = elastic_properties



cases = resinsight.project.cases()
case = cases[0]

# Use the last time step
time_steps = case.time_steps()
time_step = time_steps[len(time_steps) - 1]


fracture_model_plot_collection = project.descendants(rips.FractureModelPlotCollection)[0]
fracture_model_plot = fracture_model_plot_collection.new_fracture_model_plot(eclipse_case=case, fracture_model=fracture_model, time_step=time_step)

