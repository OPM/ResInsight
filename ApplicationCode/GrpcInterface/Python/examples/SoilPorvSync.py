import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

porvChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PORV', 0)
porvResults = []
for porvChunk in porvChunks:
    for porv in porvChunk.values:
        porvResults.append(porv)

timeStepInfo = resInsight.gridInfo.timeSteps(0)

for i in range (0, len(timeStepInfo.date)): 
    soilChunks = resInsight.properties.activeCellResults(0, 'DYNAMIC_NATIVE', 'SOIL', i)
    soilResults = []
    for soilChunk in soilChunks:
        for soil in soilChunk.values:
            soilResults.append(soil)
    results = []
    for (soil, porv) in zip(soilResults, porvResults):
        results.append(soil * porv)

    resInsight.properties.setActiveCellResults(results, 0, 'GENERATED', 'SOILPORVSync', i)
print("Transferred all results back")