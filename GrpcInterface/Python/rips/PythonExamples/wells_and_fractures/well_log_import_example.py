"""
Example: Import Well Log Data to Well Paths

This example demonstrates how to import well log data with measured depth
and multiple channels from Python arrays into ResInsight well paths using
the new well log import API.
"""

import rips


def create_example_well_log_data():
    """Create example well log data with measured depth and multiple channels"""
    # Measured depth values in meters
    measured_depth = [1000, 1200, 1400, 1600, 1800, 2000, 2200, 2400, 2600, 2800]

    # Multiple channel data corresponding to each measured depth
    channel_data = {
        "gamma_ray": [75.2, 82.1, 78.5, 80.3, 85.7, 88.2, 90.1, 87.3, 84.6, 82.8],
        "porosity": [0.12, 0.15, 0.18, 0.22, 0.25, 0.28, 0.24, 0.20, 0.16, 0.14],
        "permeability": [15.5, 22.3, 35.8, 48.2, 65.1, 78.4, 52.9, 38.7, 25.6, 18.9],
        "resistivity": [2.1, 1.8, 3.2, 2.5, 1.9, 1.4, 2.8, 3.1, 2.7, 2.3],
    }

    return measured_depth, channel_data


def main():
    # Connect to ResInsight
    resinsight = rips.Instance.find()
    project = resinsight.project

    # Create a well path from coordinates
    coordinates = [
        [458000, 5934000, -1000],  # X, Y, Z coordinates
        [458100, 5934100, -1500],
        [458200, 5934200, -2000],
        [458300, 5934300, -2500],
        [458400, 5934400, -3000],
    ]

    well_path = project.well_path_collection().import_well_path_from_points(
        name="ExampleWell", coordinates=coordinates
    )

    print(f"Created well path: {well_path.name}")

    # Get example well log data
    measured_depth, channel_data = create_example_well_log_data()

    # Import well log with multiple channels using the new API
    well_log = well_path.add_well_log(
        name="ComprehensiveWellLog",
        measured_depth=measured_depth,
        channel_data=channel_data,
    )

    print(f"Added well log: {well_log.name}")
    print(
        f"  - Measured depth range: {min(measured_depth):.1f} - {max(measured_depth):.1f} m"
    )
    print(f"  - Number of data points: {len(measured_depth)}")
    print(f"  - Channels imported: {', '.join(channel_data.keys())}")

    # You can also import additional well logs with different data
    # For example, a well log with different measured depth intervals
    additional_measured_depth = [1100, 1300, 1500, 1700, 1900, 2100, 2300, 2500, 2700]
    additional_channel_data = {
        "caliper": [8.5, 8.2, 8.7, 8.3, 8.1, 8.4, 8.6, 8.2, 8.3],
        "neutron": [0.18, 0.22, 0.25, 0.28, 0.31, 0.29, 0.26, 0.23, 0.20],
    }

    additional_well_log = well_path.add_well_log(
        name="AdditionalWellLog",
        measured_depth=additional_measured_depth,
        channel_data=additional_channel_data,
    )

    print(f"Added additional well log: {additional_well_log.name}")
    print(
        f"  - MD range: {min(additional_measured_depth):.1f} - {max(additional_measured_depth):.1f} m"
    )
    print(f"  - Channels imported: {', '.join(additional_channel_data.keys())}")


if __name__ == "__main__":
    main()
