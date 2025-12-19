###################################################################################
# This example prints the distance to and the name of the fault closest to a point
###################################################################################

import rips

resinsight = rips.Instance.find()
if resinsight is None:
    exit(1)

cases = resinsight.project.cases()
if len(cases) == 0:
    exit(1)

case = cases[0]
print("Using case: " + case.name)

# random test point (positive Z for depth)
point_x = 5039.84
point_y = 6303.76
point_z = 4144.21

print("Looking for closest fault to point %f, %f, %f:" % (point_x, point_y, point_z))

faultname, distance, facename = case.distance_to_closest_fault(
    point_x, point_y, point_z
)

if facename == "":
    print("- No fault found!")
else:
    print(
        "- Distance to closest fault %s is %f, closest face direction is %s"
        % (faultname, distance, facename)
    )
