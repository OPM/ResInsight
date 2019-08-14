import rips
import grpc

resInsight     = rips.Instance.find()

case = None

# Try loading a non-existing case. We should get a grpc.RpcError exception from the server
try:
    case = resInsight.project.loadCase("Nonsense")
except grpc.RpcError as e:
    print("Expected Server Exception Received: ", e)

case = resInsight.project.case(id=0)
if case is not None:
    results = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
    activeCellCount = len(results)

    # Send the results back to ResInsight inside try / except construct
    try:        
        case.properties.setActiveCellProperty(results, 'GENERATED', 'POROAPPENDED', 0)
        print("Everything went well as expected")
    except: # Match any exception, but it should not happen
        print("Ooops!")

    # Add another value, so this is outside the bounds of the active cell result storage
    results.append(1.0)

    # This time we should get a grpc.RpcError exception, which is a server side error.
    try:        
        case.properties.setActiveCellProperty(results, 'GENERATED', 'POROAPPENDED', 0)
        print("Everything went well??")
    except grpc.RpcError as e:
        print("Expected Server Exception Received: ", e)
    except IndexError:
        print ("Got index out of bounds error. This shouldn't happen here")

    # With a chunk size exactly matching the active cell count the server will not
    # be able to see any error as it will successfully close the stream after receiving
    # the correct number of values, even if the python client has more chunks to send
    case.properties.chunkSize = activeCellCount

    try:        
        case.properties.setActiveCellProperty(results, 'GENERATED', 'POROAPPENDED', 0)
        print("Everything went well??")
    except grpc.RpcError as e:
        print("Got unexpected server exception", e, "This should not happen now")
    except IndexError:
        print ("Got expected index out of bounds error on client side")

        

