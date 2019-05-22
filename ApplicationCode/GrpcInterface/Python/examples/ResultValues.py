import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

resultChunks = resInsight.properties.activeCellResults(0, 'DYNAMIC_NATIVE', 'SOIL', 2)

results = []
for resultChunk in resultChunks:
	for value in resultChunk.values:
		results.append(value)
print("Transferred " + str(len(results)) + " cell results")
print("30th active cell: ")
print(results[29])

resultChunks = resInsight.properties.gridCellResults(0, 'DYNAMIC_NATIVE', 'SOIL', 2)

results = []
for resultChunk in resultChunks:
	for value in resultChunk.values:
		results.append(value)
print("Transferred " + str(len(results)) + " cell results")
print("124498th cell: ")
print(results[124498])
