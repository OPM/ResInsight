import math
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot
import rips


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

    name = f"{c.name} bounding box"
    p = polygon_collection.create_polygon(name=name, coordinates=coordinates)
    assert p.name == name
    assert len(coordinates) == len(p.coordinates)
    for expected, actual in zip(coordinates, p.coordinates):
        assert len(expected) == len(actual)
        for e, a in zip(expected, actual):
            assert math.isclose(e, a, rel_tol=1e-9, abs_tol=0.0)


def test_add_folders_and_polygons(rips_instance, initialize_test):
    rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    folder_a = root.add_folder(folder_name="Folder A")
    folder_b = root.add_folder(folder_name="Folder B")
    nested = folder_a.add_folder(folder_name="Nested")

    sub_names = [s.polygon_collection_name for s in root.sub_collections()]
    assert "Folder A" in sub_names
    assert "Folder B" in sub_names
    assert [s.polygon_collection_name for s in folder_a.sub_collections()] == ["Nested"]

    coordinates = [
        [0.0, 0.0, -1000.0],
        [100.0, 0.0, -1000.0],
        [100.0, 100.0, -1000.0],
        [0.0, 100.0, -1000.0],
    ]

    root.create_polygon(name="root poly", coordinates=coordinates)
    folder_a.create_polygon(name="A poly", coordinates=coordinates)
    nested.create_polygon(name="nested poly", coordinates=coordinates)

    assert [p.name for p in root.polygons()] == ["root poly"]
    assert [p.name for p in folder_a.polygons()] == ["A poly"]
    assert [p.name for p in folder_b.polygons()] == []
    assert [p.name for p in nested.polygons()] == ["nested poly"]
