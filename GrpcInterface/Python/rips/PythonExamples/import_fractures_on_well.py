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
stim_plan_file_path = (Path(home_dir) / "contour.xml").as_posix()
print("StimPlan contour file path:", stim_plan_file_path)

# Find a case
cases = resinsight.project.cases()
case = cases[0]

# Create stim plan template
fmt_collection = project.descendants(rips.FractureTemplateCollection)[0]
fracture_template = fmt_collection.append_fracture_template(
    file_path=stim_plan_file_path
)

well_name = "B-2 H"

# Find a well
well_path = project.well_path_by_name(well_name)
print("well path:", well_path.name)

# Place fracture at given depths
measured_depths = [3200.0, 3400.0, 3600.0]
for measured_depth in measured_depths:

    print("Placing fracture at {} depth (MD)".format(measured_depth))
    # Create stim plan  at a give measured depth
    fracture = well_path.add_fracture(
        measured_depth=measured_depth,
        stim_plan_fracture_template=fracture_template,
        align_dip=True,
        eclipse_case=case,
    )

# Update the orientation of the fracture
# Call update() to propagate changes from the Python object back to ResInsight
fracture_template.orientation = "Azimuth"
fracture_template.azimuth_angle = 60.0
fracture_template.update()
