######################################################################
# This example imports a polygon from a .pol file
######################################################################
import os
import tempfile

import rips

resinsight = rips.Instance.find()

polygon_collection = resinsight.project.descendants(rips.PolygonCollection)[0]

with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    # A .pol file has one "x y depth" point per line. "999.0 999.0 999.0" ends a polygon.
    file_name = os.path.join(tmpdirname, "example_polygon.pol")
    with open(file_name, "w") as f:
        f.write("0.0 0.0 1000.0\n")
        f.write("1000.0 0.0 1000.0\n")
        f.write("1000.0 1000.0 1000.0\n")
        f.write("0.0 1000.0 1000.0\n")
        f.write("999.0 999.0 999.0\n")

    # The polygons are imported into a new folder named after the file.
    # The first polygon in the file is returned.
    polygon = polygon_collection.import_polygon_file(
        file_name=file_name,
        on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME,
    )

    print(f"Imported polygon: {polygon.name}")
    for coordinate in polygon.coordinates:
        print(coordinate)
