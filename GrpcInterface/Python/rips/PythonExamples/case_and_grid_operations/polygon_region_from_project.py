####################################################################################
# This example demonstrates how to use existing polygons already defined in the
# ResInsight project to create grid cell filters and assign a unique integer
# region value to every active cell:
# 1. Retrieve polygons from the project's PolygonCollection
# 2. For each active cell, determine which polygon region it belongs to
# 3. Create a Generated result that stores the unique integer region index
####################################################################################

import rips


def point_in_polygon_2d(px, py, polygon_xy):
    """Check if point (px, py) is inside a 2D polygon using the ray casting algorithm.

    Arguments:
        px (float): X coordinate of the point
        py (float): Y coordinate of the point
        polygon_xy (list): List of [x, y] pairs defining the polygon vertices

    Returns:
        bool: True if the point is inside the polygon, False otherwise
    """
    n = len(polygon_xy)
    inside = False
    j = n - 1
    for i in range(n):
        xi, yi = polygon_xy[i][0], polygon_xy[i][1]
        xj, yj = polygon_xy[j][0], polygon_xy[j][1]
        if ((yi > py) != (yj > py)) and (px < (xj - xi) * (py - yi) / (yj - yi) + xi):
            inside = not inside
        j = i
    return inside


resinsight = rips.Instance.find()
if resinsight is None:
    print("No ResInsight instance found")
    exit()

cases = resinsight.project.cases()
if not cases:
    print("No cases found in the project")
    exit()

case = cases[0]
print(f"Using case: {case.name}")

# Step 1: Retrieve polygons from the project's PolygonCollection.
# These polygons must already exist in the project before running this script.
# Use the companion example polygon_grid_region.py to create example polygons,
# or define your own interactively in ResInsight.
polygon_collection = resinsight.project.descendants(rips.PolygonCollection)[0]
polygons = polygon_collection.polygons()

if not polygons:
    print(
        "No polygons found in the project. "
        "Please create polygons in ResInsight before running this script."
    )
    exit()

print(f"Found {len(polygons)} polygon(s) in the project:")
for i, p in enumerate(polygons):
    print(f"  [{i + 1}] {p.name}  ({len(p.coordinates)} vertices)")

# Step 2: For each active cell, determine which polygon region it belongs to
# by checking whether the cell center (X, Y) falls inside the polygon.
# The polygon coordinates contain Z (depth) values which are ignored for the
# 2D containment test.
print("Computing cell-to-region assignments...")
cell_centers = case.active_cell_centers()

# Initialize all cells with value 0 (no region assigned)
region_values = [0.0] * len(cell_centers)

for cell_idx, cell_center in enumerate(cell_centers):
    for polygon_idx, polygon in enumerate(polygons):
        polygon_xy = [[c[0], c[1]] for c in polygon.coordinates]
        if point_in_polygon_2d(cell_center.x, cell_center.y, polygon_xy):
            # Assign 1-based region index so each region has a unique integer value
            region_values[cell_idx] = float(polygon_idx + 1)
            break  # Assign cell to the first matching polygon

# Step 3: Create a Generated result that stores the unique integer region value
# for every active cell. Cells not covered by any polygon receive value 0.
property_name = "POLYGON_REGION"
case.set_active_cell_property(region_values, "GENERATED", property_name, 0)

print(f"Generated property '{property_name}' created successfully")
for i, polygon in enumerate(polygons):
    count = sum(1 for v in region_values if v == float(i + 1))
    print(f"  Region {i + 1} (polygon '{polygon.name}'): {count} cells")

unassigned = sum(1 for v in region_values if v == 0.0)
print(f"  Unassigned (no polygon): {unassigned} cells")

# Apply the generated result in the view to visualize the regions
view = case.views()[0] if case.views() else case.create_view()
view.apply_cell_result(result_type="GENERATED", result_variable=property_name)
print(f"Applied '{property_name}' cell result to view")
