import ResInsight

resInsight     = ResInsight.Instance()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

activeCellInfo = resInsight.gridInfo.getActiveCellInfoArray(caseId=0)

#print("Number of grids: " + str(gridCount))
#print(gridDimensions)

receivedActiveCells = []
for activeCell in activeCellInfo:
	receivedActiveCells.append(activeCell)
print("Number of active cells: " + str(len(receivedActiveCells)))
print("First active cell: ")
print(receivedActiveCells[0])
