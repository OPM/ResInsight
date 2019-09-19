##############################################################################
# This example will create a derived result for each time step asynchronously
##############################################################################

import rips
import time

# Internal function for creating a result from a small chunk of soil and porv results
# The return value of the function is a generator for the results rather than the result itself.
def createResult(soilChunks, porvChunks):
    for (soilChunk, porvChunk) in zip(soilChunks, porvChunks):
        resultChunk = []
        number = 0
        for (soilValue, porvValue) in zip(soilChunk.values, porvChunk.values):
            resultChunk.append(soilValue * porvValue)
        # Return a Python generator
        yield resultChunk

resinsight   = rips.Instance.find()
start        = time.time()
case         = resinsight.project.case(id=0)
timeStepInfo = case.time_steps()

# Get a generator for the porv results. The generator will provide a chunk each time it is iterated
porvChunks   = case.properties.active_cell_property_async('STATIC_NATIVE', 'PORV', 0)

# Read the static result into an array, so we don't have to transfer it for each iteration
# Note we use the async method even if we synchronise here, because we need the values chunked
# ... to match the soil chunks
porvArray = []
for porvChunk in porvChunks:
    porvArray.append(porvChunk)

for i in range (0, len(timeStepInfo)):
    # Get a generator object for the SOIL property for time step i
    soilChunks = case.properties.active_cell_property_async('DYNAMIC_NATIVE', 'SOIL', i)
    # Create the generator object for the SOIL * PORV derived result
    result_generator = createResult(soilChunks, iter(porvArray))
    # Send back the result asynchronously with a generator object
    case.properties.set_active_cell_property_async(result_generator, 'GENERATED', 'SOILPORVAsync', i)

end = time.time()
print("Time elapsed: ", end - start)
        
print("Transferred all results back")

view = case.views()[0].applyCellResult('GENERATED', 'SOILPORVAsync')