import time

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import *
from rips.generated.GridGeometryExtraction_pb2 import *

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

# grid_file_name = "MOCKED_TEST_GRID"
grid_file_name = (
    "D:/Git/resinsight-tutorials/model-data/norne/NORNE_ATW2013_RFTPLT_V2.EGRID"
)

# ijk_index_filter = GridGeometryExtraction__pb2.IJKIndexFilter(
#     iMin=0, iMax=1, jMin=1, jMax=3, kMin=3, kMax=3
# )
ijk_index_filter = None

num_calls = 30
sleep_time_s = 0.5

for i in range(num_calls):
    print(f"Call {i+1}/{num_calls}")
    get_grid_surface_request = GridGeometryExtraction__pb2.GetGridSurfaceRequest(
        gridFilename=grid_file_name,
        ijkIndexFilter=ijk_index_filter,
        cellIndexFilter=None,
        propertyFilter=None,
    )
    get_grid_surface_response: GridGeometryExtraction__pb2.GetGridSurfaceResponse = (
        grid_geometry_extraction_stub.GetGridSurface(get_grid_surface_request)
    )

    time.sleep(sleep_time_s)

print("Done!")
