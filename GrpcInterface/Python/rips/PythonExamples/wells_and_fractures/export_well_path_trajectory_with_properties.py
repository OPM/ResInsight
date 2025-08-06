###################################################################################
# This example will connect to ResInsight, extract trajectory properties for
# each wells in the project, and extract some properties for each points on the
# trajectory.
#
###################################################################################

import rips


def print_dictionary(title, data):
    # Dictionary of lists of floats.
    keys = list(data.keys())

    print(title)
    for i, properties in enumerate(zip(*data.values()), 1):
        prop_pairs = [
            f"{key.replace('coordinate_', '')}={prop:.3f}"
            for key, prop in zip(keys, properties)
        ]
        print(f"Point {i}: {', '.join(prop_pairs)}")


# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is not None:
    # Get a list of all wells
    wells = resinsight.project.well_paths()

    # Find the first case
    cases = resinsight.project.cases()
    c = cases[0] if len(cases) else None

    for well in wells:
        result = well.trajectory_properties(resampling_interval=10.0)

        if c:
            # Convert the result data into points
            positions = [
                list(coord)
                for coord in zip(
                    result["coordinate_x"],
                    result["coordinate_y"],
                    result["coordinate_z"],
                )
            ]

            # Extract some properties
            properties = [
                ("DYNAMIC_NATIVE", "PRESSURE", 0),
                ("STATIC_NATIVE", "FAULTDIST", 0),
            ]

            for property_type, property_name, time_step in properties:
                porosity_model = "MATRIX_MODEL"
                result[property_name] = c.grid_property_for_positions(
                    positions, property_type, property_name, time_step, porosity_model
                )

            title = "Well name: " + well.name
            print_dictionary(title, result)
