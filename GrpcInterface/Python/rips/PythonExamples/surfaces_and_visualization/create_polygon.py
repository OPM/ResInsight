###################################################################################
# This example will connect to ResInsight, find bounding box and add a polygon
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is not None:
    # Get a list of all cases
    cases = resinsight.project.cases()

    for c in cases:
        print("Case name: " + c.name)

        # create a polygon which is same a the bounding box in x and y.
        # depth is set to middle of the bounding box
        bbox = c.reservoir_boundingbox()
        depth = bbox.max_z - ((bbox.max_z - bbox.min_z) / 2.0)

        coordinates = []
        coordinates.append([bbox.min_x, bbox.min_y, depth])
        coordinates.append([bbox.max_x, bbox.min_y, depth])
        coordinates.append([bbox.max_x, bbox.max_y, depth])
        coordinates.append([bbox.min_x, bbox.max_y, depth])

        polygon_collection = resinsight.project.descendants(rips.PolygonCollection)[0]
        p = polygon_collection.create_polygon(
            name="{} bounding box".format(c.name), coordinates=coordinates
        )
        print("Coordinates for {}:".format(p.name))
        for coord in p.coordinates:
            print(coord)
