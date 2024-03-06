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
grid_file_name = (
    "D:\\Git\\resinsight-tutorials\\model-data\\norne\\NORNE_ATW2013_RFTPLT_V2.EGRID"
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


fence_poly_line_utm_xy = norne_case_single_segment_poly_line_utm_xy

cut_along_polyline_request = GridGeometryExtraction__pb2.CutAlongPolylineRequest(
    gridFilename=grid_file_name,
    fencePolylineUtmXY=fence_poly_line_utm_xy,
)
cut_along_polyline_response: GridGeometryExtraction__pb2.CutAlongPolylineResponse = (
    grid_geometry_extraction_stub.CutAlongPolyline(cut_along_polyline_request)
)

polygon_vertex_array_org = cut_along_polyline_response.polygonVertexArray
vertices_per_polygon = cut_along_polyline_response.verticesPerPolygonArr
source_cell_indices = cut_along_polyline_response.sourceCellIndicesArr

x_start = polygon_vertex_array_org[0]
y_start = polygon_vertex_array_org[1]
z_start = polygon_vertex_array_org[2]

# Subtract x_start, y_start, z_start from all x, y, z coordinates
polygon_vertex_array = []
for i in range(0, len(polygon_vertex_array_org), 3):
    polygon_vertex_array.extend(
        [
            polygon_vertex_array_org[i] - x_start,
            polygon_vertex_array_org[i + 1] - y_start,
            polygon_vertex_array_org[i + 2] - z_start,
        ]
    )

num_vertex_coords = 3  # [x, y, z]

# Create x-, y-, and z-arrays
x_array = []
y_array = []
z_array = []
for i in range(0, len(polygon_vertex_array), num_vertex_coords):
    # vertex array is provided as a single array of x, y, z coordinates
    # i.e. [x1, y1, z1, x2, y2, z2, x3, y3, z3, ... , xn, yn, zn]
    x_array.append(polygon_vertex_array[i + 0])
    y_array.append(polygon_vertex_array[i + 1])
    z_array.append(polygon_vertex_array[i + 2])

# Create triangular mesh
vertices = np.array(polygon_vertex_array).reshape(-1, 3)

# Create mesh data
x, y, z = vertices.T
i = []
j = []
k = []

# Create edges between points in triangles
triangle_edges_x = []
triangle_edges_y = []
triangle_edges_z = []

# Populate i, j, k based on vertices_per_polygon
# Create triangles from each polygon
# A quad with vertex [0,1,2,3] will be split into two triangles [0,1,2] and [0,2,3]
# A hexagon with vertex [0,1,2,3,4,5] will be split into four triangles [0,1,2], [0,2,3], [0,3,4], [0,4,5]

polygon_v0_idx = 0  # Index of vertex 0 in the polygon
for vertex_count in vertices_per_polygon:
    # Must have at least one triangle
    if vertex_count < 3:
        polygon_v0_idx += vertex_count
        continue

    indices = list(range(polygon_v0_idx, polygon_v0_idx + vertex_count))

    # Build triangles from polygon
    num_triangles = vertex_count - 2
    for triangle_index in range(0, num_triangles):
        triangle_v0_idx = polygon_v0_idx
        triangle_v1_idx = indices[triangle_index + 1]
        triangle_v2_idx = indices[triangle_index + 2]

        # Vertex indices for the triangle
        i.append(triangle_v0_idx)
        j.append(triangle_v1_idx)
        k.append(triangle_v2_idx)

        # Create edge between vertices in triangle with x,y,z coordinates, coordinates per vertex is 3
        coordinate_step = 3  # step per vertex
        triangle_v0_global_idx = triangle_v0_idx * coordinate_step
        triangle_v1_global_idx = triangle_v1_idx * coordinate_step
        triangle_v2_global_idx = triangle_v2_idx * coordinate_step

        # Add x,y,z coordinates for the triangle vertices (closing triangle with 'None')
        triangle_edges_x.extend(
            [
                polygon_vertex_array[triangle_v0_global_idx + 0],
                polygon_vertex_array[triangle_v1_global_idx + 0],
                polygon_vertex_array[triangle_v2_global_idx + 0],
                polygon_vertex_array[triangle_v0_global_idx + 0],
                None,
            ]
        )
        triangle_edges_y.extend(
            [
                polygon_vertex_array[triangle_v0_global_idx + 1],
                polygon_vertex_array[triangle_v1_global_idx + 1],
                polygon_vertex_array[triangle_v2_global_idx + 1],
                polygon_vertex_array[triangle_v0_global_idx + 1],
                None,
            ]
        )
        triangle_edges_z.extend(
            [
                polygon_vertex_array[triangle_v0_global_idx + 2],
                polygon_vertex_array[triangle_v1_global_idx + 2],
                polygon_vertex_array[triangle_v2_global_idx + 2],
                polygon_vertex_array[triangle_v0_global_idx + 2],
                None,
            ]
        )

    # Move to next polygon
    polygon_v0_idx += vertex_count

# Create edges between points in polygons
polygon_edges_x = []
polygon_edges_y = []
polygon_edges_z = []
polygon_global_start_index = 0
coordinate_step = 3  # step per vertex
for vertex_count in vertices_per_polygon:
    # Must have at least a triangle
    if vertex_count < 3:
        polygon_global_start_index += vertex_count * coordinate_step
        continue

    for vertex_idx in range(0, vertex_count):
        vertex_global_idx = polygon_global_start_index + vertex_idx * coordinate_step
        polygon_edges_x.append(polygon_vertex_array[vertex_global_idx + 0])
        polygon_edges_y.append(polygon_vertex_array[vertex_global_idx + 1])
        polygon_edges_z.append(polygon_vertex_array[vertex_global_idx + 2])

    # Close the polygon
    polygon_edges_x.append(polygon_vertex_array[polygon_global_start_index + 0])
    polygon_edges_y.append(polygon_vertex_array[polygon_global_start_index + 1])
    polygon_edges_z.append(polygon_vertex_array[polygon_global_start_index + 2])

    polygon_edges_x.append(None)
    polygon_edges_y.append(None)
    polygon_edges_z.append(None)

    polygon_global_start_index += vertex_count * coordinate_step


# Create mesh
mesh_3D = go.Mesh3d(
    x=x, y=y, z=z, i=i, j=j, k=k, opacity=0.8, color="rgba(244,22,100,0.6)"
)

# Create edge lines for triangles
triangle_edges_3d = go.Scatter3d(
    x=triangle_edges_x,
    y=triangle_edges_y,
    z=triangle_edges_z,
    mode="lines",
    name="",
    line=dict(color="rgb(0,0,0)", width=1),
)

# Create outer edge lines for polygon
polygon_edges_3d = go.Scatter3d(
    x=polygon_edges_x,
    y=polygon_edges_y,
    z=polygon_edges_z,
    mode="lines",
    name="",
    line=dict(color="rgb(0,0,0)", width=1),
)

fig = go.Figure(
    data=[
        mesh_3D,
        # triangle_edges_3d,
        polygon_edges_3d,
    ]
)

# print(f"j array: {j_array}")
# print(f"Number of vertices: {len(vertex_array) / 3}")
# print(f"Number of traingles: {num_triangles}")
# print(f"Source cell indices array length: {len(source_cell_indices_arr)}")
# print(
#     f"Origin UTM coordinates [x, y, z]: [{origin_utm.x}, {origin_utm.y}, {origin_utm.z}]"
# )
# print(
#     f"Grid dimensions [I, J, K]: [{grid_dimensions.dimensions.i}, {grid_dimensions.dimensions.j}, {grid_dimensions.dimensions.k}]"
# )
print(fig.data)

fig.show()
