######################################################################
# This script creates a regular surface for each case
######################################################################
import rips
import math


def create_x_surface(nx, ny):
    surface = []
    for j in range(ny):
        for i in range(nx):
            surface.append(float(i))
    return surface


def create_wave_surface(nx, ny):
    # Fill the coordinate and wave pattern arrays
    surface = []
    for j in range(ny):
        for i in range(nx):
            # Create wave pattern - combining multiple sine waves
            x = -5 + 10 * i / (nx - 1)
            y = -5 + 10 * j / (ny - 1)
            offset = (
                math.sin(x**2 + y**2)
                + 0.5 * math.sin(2 * x) * math.cos(2 * y)
                + 25.0 * math.cos(5 * x + 2 * y)
            )
            surface.append(-depth + offset)
    return surface


resinsight = rips.Instance.find()

project = resinsight.project


if resinsight is not None:
    # Get a list of all cases
    cases = resinsight.project.cases()

    for c in cases:
        bbox = c.reservoir_boundingbox()
        depth = bbox.max_z - ((bbox.max_z - bbox.min_z) / 2.0)

        origin_x = bbox.min_x
        origin_y = bbox.min_y

        name = "{} surface".format(c.name)

        nx = 200
        ny = 100

        increment_x = (bbox.max_x - bbox.min_x) / float(nx)
        increment_y = (bbox.max_y - bbox.min_y) / float(ny)

        surface_collection = resinsight.project.descendants(rips.SurfaceCollection)[0]

        # Create a surface at a given depth
        s = surface_collection.new_regular_surface(
            name=name,
            origin_x=origin_x,
            origin_y=origin_y,
            depth=-depth,
            nx=nx,
            ny=ny,
            increment_x=increment_x,
            increment_y=increment_y,
        )

        # Rotate the resulting surface
        s.rotation = 45.0
        s.update()

        # Add one property
        s.set_property("first_property", create_x_surface(nx, ny))

        # Add a wave surface
        s.set_property("wave", create_wave_surface(nx, ny))

        # Use the wave as depth for the surface
        s.set_property_as_depth("wave")
