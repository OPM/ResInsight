import math
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot
import rips


def test_static_completion_type_on_roff_grid(rips_instance, initialize_test):
    # Load a ROFF grid only - no restart and therefore no simulation time steps.
    # The "Completion Type" result used to be available only as a DYNAMIC result,
    # which requires time steps, so it could not be computed for a grid-only case.
    # This test verifies the STATIC_NATIVE variant added for issue #14127.
    #
    # Note: loading this case logs an error about missing PERMX/PERMY/PERMZ. That
    # comes from the (unrelated) fracture completion path and is harmless here - the
    # grid has no permeability, and the well has no fractures. Perforation and well
    # path intersections do not depend on it.
    case_path = dataroot.PATH + "/reek/reek_box_grid_w_props.roff"
    case = rips_instance.project.load_case(path=case_path)

    # This is the scenario from #14127: a grid with no time steps.
    assert len(case.time_steps()) == 0

    grid = case.grid(index=0)
    dims = grid.dimensions()
    total_cell_count = dims.i * dims.j * dims.k

    centers = grid.cell_centers()
    assert len(centers) == total_cell_count

    # Build a vertical well straight down a central column using real cell geometry,
    # so the well path is guaranteed to intersect cells in this case.
    i0 = dims.i // 2
    j0 = dims.j // 2

    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "static_completion_well"
    well_path.update()

    geometry = well_path.well_path_geometry()
    for k in range(1, dims.k + 1):  # IJK is 1-based
        index = grid.property_data_index_from_ijk(i0, j0, k)
        center = centers[index]
        # cell_centers() returns positive depth; append_well_target negates z internally.
        geometry.append_well_target([center.x, center.y, center.z], absolute=True)
    geometry.update()

    # Perforate the whole trajectory so every intersected cell is covered.
    trajectory = well_path.trajectory_properties(resampling_interval=5.0)
    measured_depths = trajectory["measured_depth"]
    assert len(measured_depths) > 2
    well_path.append_perforation_interval(
        start_md=measured_depths[0],
        end_md=measured_depths[-1],
        diameter=0.2,
        skin_factor=0.1,
    )

    # Extract the STATIC completion type result. This must succeed even though the
    # case has no time steps.
    completion_type = case.grid_property("STATIC_NATIVE", "Completion Type", 0)
    assert len(completion_type) == total_cell_count

    # Cells not touched by the well are undefined (HUGE_VAL -> inf), so only the
    # handful of cells along the well are expected to carry a value.
    defined_cells = [v for v in completion_type if math.isfinite(v)]
    assert len(defined_cells) > 0, (
        "Static completion type produced no intersected cells"
    )

    # RiaDefines::WellPathComponentType codes: WELL_PATH = 0, PERFORATION_INTERVAL = 1,
    # FISHBONES = 2, FRACTURE = 3. Every defined cell must be a valid code.
    valid_codes = {0.0, 1.0, 2.0, 3.0}
    assert all(v in valid_codes for v in defined_cells)

    # The full-length perforation must override the well path cells, so every defined
    # cell should be reported as a perforation.
    perforation_code = 1.0
    perforated_cells = [v for v in defined_cells if v == perforation_code]
    assert len(perforated_cells) == len(defined_cells), (
        "Expected all intersected cells to be reported as perforations"
    )
