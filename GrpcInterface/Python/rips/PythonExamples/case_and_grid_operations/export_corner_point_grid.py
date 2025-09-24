######################################################################
# This script demonstrates how to export corner point grid data from
# ResInsight cases and recreate new grids from the exported data.
#
# This is the inverse operation of create_corner_point_grid.py - it
# extracts COORD, ZCORN, and ACTNUM arrays from existing Eclipse cases.
######################################################################

import rips


def validate_grid_dimensions(coord_array, zcorn_array, actnum_array, nx, ny, nz):
    """
    Validate that the exported arrays match the expected dimensions.

    Args:
        coord_array: COORD array (coordinate lines)
        zcorn_array: ZCORN array (corner depths)
        actnum_array: ACTNUM array (active cell flags)
        nx, ny, nz: Grid dimensions

    Returns:
        bool: True if arrays match expected dimensions
    """
    total_cells = nx * ny * nz
    expected_coord_size = (nx + 1) * (ny + 1) * 6  # 6 values per coordinate line
    expected_zcorn_size = nx * ny * nz * 8  # 8 corner depths per cell

    return (
        len(actnum_array) == total_cells
        and len(coord_array) == expected_coord_size
        and len(zcorn_array) == expected_zcorn_size
    )


def main():
    # Connect to ResInsight
    resinsight = rips.Instance.find()
    if resinsight is None:
        print("Starting ResInsight...")
        resinsight = rips.Instance.launch(console=True)

    project = resinsight.project
    if not project.cases():
        print("No cases in current project")
        return

    original_case = project.cases()[0]
    if original_case is None:
        print("Failed to load case")
        return

    print(f"Loaded case: {original_case.name}")

    # Get basic case information
    cell_count = original_case.cell_count()
    print("Case information:")
    print(f"  Total cells: {cell_count.reservoir_cell_count}")
    print(f"  Active cells: {cell_count.active_cell_count}")

    # Export corner point grid data
    print("\nExporting corner point grid data...")
    zcorn, coord, actnum, nx, ny, nz = original_case.export_corner_point_grid()

    print("Exported arrays:")
    print(f"  ZCORN: {len(zcorn):,} values (corner depths)")
    print(f"  COORD: {len(coord):,} values (coordinate lines)")
    print(f"  ACTNUM: {len(actnum):,} values (active cell flags)")

    # Grid dimensions are now returned directly from the export method!
    print(f"\nGrid dimensions from export: {nx} x {ny} x {nz} = {nx * ny * nz:,} cells")

    # Validate that the arrays match the expected dimensions
    if validate_grid_dimensions(coord, zcorn, actnum, nx, ny, nz):
        print("✓ Array sizes match expected dimensions perfectly")
    else:
        print("⚠ Array sizes don't match expected dimensions - this shouldn't happen")
        return

    # Create a new corner point grid from the exported data
    print("\nCreating new corner point grid from exported data...")
    recreated_name = f"{original_case.name}_Recreated"
    recreated_case = project.create_corner_point_grid(
        recreated_name, nx, ny, nz, coord, zcorn, actnum
    )

    if recreated_case is None:
        print("Failed to create recreated case")
        return

    print(f"Created recreated case: {recreated_case.name}")

    # Compare the two grids
    print("\nComparing original and recreated grids:")

    orig_cell_count = original_case.cell_count()
    recreated_cell_count = recreated_case.cell_count()

    print("Original grid:")
    print(f"  Total cells: {orig_cell_count.reservoir_cell_count:,}")
    print(f"  Active cells: {orig_cell_count.active_cell_count:,}")

    print("Recreated grid:")
    print(f"  Total cells: {recreated_cell_count.reservoir_cell_count:,}")
    print(f"  Active cells: {recreated_cell_count.active_cell_count:,}")

    # Get coordinate ranges for comparison
    print("\nCoordinate ranges:")

    # Sample some coordinates for comparison
    orig_bbox = original_case.reservoir_boundingbox()
    recreated_bbox = recreated_case.reservoir_boundingbox()

    print("Original bounding box:")
    print(f"  X: {orig_bbox.min_x:.2f} to {orig_bbox.max_x:.2f}")
    print(f"  Y: {orig_bbox.min_y:.2f} to {orig_bbox.max_y:.2f}")
    print(f"  Z: {orig_bbox.min_z:.2f} to {orig_bbox.max_z:.2f}")

    print("Recreated bounding box:")
    print(f"  X: {recreated_bbox.min_x:.2f} to {recreated_bbox.max_x:.2f}")
    print(f"  Y: {recreated_bbox.min_y:.2f} to {recreated_bbox.max_y:.2f}")
    print(f"  Z: {recreated_bbox.min_z:.2f} to {recreated_bbox.max_z:.2f}")


if __name__ == "__main__":
    main()
