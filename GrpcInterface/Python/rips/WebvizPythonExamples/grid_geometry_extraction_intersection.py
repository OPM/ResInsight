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

grid_file_name = None
fence_poly_line_utm_xy = [11.2631, 11.9276, 14.1083, 18.2929, 18.3523, 10.9173]

cut_along_polyline_request = GridGeometryExtraction__pb2.CutAlongPolylineRequest(
    gridFilename=grid_file_name,
    fencePolylineUtmXY=fence_poly_line_utm_xy,
)
cut_along_polyline_response: GridGeometryExtraction__pb2.CutAlongPolylineResponse = (
    grid_geometry_extraction_stub.CutAlongPolyline(cut_along_polyline_request)
)

cut_along_polyline_response.gridDimensions
vertex_array = cut_along_polyline_response.vertexArray

num_vertex_coords = 3  # [x, y, z]
num_vertices_per_triangle = 3  # [v1, v2, v3]
num_triangles = len(vertex_array) / (num_vertex_coords * num_vertices_per_triangle)

# Create x-, y-, and z-arrays
x_array = []
y_array = []
z_array = []
for i in range(0, len(vertex_array), num_vertex_coords):
    x_array.append(vertex_array[i + 0] )
    y_array.append(vertex_array[i + 1] )
    z_array.append(vertex_array[i + 2] )

# Create triangular mesh
i_array = []
j_array = []
k_array = []
for i in range(0, len(x_array), num_vertices_per_triangle):
    # Set the indices of the vertices of the triangles
    i_array.extend([i + 0])
    j_array.extend([i + 1])
    k_array.extend([i + 2])


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

print(f"j array: {j_array}")
print(f"Number of vertices: {len(vertex_array) / 3}")
print(f"Number of traingles: {num_triangles}")
# print(f"Source cell indices array length: {len(source_cell_indices_arr)}")
# print(
#     f"Origin UTM coordinates [x, y, z]: [{origin_utm.x}, {origin_utm.y}, {origin_utm.z}]"
# )
# print(
#     f"Grid dimensions [I, J, K]: [{grid_dimensions.dimensions.i}, {grid_dimensions.dimensions.j}, {grid_dimensions.dimensions.k}]"
# )
print(fig.data)

fig.show()
