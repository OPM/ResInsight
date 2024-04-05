import numpy as np

import plotly.graph_objects as go

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import *
from rips.generated.GridGeometryExtraction_pb2 import *

# from ..instance import *
# from ..generated.GridGeometryExtraction_pb2_grpc import *
# from ..generated.GridGeometryExtraction_pb2 import *

from drogon_grid_well_path_polyline_xy_utm import drogon_well_path_polyline_xy_utm

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

grid_file_name = "MOCKED_TEST_GRID"
grid_file_name = (
    "D:/Git/resinsight-tutorials/model-data/norne/NORNE_ATW2013_RFTPLT_V2.EGRID"
)
grid_file_name = "D:/ResInsight/GRID__DROGON_13M.roff"

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

# Drogon 13M case

# Polyline along J-direction with I-index = 6
drogon_13M_start_utm_xy = [456189, 5.93605e06]
drogon_13M_end_utm_xy = [461625, 5.92663e06]
num_j_samples = 438

# Polyline random
# drogon_13M_start_utm_xy = [457026, 5.93502e06]
# drogon_13M_end_utm_xy = [466228, 5.93108e06]

num_polyline_samples = 10
drogon_13M_case_poly_line_utm_xy = [
    drogon_13M_start_utm_xy[0],
    drogon_13M_start_utm_xy[1],
]
for i in range(1, num_polyline_samples):
    x = drogon_13M_start_utm_xy[0] + (i / num_polyline_samples) * (
        drogon_13M_end_utm_xy[0] - drogon_13M_start_utm_xy[0]
    )
    y = drogon_13M_start_utm_xy[1] + (i / num_polyline_samples) * (
        drogon_13M_end_utm_xy[1] - drogon_13M_start_utm_xy[1]
    )

    drogon_13M_case_poly_line_utm_xy.append(x)
    drogon_13M_case_poly_line_utm_xy.append(y)
drogon_13M_case_poly_line_utm_xy.append(drogon_13M_end_utm_xy[0])
drogon_13M_case_poly_line_utm_xy.append(drogon_13M_end_utm_xy[1])

fence_poly_line_utm_xy = drogon_13M_case_poly_line_utm_xy

cut_along_polyline_request = GridGeometryExtraction__pb2.CutAlongPolylineRequest(
    gridFilename=grid_file_name,
    fencePolylineUtmXY=fence_poly_line_utm_xy,
)
cut_along_polyline_response: GridGeometryExtraction__pb2.CutAlongPolylineResponse = (
    grid_geometry_extraction_stub.CutAlongPolyline(cut_along_polyline_request)
)

total_time_elapsed = cut_along_polyline_response.timeElapsedInfo.totalTimeElapsedMs
named_events_and_time_elapsed = (
    cut_along_polyline_response.timeElapsedInfo.namedEventsAndTimeElapsedMs
)

fence_mesh_sections = cut_along_polyline_response.fenceMeshSections
print(f"Number of fence mesh sections: {len(fence_mesh_sections)}")

num_polygons = 0
for section in fence_mesh_sections:
    vertices_per_polygon = section.verticesPerPolygonArr
    num_polygons += len(vertices_per_polygon)

print(f"Total time elapsed: {total_time_elapsed} ms")
for message, time_elapsed in named_events_and_time_elapsed.items():
    print(f"{message}: {time_elapsed}")

print(f"Expected number of segments: {len(fence_poly_line_utm_xy) / 2 - 1}")
print(f"Number of segments: {len(fence_mesh_sections)}")
print(f"Number of polygons: {num_polygons}")
