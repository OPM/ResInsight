# Load ResInsight Processing Server Client Library
import math, time
import rips

resinsight = rips.Instance.find()

# The coordinates in this example is based on the Drogon test case from Equinor
file_path = "e:/models/from_equinor_sftp/drogon-real0-iter3/DROGON-0.EGRID"

case = resinsight.project.load_case(file_path)

view = case.create_view()
view.set_time_step(2)

intersection_coll = resinsight.project.descendants(rips.IntersectionCollection)[0]

# Add a CurveIntersection and set coordinates for the polyline
intersection = intersection_coll.add_new_object(rips.CurveIntersection)
intersection.points = [
    [45854, 595757, 1500],
    [46493, 534259.1, 1500],
    [46598, 590044.1, 1500],
]
intersection.update()

# Add a new modeled well path
well_path_coll = resinsight.project.descendants(rips.WellPathCollection)[0]
well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
well_path.name = "Test Well-1"
well_path.update()

# Set reference coordinate
geometry = well_path.well_path_geometry()
reference_point = geometry.reference_point
reference_point[0] = 458580
reference_point[1] = 5935514
reference_point[2] = 1742
geometry.update()  # Commit updates back to ResInsight

# Create the first well target at the reference point
coord = [0, 0, 0]
geometry.append_well_target(coord)

# Append well target with fixed azimuth
coord = [2229.10, -833.74, -74.70]
target = geometry.append_well_target(
    coord, use_fixed_azimuth=True, fixed_azimuth_value=45.1
)

# Append well target with fixed inclination
coord = [3403.15, -1938.61, -80.93]
target = geometry.append_well_target(
    coord, use_fixed_inclination=True, fixed_inclination_value=115.2
)

coord = [4577.21, -3043.47, -87.15]
target = geometry.append_well_target(coord)
geometry.update()

# Read out estimated dogleg and azimuth/inclination for well targets
for w in geometry.well_path_targets():
    print(
        "DL1:{}   DL2:{}   Azi: {}   Incl: {}".format(
            w.estimated_dogleg1,
            w.estimated_dogleg2,
            w.estimated_azimuth,
            w.estimated_inclination,
        )
    )

# Add a curve intersection based on the modeled well path
well_path_intersection = intersection_coll.add_new_object(rips.CurveIntersection)
well_path_intersection.type = "CS_WELL_PATH"
well_path_intersection.well_path = well_path
well_path_intersection.update()
