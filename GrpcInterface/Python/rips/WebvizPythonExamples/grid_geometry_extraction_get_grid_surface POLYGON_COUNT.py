import numpy as np

import plotly.graph_objects as go

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import *
from rips.generated.GridGeometryExtraction_pb2 import *

# from ..instance import *
# from ..generated.GridGeometryExtraction_pb2_grpc import *
# from ..generated.GridGeometryExtraction_pb2 import *

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

grid_file_name = (
    "D:/Git/resinsight-tutorials/model-data/norne/NORNE_ATW2013_RFTPLT_V2.EGRID"
)
# grid_file_name = "MOCKED_TEST_GRID"
# grid_file_name = "D:/ResInsight/GRID__SNORRE_BASECASEGRID.roff"

ijk_index_filter = GridGeometryExtraction__pb2.IJKIndexFilter(
    iMin=-1, iMax=-1, jMin=-1, jMax=-1, kMin=1, kMax=12
)
ijk_index_filter = None

get_grid_surface_request = GridGeometryExtraction__pb2.GetGridSurfaceRequest(
    gridFilename=grid_file_name,
    ijkIndexFilter=ijk_index_filter,
    cellIndexFilter=None,
    propertyFilter=None,
)
get_grid_surface_response: GridGeometryExtraction__pb2.GetGridSurfaceResponse = (
    grid_geometry_extraction_stub.GetGridSurface(get_grid_surface_request)
)

total_time_elapsed = get_grid_surface_response.timeElapsedInfo.totalTimeElapsedMs
named_events_and_time_elapsed = (
    get_grid_surface_response.timeElapsedInfo.namedEventsAndTimeElapsedMs
)

vertex_array = get_grid_surface_response.vertexArray
quad_indices_array = get_grid_surface_response.quadIndicesArr
origin_utm_xy = get_grid_surface_response.originUtmXy
source_cell_indices_arr = get_grid_surface_response.sourceCellIndicesArr
grid_dimensions = get_grid_surface_response.gridDimensions

num_vertex_coords = 3  # [x, y, z]
num_vertices_per_quad = 4  # [v1, v2, v3, v4]
num_quads = len(vertex_array) / (num_vertex_coords * num_vertices_per_quad)


print(f"Number of quads: {num_quads}")
print(f"Source cell indices array length: {len(source_cell_indices_arr)}")
print(f"Origin UTM coordinates [x, y]: [{origin_utm_xy.x}, {origin_utm_xy.y}]")
print(
    f"Grid dimensions [I, J, K]: [{grid_dimensions.i}, {grid_dimensions.j}, {grid_dimensions.k}]"
)
print(f"Total time elapsed: {total_time_elapsed} ms")
# print(f"Time elapsed per event [ms]: {named_events_and_time_elapsed}")
for message, time_elapsed in named_events_and_time_elapsed.items():
    print(f"{message}: {time_elapsed}")
