###############################################################################
# This example will get the cell info for the active cells for the first case
###############################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()

# Get the first case. This will fail if you haven't loaded any cases
case = resinsight.project.cases()[0]

# Get the cell count object
cell_counts = case.cell_count()
print("Number of active cells: " + str(cell_counts.active_cell_count))
print("Total number of reservoir cells: " + str(cell_counts.reservoir_cell_count))

# Get information for all active cells
active_cell_infos = case.cell_info_for_active_cells()

# A simple check on the size of the cell info
assert cell_counts.active_cell_count == len(active_cell_infos)

# Print information for the first active cell
print("First active cell: ")
print(active_cell_infos[0])
