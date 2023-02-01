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
fracture_template.user_defined_perforation_length = True
fracture_template.conductivity_type = "InfiniteConductivity"
fracture_template.perforation_length = 12.3
fracture_template.update()

# Scale the template
fracture_template.set_scale_factors(
    half_length=2.0, height=2.0, d_factor=1.1, conductivity=1.2
)

# Output scale factors for all fracture templates
fmt_collection = project.descendants(rips.FractureTemplate)
for fracture_template in fmt_collection:
    print(
        "Fracture: '{}' Scale factors: Height={} Half Length={} D Factor={} Conductivity={}".format(
            fracture_template.user_description,
            fracture_template.height_scale_factor,
            fracture_template.width_scale_factor,
            fracture_template.d_factor_scale_factor,
            fracture_template.conductivity_factor,
        )
    )
