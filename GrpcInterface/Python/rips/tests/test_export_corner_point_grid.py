import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot


def test_export_corner_point_grid_basic(rips_instance, initialize_test):
    """Test the export_corner_point_grid method with a simple created grid"""

    # Create a simple test case
    nx, ny, nz = 2, 2, 2

    # Generate simple coordinate data
    coord = []
    for j in range(ny + 1):
        for i in range(nx + 1):
            x = i * 100.0
            y = j * 100.0
            # Top point
            coord.extend([x, y, 1000.0])
            # Bottom point
            coord.extend([x, y, 1100.0])

    # Generate simple ZCORN data (8 values per cell)
    zcorn = []
    for k in range(nz):
        for j in range(ny):
            for i in range(nx):
                base_depth = 1000.0 + k * 100.0
                # 8 corner depths for each cell
                zcorn.extend(
                    [
                        base_depth,
                        base_depth,
                        base_depth,
                        base_depth,
                        base_depth + 100.0,
                        base_depth + 100.0,
                        base_depth + 100.0,
                        base_depth + 100.0,
                    ]
                )

    # Generate ACTNUM (all cells active)
    actnum = [1] * (nx * ny * nz)

    # Create the grid case
    case = rips_instance.project.create_corner_point_grid(
        "TestGrid", nx, ny, nz, coord, zcorn, actnum
    )

    # Test our export function
    exported_zcorn, exported_coord, exported_actnum, export_nx, export_ny, export_nz = (
        case.export_corner_point_grid()
    )

    # Basic validation
    assert len(exported_zcorn) == len(zcorn)
    assert len(exported_coord) == len(coord)
    assert len(exported_actnum) == len(actnum)

    # Check that all cells are active
    active_count = sum(1 for x in exported_actnum if x > 0)
    assert active_count == sum(actnum)

    # Verify dimensions are returned correctly
    assert export_nx == nx, f"Dimension mismatch: nx={export_nx} vs expected {nx}"
    assert export_ny == ny, f"Dimension mismatch: ny={export_ny} vs expected {ny}"
    assert export_nz == nz, f"Dimension mismatch: nz={export_nz} vs expected {nz}"


def test_export_corner_point_grid_return_types(rips_instance, initialize_test):
    """Test that export_corner_point_grid returns the correct types"""

    # Create a minimal test case
    nx, ny, nz = 1, 1, 1
    coord = [0, 0, 1000, 0, 0, 1100] * 4  # 4 coordinate lines for 1x1 grid
    zcorn = [1000] * 8  # 8 corners
    actnum = [1]  # 1 active cell

    case = rips_instance.project.create_corner_point_grid(
        "MinimalGrid", nx, ny, nz, coord, zcorn, actnum
    )

    exported_zcorn, exported_coord, exported_actnum, export_nx, export_ny, export_nz = (
        case.export_corner_point_grid()
    )

    # Check return types
    assert isinstance(exported_zcorn, list), "ZCORN should be a list"
    assert isinstance(exported_coord, list), "COORD should be a list"
    assert isinstance(exported_actnum, list), "ACTNUM should be a list"

    # Check element data types
    assert all(isinstance(x, float) for x in exported_zcorn)
    assert all(isinstance(x, float) for x in exported_coord)
    assert all(isinstance(x, int) for x in exported_actnum)

    # Check dimension types
    assert isinstance(export_nx, int), "nx dimension should be integer"
    assert isinstance(export_ny, int), "ny dimension should be integer"
    assert isinstance(export_nz, int), "nz dimension should be integer"


def test_export_corner_point_grid_roundtrip(rips_instance, initialize_test):
    """Test round-trip: load Eclipse grid, export data, recreate grid, and compare geometry"""

    # Load the Eclipse grid file (using BRUGGE case without LGR complications)
    original_case = rips_instance.project.load_case(
        dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    )

    assert original_case is not None, "Failed to load test case"
    assert (
        original_case.name == "BRUGGE_0000"
    ), f"Expected case name BRUGGE_0000, got {original_case.name}"

    # Get original geometry data for comparison
    original_active_count = original_case.cell_count().active_cell_count
    original_cell_corners = original_case.active_cell_corners()

    # Export corner point grid data
    exported_zcorn, exported_coord, exported_actnum, nx, ny, nz = (
        original_case.export_corner_point_grid()
    )

    # Validate that the returned dimensions match the exported array sizes
    total_cells = len(exported_actnum)
    assert nx * ny * nz == total_cells

    # Validate array sizes make sense for these dimensions
    expected_coord_size = (nx + 1) * (ny + 1) * 6  # 6 values per coordinate line
    expected_zcorn_size = nx * ny * nz * 8  # 8 corner depths per cell
    expected_actnum_size = nx * ny * nz  # 1 value per cell

    assert len(exported_coord) == expected_coord_size
    assert len(exported_zcorn) == expected_zcorn_size
    assert len(exported_actnum) == expected_actnum_size

    # Create new corner point grid from exported data
    recreated_case = rips_instance.project.create_corner_point_grid(
        "Recreated_BRUGGE", nx, ny, nz, exported_coord, exported_zcorn, exported_actnum
    )

    assert recreated_case is not None, "Failed to create recreated case"

    # Compare geometry between original and recreated cases
    recreated_active_count = recreated_case.cell_count().active_cell_count
    recreated_cell_corners = recreated_case.active_cell_corners()

    # Compare active cell counts
    assert recreated_active_count == original_active_count

    # Compare number of corner points (should be same for active cells)
    assert len(recreated_cell_corners) == len(original_cell_corners)

    # Now do a focused comparison on a smaller sample
    sample_size = min(200, len(original_cell_corners))

    for i in range(sample_size):
        orig_cell = original_cell_corners[i]
        recreated_cell = recreated_cell_corners[i]

        for corner_idx in range(8):
            orig_corner = getattr(orig_cell, f"c{corner_idx}")
            recreated_corner = getattr(recreated_cell, f"c{corner_idx}")

            diff_x = abs(orig_corner.x - recreated_corner.x)
            diff_y = abs(orig_corner.y - recreated_corner.y)
            diff_z = abs(orig_corner.z - recreated_corner.z)

            max_diff = max(diff_x, diff_y, diff_z)
            assert max_diff < 1e-3
