##############################################################################
# This example will create a derived result for each time step asynchronously
##############################################################################

import rips
import time

# Internal function for creating a result from a small chunk of soil and porv results
# The return value of the function is a generator for the results rather than the result itself.
def create_result(soil_chunks, porv_chunks):
    for (soil_chunk, porv_chunk) in zip(soil_chunks, porv_chunks):
        resultChunk = []
        number = 0
        for (soil_value, porv_value) in zip(soil_chunk.values, porv_chunk.values):
            resultChunk.append(soil_value * porv_value)
        # Return a Python generator
        yield resultChunk

resinsight   = rips.Instance.find()
start        = time.time()
case         = resinsight.project.cases()[0]
timeStepInfo = case.time_steps()

# Get a generator for the porv results. The generator will provide a chunk each time it is iterated
porv_chunks   = case.active_cell_property_async('STATIC_NATIVE', 'PORV', 0)

# Read the static result into an array, so we don't have to transfer it for each iteration
# Note we use the async method even if we synchronise here, because we need the values chunked
# ... to match the soil chunks
porv_array = []
for porv_chunk in porv_chunks:
    porv_array.append(porv_chunk)

for i in range (0, len(timeStepInfo)):
    # Get a generator object for the SOIL property for time step i
    soil_chunks = case.active_cell_property_async('DYNAMIC_NATIVE', 'SOIL', i)
    # Create the generator object for the SOIL * PORV derived result
    result_generator = create_result(soil_chunks, iter(porv_array))
    # Send back the result asynchronously with a generator object
    case.set_active_cell_property_async(result_generator, 'GENERATED', 'SOILPORVAsync', i)

end = time.time()
print("Time elapsed: ", end - start)
        
print("Transferred all results back")

view = case.views()[0].apply_cell_result('GENERATED', 'SOILPORVAsync')