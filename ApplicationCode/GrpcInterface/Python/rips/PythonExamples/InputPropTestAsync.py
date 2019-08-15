import rips
import time

def createResult(poroChunks, permxChunks):
    for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
        resultChunk = []
        for (poro, permx) in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        yield resultChunk

resInsight     = rips.Instance.find()
start = time.time()
case = resInsight.project.case(id=0)

poroChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PORO', 0)
permxChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PERMX', 0)

case.properties.setActiveCellPropertyAsync(createResult(poroChunks, permxChunks),
                                           'GENERATED', 'POROPERMXAS', 0)

end = time.time()
print("Time elapsed: ", end - start)

print("Transferred all results back")