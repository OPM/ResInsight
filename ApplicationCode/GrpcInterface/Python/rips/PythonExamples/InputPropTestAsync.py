########################################################################################
# This example generates a derived property in an asynchronous manner
# Meaning it does not wait for all the data for each stage to be read before proceeding
########################################################################################
import rips
import time

# Internal function for creating a result from a small chunk of poro and permx results
# The return value of the function is a generator for the results rather than the result itself.
def createResult(poroChunks, permxChunks):
    # Loop through all the chunks of poro and permx in order
    for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
        resultChunk = []
        # Loop through all the values inside the chunks, in order
        for (poro, permx) in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        # Return a generator object that behaves like a Python iterator
        yield resultChunk

resInsight     = rips.Instance.find()
start = time.time()
case = resInsight.project.case(id=0)

# Get a generator for the poro results. The generator will provide a chunk each time it is iterated
poroChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PORO', 0)
# Get a generator for the permx results. The generator will provide a chunk each time it is iterated
permxChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PERMX', 0)

# Send back the result with the result provided by a generator object.
# Iterating the result generator will cause the script to read from the poro and permx generators
# And return the result of each iteration
case.properties.setActiveCellPropertyAsync(createResult(poroChunks, permxChunks),
                                           'GENERATED', 'POROPERMXAS', 0)

end = time.time()
print("Time elapsed: ", end - start)
print("Transferred all results back")
view = case.views()[0].applyCellResult('GENERATED', 'POROPERMXAS')