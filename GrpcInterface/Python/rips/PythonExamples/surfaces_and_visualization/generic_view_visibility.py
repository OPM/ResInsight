###################################################################################
# This example shows how to create a case-less generic 3D view and toggle
# surface and polygon visibility in it via Python
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()

project = resinsight.project

# Create a generic view without loading a grid case
view = project.create_generic_view(name="Overview")

# Create a regular surface and toggle its visibility in the generic view
surface_collection = project.descendants(rips.SurfaceCollection)[0]
surface = surface_collection.new_regular_surface(name="S1", nx=2, ny=2)
surface.set_property("Depth", [1000.0, 1010.0, 1020.0, 1030.0])

view.set_surface_visible(surface=surface, visible=True)
view.set_surface_property(surface=surface, property_name="Depth")

# Create a polygon and toggle its visibility in the generic view
polygon_collection = project.descendants(rips.PolygonCollection)[0]
polygon = polygon_collection.create_polygon(
    name="P1",
    coordinates=[
        [0.0, 0.0, -1000.0],
        [100.0, 0.0, -1000.0],
        [100.0, 100.0, -1000.0],
    ],
)

view.set_polygon_visible(polygon=polygon, visible=True)

print(
    "Generic view '{}' now shows surface '{}' and polygon '{}'".format(
        view.address(), surface.surface_user_description, polygon.name
    )
)
