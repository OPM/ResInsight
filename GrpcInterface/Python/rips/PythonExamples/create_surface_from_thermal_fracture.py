#!/usr/bin/env python
# coding: utf-8

import rips
import tempfile
from os.path import expanduser
from pathlib import Path
import numpy as np
import pyvista as pv


def generate_surface_from_file(path):
    point_cloud_data = np.loadtxt(path, delimiter=" ", skiprows=1)

    # Get [x, y, z] components in separate matrix
    num_rows = point_cloud_data.shape[0]
    xyz = point_cloud_data[0:num_rows, 0:3]

    # Generate surface
    cloud = pv.PolyData(xyz)
    surf = cloud.delaunay_2d()

    # Read properties names from header data
    f = open(path)
    header = f.readline()
    properties = header.strip().split(" ")

    return (surf, point_cloud_data, properties)


def export_surface_as_ts_file(surf, point_cloud, properties, path):
    # open text file
    text_file = open(path, "w")

    # write GOCAD header
    top_header = """GOCAD TSurf 1
HEADER {
name:MF_027_SU
}
"""

    properties_str = "PROPERTIES " + " ".join(properties)

    bottom_header = """
GOCAD_ORIGINAL_COORDINATE_SYSTEM
NAME Default
AXIS_NAME "X" "Y" "Z"
AXIS_UNIT "m" "m" "m"
ZPOSITIVE Depth
END_ORIGINAL_COORDINATE_SYSTEM
TFACE
"""

    text_file.write(top_header)
    text_file.write(properties_str)
    text_file.write(bottom_header)

    i = 1
    (num_rows, num_props) = point_cloud.shape
    for row in range(0, num_rows):
        x = point_cloud[row, 0]
        y = point_cloud[row, 1]
        z = point_cloud[row, 2]
        txt = "PVRTX {} {:.3f} {:.3f} {:.3f} ".format(i, x, y, z)
        for property_index in range(0, num_props):
            txt += "{:.3f} ".format(point_cloud[row, property_index])
        txt += "\n"
        text_file.write(txt)
        i += 1

    mysurface = surf.faces.reshape(-1, 4)
    for p in mysurface:
        txt = "TRGL {} {} {}\n".format(p[1] + 1, p[2] + 1, p[3] + 1)
        text_file.write(txt)

    text_file.write("END")
    text_file.close()


# Connect to ResInsight instance
resinsight = rips.Instance.find()
project = resinsight.project


fractures = project.descendants(rips.ThermalFractureTemplate)
print("Number of thermal fractures: ", len(fractures))


temp_folder = tempfile.gettempdir()

# Write results to a suitable directory
home_dir = expanduser("~")

for fracture in fractures:
    fracture_name = fracture.user_description

    # Create the ouput directory
    output_directory = (
        Path(home_dir) / "thermal_fracture_surfaces" / "{}".format(fracture_name)
    )

    output_directory.mkdir(parents=True, exist_ok=True)
    print("Creating result directory: ", output_directory.as_posix())

    time_steps = fracture.time_steps().values
    for time_step_index, time_step in enumerate(time_steps):
        print(
            "Generating surface for time step #{}: {}".format(
                time_step_index, time_step
            )
        )
        temp_file_path = Path(temp_folder) / "output.xyz"
        fracture.export_to_file(
            file_path=temp_file_path.as_posix(), time_step=time_step_index
        )

        # Reconstruct a surface from the exported values file
        (surface, point_cloud, properties) = generate_surface_from_file(
            temp_file_path.as_posix()
        )

        # Export surface ts file from the surface data
        output_file_path = output_directory / "time_step_{:03d}.ts".format(
            time_step_index
        )
        export_surface_as_ts_file(
            surface, point_cloud, properties, output_file_path.as_posix()
        )
        print(
            "Wrote surface for time step #{} to {}".format(
                time_step, output_file_path.as_posix()
            )
        )
