"""
Example demonstrating how to create well paths from XYZ coordinates
using the import_fixed_trajectory_well_path API.
"""

import sys
import os
import rips


def create_norne_well():
    coordinates = [
        [457121.858, 7322122.992, -371.7],
        [457121.820, 7322122.956, -404.999],
        [457121.767, 7322122.946, -434.999],
        [457123.250, 7322120.048, -644.946],
        [457132.573, 7322112.750, -914.534],
        [457132.853, 7322111.663, -1184.492],
        [457141.541, 7322111.158, -1484.359],
        [457147.535, 7322109.991, -1694.269],
        [457152.835, 7322107.659, -1934.198],
        [457142.201, 7322088.313, -2182.14],
        [457108.159, 7322025.317, -2335.345],
        [457077.300, 7321905.985, -2464.557],
        [457100.629, 7321698.916, -2584.388],
        [457226.480, 7321457.656, -2629.045],
        [457374.455, 7321253.227, -2633.377],
        [457499.332, 7321103.741, -2630.816],
        [457581.202, 7321019.105, -2630.42],
        [457661.101, 7320934.536, -2630.212],
        [457727.784, 7320862.819, -2632.240],
    ]

    return coordinates


resinsight = rips.Instance.find()

# Get the well path collection
well_path_coll = resinsight.project.well_path_collection()

# Create different types of well paths
well_paths_data = [
    ("B 2-H", create_norne_well()),
]

created_wells = []

for name, coordinates in well_paths_data:
    print(f"\nCreating well path: {name}")
    print(f"  Number of coordinate points: {len(coordinates)}")
    print(
        f"  Start: [{coordinates[0][0]:.1f}, {coordinates[0][1]:.1f}, {coordinates[0][2]:.1f}]"
    )
    print(
        f"  End: [{coordinates[-1][0]:.1f}, {coordinates[-1][1]:.1f}, {coordinates[-1][2]:.1f}]"
    )

    # Create the well path using the new API
    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name=name, coordinates=coordinates
    )

    created_wells.append(well_path)
