import time

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import *
from rips.generated.GridGeometryExtraction_pb2 import *

# from ..instance import *
# from ..generated.GridGeometryExtraction_pb2_grpc import *
# from ..generated.GridGeometryExtraction_pb2 import *

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

# grid_file_name = "MOCKED_TEST_GRID"
grid_file_name = (
    "D:/Git/resinsight-tutorials/model-data/norne/NORNE_ATW2013_RFTPLT_V2.EGRID"
)

# Test polylines
mocked_model_fence_poly_line_utm_xy = [
    11.2631,
    11.9276,
    14.1083,
    18.2929,
    18.3523,
    10.9173,
]
norne_case_fence_poly_line_utm_xy = [
    456221,
    7.32113e06,
    457150,
    7.32106e06,
    456885,
    7.32176e06,
    457648,
    7.3226e06,
    458805,
    7.32278e06,
]
norne_case_single_segment_poly_line_utm_xy = [457150, 7.32106e06, 456885, 7.32176e06]
norne_case_single_segment_poly_line_gap_utm_xy = [460877, 7.3236e06, 459279, 7.32477e06]


fence_poly_line_utm_xy = norne_case_fence_poly_line_utm_xy

num_calls = 20
sleep_time_s = 0.5

for i in range(num_calls):
    print(f"Call {i+1}/{num_calls}")
    cut_along_polyline_request = GridGeometryExtraction__pb2.CutAlongPolylineRequest(
        gridFilename=grid_file_name,
        fencePolylineUtmXY=fence_poly_line_utm_xy,
    )
    cut_along_polyline_response: (
        GridGeometryExtraction__pb2.CutAlongPolylineResponse
    ) = grid_geometry_extraction_stub.CutAlongPolyline(cut_along_polyline_request)

    time.sleep(sleep_time_s)

print("Done!")
