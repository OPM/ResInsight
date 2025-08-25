"""
Example: Import Well Log Data to Well Paths

This example demonstrates how to import well log data from Python arrays
into ResInsight well paths using the new well log import API.
"""

import rips


def create_example_well_log_data():
    """Create example well log data"""
    # Simulated gamma ray log data
    gamma_ray = [75.2, 82.1, 78.5, 80.3, 85.7, 88.2, 90.1, 87.3, 84.6, 82.8]

    # Simulated porosity log data
    porosity = [0.12, 0.15, 0.18, 0.22, 0.25, 0.28, 0.24, 0.20, 0.16, 0.14]

    # Simulated permeability log data
    permeability = [15.5, 22.3, 35.8, 48.2, 65.1, 78.4, 52.9, 38.7, 25.6, 18.9]

    return {"Gamma_Ray": gamma_ray, "Porosity": porosity, "Permeability": permeability}


def main():
    # Connect to ResInsight
    resinsight = rips.Instance()
    project = resinsight.project

    # Create a well path from coordinates
    coordinates = [
        [458000, 5934000, 1000],  # X, Y, Z coordinates
        [458100, 5934100, 1500],
        [458200, 5934200, 2000],
        [458300, 5934300, 2500],
        [458400, 5934400, 3000],
    ]

    well_path = project.well_path_collection.import_fixed_trajectory_well_path(
        name="ExampleWell", coordinates=coordinates
    )

    print(f"Created well path: {well_path.name}")

    # Get example well log data
    log_data = create_example_well_log_data()

    # Import each well log channel
    for log_name, values in log_data.items():
        well_log = well_path.add_well_log(name=log_name, values=values)
        print(f"Added well log: {well_log.name} with {len(values)} data points")

    # Verify all logs were added
    well_logs = well_path.well_logs()
    print(f"\\nWell path now has {len(well_logs)} well logs:")
    for log in well_logs:
        print(f"  - {log.name}")

    print("\\nWell log import completed successfully!")


if __name__ == "__main__":
    main()
