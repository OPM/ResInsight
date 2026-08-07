import sys
import os
import math

import pytest

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


def test_polygon_name_uniqueness(rips_instance, initialize_test):
    rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    coordinates = [
        [0.0, 0.0, -1000.0],
        [100.0, 0.0, -1000.0],
        [100.0, 100.0, -1000.0],
        [0.0, 100.0, -1000.0],
    ]

    root.create_polygon(name="Fence", coordinates=coordinates)

    # The default policy is to fail on a name already used in the same folder
    with pytest.raises(rips.RipsError, match="already exists"):
        root.create_polygon(name="Fence", coordinates=coordinates)

    renamed = root.create_polygon(
        name="Fence",
        coordinates=coordinates,
        on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME,
    )
    assert renamed.name == "Fence_1"

    # Names are case sensitive, so this is not a conflict
    other_case = root.create_polygon(name="fence", coordinates=coordinates)
    assert other_case.name == "fence"

    overwritten = root.create_polygon(
        name="Fence",
        coordinates=coordinates,
        on_name_conflict=rips.NameConflictPolicy.OVERWRITE,
    )
    assert overwritten.name == "Fence"
    assert [p.name for p in root.polygons()].count("Fence") == 1

    # A different folder is a separate namespace
    folder = root.add_folder(folder_name="Folder A")
    in_folder = folder.create_polygon(name="Fence", coordinates=coordinates)
    assert in_folder.name == "Fence"


def test_folder_name_uniqueness(rips_instance, initialize_test):
    rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    root.add_folder(folder_name="Folder A")

    with pytest.raises(rips.RipsError, match="already exists"):
        root.add_folder(folder_name="Folder A")

    renamed = root.add_folder(
        folder_name="Folder A",
        on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME,
    )
    assert renamed.polygon_collection_name == "Folder A_1"
