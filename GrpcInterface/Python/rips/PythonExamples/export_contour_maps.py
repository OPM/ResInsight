# Load ResInsight Processing Server Client Library
import rips
import tempfile
import pathlib

# Connect to ResInsight instance
resInsight = rips.Instance.find()

# Data will be written to temp
tmpdir = pathlib.Path(tempfile.gettempdir())

# Find all eclipse contour maps of the project
contour_maps = resInsight.project.descendants(rips.EclipseContourMap)
print("Number of eclipse contour maps:", len(contour_maps))

# Export the contour maps to a text file
for (index, contour_map) in enumerate(contour_maps):
    filename = "eclipse_contour_map" + str(index) + ".txt"
    filepath = tmpdir / filename
    print("Exporting to:", filepath)
    contour_map.export_to_text(str(filepath))

# The contour maps is also available for a Case
cases = resInsight.project.cases()
for case in cases:
    contour_maps = case.descendants(rips.GeoMechContourMap)
    # Export the contour maps to a text file
    for (index, contour_map) in enumerate(contour_maps):
        filename = "geomech_contour_map" + str(index) + ".txt"
        filepath = tmpdir / filename
        print("Exporting to:", filepath)
        contour_map.export_to_text(str(filepath))
