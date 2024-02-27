import numpy as np

import plotly.graph_objects as go

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import *
from rips.generated.GridGeometryExtraction_pb2 import *

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

grid_file_name = None
grid_file_name = (
    "D:\\Git\\resinsight-tutorials\\model-data\\norne\\NORNE_ATW2013_RFTPLT_V2.EGRID"
)
ijk_index_filter = GridGeometryExtraction__pb2.IJKIndexFilter(
    iMin=0, iMax=1, jMin=1, jMax=3, kMin=0, kMax=3
)
# ijk_index_filter = None

get_grid_surface_request = GridGeometryExtraction__pb2.GetGridSurfaceRequest(
    gridFilename=grid_file_name,
    ijkIndexFilter=ijk_index_filter,
    cellIndexFilter=None,
    propertyFilter=None,
)
get_grid_surface_response: GridGeometryExtraction__pb2.GetGridSurfaceResponse = (
    grid_geometry_extraction_stub.GetGridSurface(get_grid_surface_request)
)

get_grid_surface_response.gridDimensions
vertex_array = get_grid_surface_response.vertexArray
quad_indices_array = get_grid_surface_response.quadIndicesArr
origin_utm = get_grid_surface_response.originUtm
source_cell_indices_arr = get_grid_surface_response.sourceCellIndicesArr
grid_dimensions = get_grid_surface_response.gridDimensions

num_vertex_coords = 3  # [x, y, z]
num_vertices_per_quad = 4  # [v1, v2, v3, v4]
num_quads = len(vertex_array) / (num_vertex_coords * num_vertices_per_quad)

# Create x-, y-, and z-arrays
x_array = []
y_array = []
z_array = []
for i in range(0, len(vertex_array), num_vertex_coords):
    x_array.append(vertex_array[i + 0] + origin_utm.x)
    y_array.append(vertex_array[i + 1] + origin_utm.y)
    z_array.append(vertex_array[i + 2] + origin_utm.z)

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

print(f"Number of quads: {num_quads}")
print(f"Source cell indices array length: {len(source_cell_indices_arr)}")
print(
    f"Origin UTM coordinates [x, y, z]: [{origin_utm.x}, {origin_utm.y}, {origin_utm.z}]"
)
print(
    f"Grid dimensions [I, J, K]: [{grid_dimensions.dimensions.i}, {grid_dimensions.dimensions.j}, {grid_dimensions.dimensions.k}]"
)
print(fig.data)

fig.show()
