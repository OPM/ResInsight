import rips
import time

resInsight     = rips.Instance.find()
start = time.time()
case = resInsight.project.case(id=0)

poroResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
permxResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)

results = []
for (poro, permx) in zip(poroResults, permxResults):
    results.append(poro * permx)

case.properties.setActiveCellProperty(results, 'GENERATED', 'POROPERMXSY', 0)

end = time.time()
print("Time elapsed: ", end - start)
print("Transferred all results back")