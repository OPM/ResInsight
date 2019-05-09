import ResInsight

resInsight     = ResInsight.Instance()
gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

activeCellInfo = resInsight.gridInfo.getAllActiveCellInfos(caseId=0)

print("Number of grids: " + str(gridCount))
print(gridDimensions)

print("Number of active cells: " + str(len(activeCellInfo)))
print("First active cell: ")
print(activeCellInfo[0])
