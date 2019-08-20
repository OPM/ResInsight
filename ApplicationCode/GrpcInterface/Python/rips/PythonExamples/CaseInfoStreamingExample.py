###############################################################################
# This example will get the cell info for the active cells for the first case
###############################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resInsight  = rips.Instance.find()

# Get the case with id == 0. This will fail if your project doesn't have a case with id == 0
case = resInsight.project.case(id = 0)

# Get the cell count object
cellCounts = case.cellCount()
print("Number of active cells: " + str(cellCounts.active_cell_count))
print("Total number of reservoir cells: " + str(cellCounts.reservoir_cell_count))

# Get information for all active cells
activeCellInfos = case.cellInfoForActiveCells()

# A simple check on the size of the cell info
assert(cellCounts.active_cell_count == len(activeCellInfos))

# Print information for the first active cell
print("First active cell: ")
print(activeCellInfos[0])
