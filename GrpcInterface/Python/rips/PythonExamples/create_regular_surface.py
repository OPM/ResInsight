######################################################################
# This script creates a regular surface for each case
######################################################################
import rips


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

        nx = 20
        ny = 10

        increment_x = (bbox.max_x - bbox.min_x) / float(nx)
        increment_y = (bbox.max_y - bbox.min_y) / float(ny)

        surface_collection = resinsight.project.descendants(rips.SurfaceCollection)[0]
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

        s.rotation = 45.0
        s.update()


        values = []
        for i in range(nx*ny):
            values.append(float(i))
        s.set_property("test", values)
