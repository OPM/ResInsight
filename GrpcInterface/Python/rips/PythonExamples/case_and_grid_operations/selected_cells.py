############################################################################
# This example prints center and corners for the currently selected cells
# in ResInsight
############################################################################

import rips

resinsight = rips.Instance.find()
if resinsight is not None:
    cases = resinsight.project.cases()

    print("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        cells = case.selected_cells()
        print("Found " + str(len(cells)) + " selected cells")

        time_step_info = case.time_steps()

        for idx, cell in enumerate(cells):
            print(
                "Selected cell: [{}, {}, {}] grid: {}".format(
                    cell.ijk.i + 1, cell.ijk.j + 1, cell.ijk.k + 1, cell.grid_index
                )
            )

            # Get the grid and dimensions
            grid = case.grids()[cell.grid_index]
            dimensions = grid.dimensions()

            # Map ijk to cell index
            cell_index = (
                dimensions.i * dimensions.j * cell.ijk.k
                + dimensions.i * cell.ijk.j
                + cell.ijk.i
            )

            # Print the cell center
            cell_centers = grid.cell_centers()
            cell_center = cell_centers[cell_index]
            print(
                "Cell center: [{}, {}, {}]".format(
                    cell_center.x, cell_center.y, cell_center.z
                )
            )

            # Print the cell corners
            cell_corners = grid.cell_corners()[cell_index]
            print("Cell corners:")
            print("c0:\n" + str(cell_corners.c0))
            print("c1:\n" + str(cell_corners.c1))
            print("c2:\n" + str(cell_corners.c2))
            print("c3:\n" + str(cell_corners.c3))
            print("c4:\n" + str(cell_corners.c4))
            print("c5:\n" + str(cell_corners.c5))
            print("c6:\n" + str(cell_corners.c6))
            print("c7:\n" + str(cell_corners.c7))

            for tidx, timestep in enumerate(time_step_info):
                # Read the full SOIL result for time step
                soil_results = case.selected_cell_property(
                    "DYNAMIC_NATIVE", "SOIL", tidx
                )
                print(
                    "SOIL: {} ({}.{}.{})".format(
                        soil_results[idx], timestep.year, timestep.month, timestep.day
                    )
                )
