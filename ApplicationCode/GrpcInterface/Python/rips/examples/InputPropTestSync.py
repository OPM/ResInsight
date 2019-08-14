import rips
import time
import grpc

resInsight     = rips.Instance.find()
start = time.time()
case = resInsight.project.case(id=0)

poroResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
permxResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)

results = []
for (poro, permx) in zip(poroResults, permxResults):
    results.append(poro * permx)

try:        
    case.properties.setActiveCellProperty(results, 'GENERATED', 'POROPERMXSY', 0)
except grpc.RpcError as e:
    print("Exception Received: ", e)


end = time.time()
print("Time elapsed: ", end - start)
print("Transferred all results back")