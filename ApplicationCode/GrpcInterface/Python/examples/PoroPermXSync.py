import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

resInsight     = rips.Instance.find()

case = resInsight.project.case(id=0)

poroChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
poroResults = []
for poroChunk in poroChunks:
    for poro in poroChunk.values:
        poroResults.append(poro)

permxChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)
permxResults = []
for permxChunk in permxChunks:
    for permx in permxChunk.values:
        permxResults.append(permx)

results = []
for (poro, permx) in zip(poroResults, permxResults):
    results.append(poro * permx)

case.properties.setActiveCellProperty(results, 'GENERATED', 'POROPERMXSY', 0)

print("Transferred all results back")