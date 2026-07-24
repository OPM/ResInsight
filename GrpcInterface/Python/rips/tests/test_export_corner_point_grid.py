import math
import os
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot

GEOMETRY_PROPERTIES = ["DEPTH", "DX", "DY", "DZ", "TOPS", "BOTTOM"]


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

    # ACTNUM should be a queryable STATIC_NATIVE property (issue #14109)
    assert "ACTNUM" in case.available_properties("STATIC_NATIVE")
    active_count = case.cell_count().active_cell_count
    actnum_values = case.active_cell_property("STATIC_NATIVE", "ACTNUM", 0)
    assert len(actnum_values) == active_count
    assert all(v == 1.0 for v in actnum_values)

    # Geometry properties should be computed automatically (issue #14223)
    for prop in GEOMETRY_PROPERTIES:
        assert prop in case.available_properties("STATIC_NATIVE")

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


def test_create_corner_point_grid_with_inactive_cells(rips_instance, initialize_test):
    """ACTNUM with some inactive cells: the result is sized per active cell, not per
    reservoir cell, and every stored value is 1.0 (issue #14109)."""

    nx, ny, nz = 2, 2, 2
    total_cells = nx * ny * nz

    # Generate simple coordinate data
    coord = []
    for j in range(ny + 1):
        for i in range(nx + 1):
            x = i * 100.0
            y = j * 100.0
            coord.extend([x, y, 1000.0])
            coord.extend([x, y, 1100.0])

    # Generate simple ZCORN data (8 values per cell)
    zcorn = []
    for k in range(nz):
        for j in range(ny):
            for i in range(nx):
                base_depth = 1000.0 + k * 100.0
                zcorn.extend([base_depth] * 4 + [base_depth + 100.0] * 4)

    # Mark 3 of the 8 cells as inactive
    actnum = [1] * total_cells
    actnum[0] = 0
    actnum[3] = 0
    actnum[5] = 0
    expected_active = sum(actnum)  # 5

    case = rips_instance.project.create_corner_point_grid(
        "InactiveGrid", nx, ny, nz, coord, zcorn, actnum
    )

    cell_count = case.cell_count()
    # Inactive cells are excluded from the active-cell space, not stored as 0.
    assert cell_count.reservoir_cell_count == total_cells
    assert cell_count.active_cell_count == expected_active
    assert cell_count.active_cell_count < cell_count.reservoir_cell_count

    # ACTNUM is sized per active cell and every stored value is 1.0.
    assert "ACTNUM" in case.available_properties("STATIC_NATIVE")
    actnum_values = case.active_cell_property("STATIC_NATIVE", "ACTNUM", 0)
    assert len(actnum_values) == expected_active
    assert all(v == 1.0 for v in actnum_values)

    # grid_property returns the larger per-reservoir-cell array (one value per cell). Active cells
    # hold 1.0; inactive cells have no slot in the per-active-cell storage and so come back as the
    # undefined marker (inf), not 0 - the per-active-cell layout cannot represent an explicit 0.
    grid_values = case.grid_property("STATIC_NATIVE", "ACTNUM", 0)
    assert len(grid_values) == total_cells
    assert len(grid_values) > len(actnum_values)
    inactive_indices = {0, 3, 5}
    for i, v in enumerate(grid_values):
        if i in inactive_indices:
            assert math.isinf(v), f"cell {i} expected undefined (inf), got {v}"
        else:
            assert v == 1.0, f"cell {i} expected 1.0, got {v}"

    # Round-tripping through export restores the per-cell activity flags.
    _, _, exported_actnum, _, _, _ = case.export_corner_point_grid()
    assert len(exported_actnum) == total_cells
    assert sum(1 for x in exported_actnum if x > 0) == expected_active


def test_create_corner_point_grid_geometry_properties(rips_instance, initialize_test):
    """Geometry properties (DEPTH, DX, DY, DZ, TOPS, BOTTOM) are computed automatically
    when creating a grid from the python API, as when importing a grid from file (issue #14223)."""

    # 2x2x2 grid of 100 m cubic cells in two layers: 1000-1100 and 1100-1200
    nx, ny, nz = 2, 2, 2

    coord = []
    for j in range(ny + 1):
        for i in range(nx + 1):
            x = i * 100.0
            y = j * 100.0
            coord.extend([x, y, 1000.0])
            coord.extend([x, y, 1200.0])

    # ZCORN uses the Eclipse layout: for each layer, the 4*nx*ny top corner depths
    # followed by the 4*nx*ny bottom corner depths.
    zcorn = []
    for k in range(nz):
        top_depth = 1000.0 + k * 100.0
        zcorn.extend([top_depth] * (4 * nx * ny))
        zcorn.extend([top_depth + 100.0] * (4 * nx * ny))

    actnum = [1] * (nx * ny * nz)

    case = rips_instance.project.create_corner_point_grid(
        "GeometryPropertiesGrid", nx, ny, nz, coord, zcorn, actnum
    )

    for prop in GEOMETRY_PROPERTIES:
        assert prop in case.available_properties("STATIC_NATIVE")

    active_count = case.cell_count().active_cell_count
    values = {
        prop: case.active_cell_property("STATIC_NATIVE", prop, 0)
        for prop in GEOMETRY_PROPERTIES
    }
    for prop in GEOMETRY_PROPERTIES:
        assert len(values[prop]) == active_count

    # Active cell index order follows the cell index order (i fastest, k slowest),
    # so the first 4 cells are in layer k=0 and the last 4 in layer k=1.
    for active_index in range(active_count):
        k = active_index // (nx * ny)
        assert values["DX"][active_index] == pytest.approx(100.0)
        assert values["DY"][active_index] == pytest.approx(100.0)
        assert values["DZ"][active_index] == pytest.approx(100.0)
        assert values["TOPS"][active_index] == pytest.approx(1000.0 + k * 100.0)
        assert values["BOTTOM"][active_index] == pytest.approx(1100.0 + k * 100.0)
        assert values["DEPTH"][active_index] == pytest.approx(1050.0 + k * 100.0)


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
    assert original_case.name == "BRUGGE_0000", (
        f"Expected case name BRUGGE_0000, got {original_case.name}"
    )

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

    # The recreated grid should expose ACTNUM just like the loaded Eclipse grid (issue #14109)
    assert "ACTNUM" in original_case.available_properties("STATIC_NATIVE")
    assert "ACTNUM" in recreated_case.available_properties("STATIC_NATIVE")
    assert (
        len(recreated_case.active_cell_property("STATIC_NATIVE", "ACTNUM", 0))
        == recreated_active_count
    )

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


