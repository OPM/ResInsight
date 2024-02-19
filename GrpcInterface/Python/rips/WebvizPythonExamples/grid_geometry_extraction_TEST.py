import sys
import os

import numpy as np

import plotly.graph_objects as go

sys.path.insert(1, os.path.join(sys.path[0], "../"))

from rips.instance import *
from rips.generated.GridGeometryExtraction_pb2_grpc import * 
from rips.generated.GridGeometryExtraction_pb2 import *

rips_instance = Instance.find()
grid_geometry_extraction_stub = GridGeometryExtractionStub(rips_instance.channel)

get_grid_surface_request = GridGeometryExtraction__pb2.GetGridSurfaceRequest(gridFilename=None, ijkIndexFilter=None,cellIndexFilter=None,propertyFilter=None)
get_grid_surface_response: GridGeometryExtraction__pb2.GetGridSurfaceResponse = grid_geometry_extraction_stub.GetGridSurface(get_grid_surface_request)

get_grid_surface_response.gridDimensions
vertex_array = get_grid_surface_response.vertexArray
quad_indices_array = get_grid_surface_response.quadIndicesArr

num_vertex_coords = 3 # [x, y, z]
num_vertices_per_quad = 4 # [v1, v2, v3, v4]
num_quads = len(vertex_array) /(num_vertex_coords * num_vertices_per_quad)

x_array = []
y_array = []
z_array = []

# Create x-, y-, and z-arrays
for i in range(0, len(vertex_array), num_vertex_coords):
    x_array.append(vertex_array[i])
    y_array.append(vertex_array[i+1])
    z_array.append(vertex_array[i+2])

# Create triangular mesh
i_array = []
j_array = []
k_array = []
for i in range(0, len(quad_indices_array), num_vertices_per_quad):
    # Set the indices of the vertices of the triangles
    i_array.extend([i, i])
    j_array.extend([i+1, i+2])
    k_array.extend([i+2, i+3])



fig = go.Figure(data=[go.Mesh3d(
    x=x_array, 
    y=y_array, 
    z=z_array,
    i=i_array,
    j=j_array,
    k=k_array,
    intensity = np.linspace(-5, 5, 1000, endpoint=True),
    showscale=True,
    colorscale=[[0, 'gold'],[0.5, 'mediumturquoise'],[1.0, 'magenta']]
)])

print(fig.data)

fig.show()

