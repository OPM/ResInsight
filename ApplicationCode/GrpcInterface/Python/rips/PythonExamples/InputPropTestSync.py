########################################################################################
# This example generates a derived property in an synchronous manner
# Meaning it completes reading each result before calculating the derived result
# See InputPropTestAsync for how to do this asynchronously instead.
########################################################################################
import rips
import time
import grpc

resinsight     = rips.Instance.find()
start = time.time()
case = resinsight.project.case(id=0)

# Read poro result into list
poroResults = case.properties.active_cell_property('STATIC_NATIVE', 'PORO', 0)
# Read permx result into list
permxResults = case.properties.active_cell_property('STATIC_NATIVE', 'PERMX', 0)

# Generate output result
results = []
for (poro, permx) in zip(poroResults, permxResults):
    results.append(poro * permx)

try:
    # Send back output result
    case.properties.set_active_cell_property(results, 'GENERATED', 'POROPERMXSY', 0)
except grpc.RpcError as e:
    print("Exception Received: ", e)


end = time.time()
print("Time elapsed: ", end - start)
print("Transferred all results back")

view = case.views()[0].applyCellResult('GENERATED', 'POROPERMXSY')