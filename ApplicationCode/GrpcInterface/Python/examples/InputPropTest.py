import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips
import itertools

resInsight     = rips.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

resultChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PORO', 0)

results = []
for resultChunk in resultChunks:
	for value in resultChunk.values:
		results.append(value)

print("Transferred " + str(len(results)) + " poro results")

resultChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PERMX', 0)

permres = []
for resultChunk in resultChunks:
	for value in resultChunk.values:
		permres.append(value)

print("Transferred " + str(len(permres)) + " permx results")
poropermx = []
for (poro, permx) in zip(results, permres):
    poropermx.append(poro * permx)

resInsight.properties.setActiveCellResults(poropermx, 0, 'GENERATED', 'PORO*PERMX2', 0)
