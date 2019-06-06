import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

resInsight = rips.Instance.find()
case       = resInsight.project.case(id=0)

porvChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PORV', 0)
porvResults = []
for porvChunk in porvChunks:
    for porv in porvChunk.values:
        porvResults.append(porv)

timeStepInfo = case.timeSteps()

for i in range (0, len(timeStepInfo)):
    soilChunks = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
    soilResults = []
    for soilChunk in soilChunks:
        for soil in soilChunk.values:
            soilResults.append(soil)
    results = []
    for (soil, porv) in zip(soilResults, porvResults):
        results.append(soil * porv)

    case.properties.setActiveCellProperty(results, 'GENERATED', 'SOILPORVSync', i)
print("Transferred all results back")