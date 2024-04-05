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

# grid_file_name = (
#     "D:/Git/resinsight-tutorials/model-data/norne/NORNE_ATW2013_RFTPLT_V2.EGRID"
# )
# grid_file_name = "MOCKED_TEST_GRID"
# grid_file_name = "D:/ResInsight/GRID__SNORRE_BASECASEGRID.roff"
grid_file_name = "D:/ResInsight/GRID__DROGON_13M.roff"

ijk_index_filter = GridGeometryExtraction__pb2.IJKIndexFilter(
    iMin=15, iMax=30, jMin=30, jMax=90, kMin=1, kMax=12
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
source_cell_indices_arr = get_grid_surface_response.sourceCellIndicesArr
origin_utm_xy = get_grid_surface_response.originUtmXy
grid_dimensions = get_grid_surface_response.gridDimensions

num_vertex_coords = 3  # [x, y, z]
num_vertices_per_quad = 4  # [v1, v2, v3, v4]
num_quads = len(quad_indices_array) / num_vertices_per_quad

# Create x-, y-, and z-arrays
x_array = []
y_array = []
z_array = []
for _, vertex_index in enumerate(quad_indices_array, 0):
    vertex_array_index = vertex_index * num_vertex_coords
    x_array.append(vertex_array[vertex_array_index + 0])
    y_array.append(vertex_array[vertex_array_index + 1])
    z_array.append(vertex_array[vertex_array_index + 2])

# Create triangular mesh
i_array = []
j_array = []
k_array = []
for i in range(0, len(quad_indices_array), num_vertices_per_quad):
    # Set the indices of the vertices of the triangles
    i_array.extend([i + 0, i + 0])
    j_array.extend([i + 1, i + 2])
    k_array.extend([i + 2, i + 3])


fig = go.Figure(
    data=[
        go.Mesh3d(
            x=x_array,
            y=y_array,
            z=z_array,
            i=i_array,
            j=j_array,
            k=k_array,
            intensity=np.linspace(-5, 5, 1000, endpoint=True),
            showscale=True,
            colorscale=[[0, "gold"], [0.5, "mediumturquoise"], [1.0, "magenta"]],
        )
    ]
)


print(f"Number of vertices in vertex array: {len(vertex_array) / 3}")
print(f"Number of quad vertices: {len(quad_indices_array)}")
print(f"Number of quads: {num_quads}")

print(f"Source cell indices array length: {len(source_cell_indices_arr)}")
print(f"Origin UTM coordinates [x, y]: [{origin_utm_xy.x}, {origin_utm_xy.y}]")
print(
    f"Grid dimensions [I, J, K]: [{grid_dimensions.i}, {grid_dimensions.j}, {grid_dimensions.k}]"
)
print(fig.data)

print(f"Total time elapsed: {total_time_elapsed} ms")
# print(f"Time elapsed per event [ms]: {named_events_and_time_elapsed}")
for message, time_elapsed in named_events_and_time_elapsed.items():
    print(f"{message}: {time_elapsed}")

fig.show()
