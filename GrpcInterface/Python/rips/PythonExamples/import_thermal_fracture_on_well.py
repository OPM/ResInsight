# Load ResInsight Processing Server Client Library
import rips
import tempfile
from os.path import expanduser
from pathlib import Path

# Connect to ResInsight instance
resinsight = rips.Instance.find()
project = resinsight.project

# Look for input files in the home directory of the user
home_dir = expanduser("~")
fracture_file_path = (Path(home_dir) / "fracture.csv").as_posix()
print("Thermal fracture file path:", fracture_file_path)

# Find a case
cases = resinsight.project.cases()
case = cases[0]

# Create thermal template
fmt_collection = project.descendants(rips.FractureTemplateCollection)[0]
fracture_template = fmt_collection.append_thermal_fracture_template(
    file_path=fracture_file_path
)

well_name = "F-1 H"

# Find a well
well_path = project.well_path_by_name(well_name)
print("Well path:", well_path.name)

# Create fracture and place it using data from the fracture template
fracture = well_path.add_thermal_fracture(
    fracture_template=fracture_template,
    place_using_template_data=True,
)


time_steps = fracture_template.time_steps().values
for time_step_index, time_stamp in enumerate(time_steps):
    print("Time step #{}: {}".format(time_step_index, time_stamp))
    fracture_template.active_time_step_index = time_step_index
    fracture_template.conductivity_result_name = "Conductivity"
    fracture_template.update()
