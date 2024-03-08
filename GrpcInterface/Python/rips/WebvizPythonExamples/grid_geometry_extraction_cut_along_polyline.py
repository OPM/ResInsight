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

grid_file_name = "MOCKED_TEST_GRID"
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


fence_poly_line_utm_xy = norne_case_fence_poly_line_utm_xy

cut_along_polyline_request = GridGeometryExtraction__pb2.CutAlongPolylineRequest(
    gridFilename=grid_file_name,
    fencePolylineUtmXY=fence_poly_line_utm_xy,
)
cut_along_polyline_response: GridGeometryExtraction__pb2.CutAlongPolylineResponse = (
    grid_geometry_extraction_stub.CutAlongPolyline(cut_along_polyline_request)
)

fence_mesh_sections = cut_along_polyline_response.feceMeshSections
print(f"Number of fence mesh sections: {len(fence_mesh_sections)}")

section_mesh_3d = []
section_polygon_edges_3d = []
section_idx = 0
for section in fence_mesh_sections:
    # Continue to next section
    if section_idx == 0 and section_idx == 3:
        section_idx += 1
        continue

    section_idx += 1

    polygon_vertex_array_uz = section.vertexArrayUZ
    vertices_per_polygon = section.verticesPerPolygonArr

    start = section.startUtmXY
    end = section.endUtmXY

    # Create directional vector from start to end
    direction_vector = [end.x - start.x, end.y - start.y]

    # Normalize the directional vector
    length = np.sqrt(direction_vector[0] ** 2 + direction_vector[1] ** 2)
    direction_vector_norm = [direction_vector[0] / length, direction_vector[1] / length]

    # Decompose the polygon vertex array into x, y, z arrays
    # 2 coordinates per vertex (u, v)
    vertex_step = 2

    # Create x-, y-, and z-arrays
    x_array = []
    y_array = []
    z_array = []
    for i in range(0, len(polygon_vertex_array_uz), vertex_step):
        u = polygon_vertex_array_uz[i]
        z = polygon_vertex_array_uz[i + 1]

        # Calculate x, y from u and directional vector,
        # where u is the length along the direction vector
        x = start.x + u * direction_vector_norm[0]
        y = start.y + u * direction_vector_norm[1]

        x_array.append(x)
        y_array.append(y)
        z_array.append(z)

    i = []
    j = []
    k = []
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

        # Move to next polygon
        polygon_v0_idx += vertex_count

    # Create edges between points in polygons
    polygon_edges_x = []
    polygon_edges_y = []
    polygon_edges_z = []
    polygon_start_index = 0
    for vertex_count in vertices_per_polygon:
        # Must have at least a triangle
        if vertex_count < 3:
            polygon_start_index += vertex_count
            continue

        for vertex_idx in range(0, vertex_count):
            vertex_global_idx = polygon_start_index + vertex_idx
            polygon_edges_x.append(x_array[vertex_global_idx])
            polygon_edges_y.append(y_array[vertex_global_idx])
            polygon_edges_z.append(z_array[vertex_global_idx])

        # Close the polygon
        polygon_edges_x.append(x_array[polygon_start_index])
        polygon_edges_y.append(y_array[polygon_start_index])
        polygon_edges_z.append(z_array[polygon_start_index])

        polygon_edges_x.append(None)
        polygon_edges_y.append(None)
        polygon_edges_z.append(None)

        polygon_start_index += vertex_count

    # Add section mesh
    section_mesh_3d.append(
        go.Mesh3d(
            x=x_array,
            y=y_array,
            z=z_array,
            i=i,
            j=j,
            k=k,
            opacity=0.8,
            color="rgba(244,22,100,0.6)",
        )
    )

    # Add section polygon edges
    section_polygon_edges_3d.append(
        go.Scatter3d(
            x=polygon_edges_x,
            y=polygon_edges_y,
            z=polygon_edges_z,
            mode="lines",
            name="",
            line=dict(color="rgb(0,0,0)", width=1),
        )
    )

figure_data = section_mesh_3d + section_polygon_edges_3d

fig = go.Figure(data=figure_data)

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
