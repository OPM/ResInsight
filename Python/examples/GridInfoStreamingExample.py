import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

activeCellInfoChunks = resInsight.gridInfo.streamActiveCellInfo(caseId=0)

#print("Number of grids: " + str(gridCount))
#print(gridDimensions)

receivedActiveCells = []
for activeCellChunk in activeCellInfoChunks:
	for activeCell in activeCellChunk.data:
		receivedActiveCells.append(activeCell)
print("Number of active cells: " + str(len(receivedActiveCells)))
print("First active cell: ")
print(receivedActiveCells[0])