def test_replace_corner_point_grid_basic(rips_instance, initialize_test):
    """Test the replace_corner_point_grid method by creating a grid and replacing it with a larger one"""

    # Create initial grid with 2x2x2 cells
    nx_init, ny_init, nz_init = 2, 2, 2

    # Generate simple coordinate data for initial grid
    coord_init = []
    for j in range(ny_init + 1):
        for i in range(nx_init + 1):
            x = i * 100.0
            y = j * 100.0
            # Top point
            coord_init.extend([x, y, 1000.0])
            # Bottom point
            coord_init.extend([x, y, 1100.0])

    # Generate simple ZCORN data for initial grid
    zcorn_init = []
    for k in range(nz_init):
        for j in range(ny_init):
            for i in range(nx_init):
                base_depth = 1000.0 + k * 100.0
                # 8 corner depths for each cell
                zcorn_init.extend(
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

    # Generate ACTNUM for initial grid (all cells active)
    actnum_init = [1] * (nx_init * ny_init * nz_init)

    # Create the initial grid case
    case = rips_instance.project.create_corner_point_grid(
        "InitialGrid", nx_init, ny_init, nz_init, coord_init, zcorn_init, actnum_init
    )

    # Verify initial grid dimensions
    initial_cell_count = case.cell_count()
    assert initial_cell_count.reservoir_cell_count == nx_init * ny_init * nz_init

    # Now create replacement grid with more cells (3x3x3)
    nx_new, ny_new, nz_new = 3, 3, 3

    # Generate coordinate data for new grid
    coord_new = []
    for j in range(ny_new + 1):
        for i in range(nx_new + 1):
            x = i * 100.0
            y = j * 100.0
            # Top point
            coord_new.extend([x, y, 1000.0])
            # Bottom point
            coord_new.extend([x, y, 1200.0])

    # Generate ZCORN data for new grid
    zcorn_new = []
    for k in range(nz_new):
        for j in range(ny_new):
            for i in range(nx_new):
                base_depth = 1000.0 + k * 100.0
                # 8 corner depths for each cell
                zcorn_new.extend(
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

    # Generate ACTNUM for new grid (all cells active)
    actnum_new = [1] * (nx_new * ny_new * nz_new)

    # Replace the grid geometry
    case.replace_corner_point_grid(
        nx_new, ny_new, nz_new, coord_new, zcorn_new, actnum_new
    )

    # Verify that the grid now has the new dimensions
    new_cell_count = case.cell_count()
    assert new_cell_count.reservoir_cell_count == nx_new * ny_new * nz_new
    assert new_cell_count.reservoir_cell_count == 27  # 3x3x3

    # Geometry properties should be re-computed for the replaced grid (issue #14223)
    for prop in GEOMETRY_PROPERTIES:
        assert prop in case.available_properties("STATIC_NATIVE")
        prop_values = case.active_cell_property("STATIC_NATIVE", prop, 0)
        assert len(prop_values) == new_cell_count.active_cell_count

    # ACTNUM should be re-created for the replaced grid (issue #14109)
    assert "ACTNUM" in case.available_properties("STATIC_NATIVE")
    actnum_values = case.active_cell_property("STATIC_NATIVE", "ACTNUM", 0)
    assert len(actnum_values) == new_cell_count.active_cell_count
    assert len(actnum_values) == 27  # 3x3x3, all active
    assert all(v == 1.0 for v in actnum_values)

    # Verify the grid has more cells than before
    assert new_cell_count.reservoir_cell_count > initial_cell_count.reservoir_cell_count

    # Export the replaced grid and verify dimensions
    exported_zcorn, exported_coord, exported_actnum, export_nx, export_ny, export_nz = (
        case.export_corner_point_grid()
    )

    # Verify dimensions match
    assert export_nx == nx_new
    assert export_ny == ny_new
    assert export_nz == nz_new

    # Verify array sizes
    print(exported_actnum)
    assert len(exported_actnum) == nx_new * ny_new * nz_new
    # All cells should be active
    assert sum(1 for x in exported_actnum if x > 0) == len(actnum_new)
