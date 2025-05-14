###################################################################
# This example demonstrates the use of ResInsight exceptions
# for proper error handling
###################################################################

import rips
import tempfile


resinsight = rips.Instance.find()

case = None

# Try loading a non-existing case. We should get a grpc.RpcError exception from the server
try:
    case = resinsight.project.load_case("Nonsense")
except rips.RipsError as e:
    print("Expected Server Exception Received while loading case: ", e)

# Try loading well paths from a non-existing folder.  We should get a rips.RipsError exception from the server
try:
    well_path_files = resinsight.project.import_well_paths(
        well_path_folder="NONSENSE/NONSENSE"
    )
except rips.RipsError as e:
    print("Server Exception Received while loading wellpaths: ", e)

# Try loading well paths from an existing but empty folder. We should get a warning.
try:
    with tempfile.TemporaryDirectory() as tmpdirname:
        well_path_files = resinsight.project.import_well_paths(
            well_path_folder=tmpdirname
        )
        assert len(well_path_files) == 0
        assert resinsight.project.has_warnings()
        print("Should get warnings below")
        for warning in resinsight.project.warnings():
            print(warning)
except rips.RipsError as e:
    print("Unexpected Server Exception caught!!!", e)

case = resinsight.project.case(case_id=0)
if case is not None:
    results = case.active_cell_property("STATIC_NATIVE", "PORO", 0)
    active_cell_count = len(results)

    # Send the results back to ResInsight inside try / except construct
    try:
        case.set_active_cell_property(results, "GENERATED", "POROAPPENDED", 0)
        print("Everything went well as expected")
    except Exception as e:  # Match any exception, but it should not happen
        print("Ooops!", e)

    # Add another value, so this is outside the bounds of the active cell result storage
    results.append(1.0)

    # This time we should get a rips.RipsError exception.
    try:
        case.set_active_cell_property(results, "GENERATED", "POROAPPENDED", 0)
        print("Everything went well??")
    except rips.RipsError as e:
        print("Server Exception Received: ", e)
    except IndexError:
        print("Got index out of bounds error. This shouldn't happen here")

    # With a chunk size exactly matching the active cell count the server will not
    # be able to see any error as it will successfully close the stream after receiving
    # the correct number of values, even if the python client has more chunks to send
    case.chunk_size = active_cell_count

    try:
        case.set_active_cell_property(results, "GENERATED", "POROAPPENDED", 0)
        print("Everything went well??")
    except rips.RipsError as e:
        print("Got unexpected server exception", e, "This should not happen now")
    except IndexError:
        print("Got expected index out of bounds error on client side")
