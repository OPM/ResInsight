import sys
import os
import math

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_create_polygon(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    c = project.cases()[0]
    bbox = c.reservoir_boundingbox()

    polygon_collection = rips_instance.project.descendants(rips.PolygonCollection)[0]

    coordinates = []
    coordinates.append([bbox.min_x, bbox.min_y, -1000.0])
    coordinates.append([bbox.max_x, bbox.min_y, -1000.0])
    coordinates.append([bbox.max_x, bbox.max_y, -1500.0])
    coordinates.append([bbox.min_x, bbox.max_y, -1500.0])

    name = "{} bounding box".format(c.name)
    p = polygon_collection.create_polygon(name=name, coordinates=coordinates)
    assert p.name == name
    assert len(coordinates) == len(p.coordinates)
    for expected, actual in zip(coordinates, p.coordinates):
        assert len(expected) == len(actual)
        for e, a in zip(expected, actual):
            assert math.isclose(e, a, rel_tol=1e-9, abs_tol=0.0)
