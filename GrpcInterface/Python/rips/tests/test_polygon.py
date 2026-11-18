import sys
import os
import math
import typing

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


def test_set_polygon_visible_in_view(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    polygon_collection = project.descendants(rips.PolygonCollection)[0]
    polygon = polygon_collection.create_polygon(
        name="View polygon",
        coordinates=[
            [0.0, 0.0, -1000.0],
            [100.0, 0.0, -1000.0],
            [100.0, 100.0, -1000.0],
        ],
    )

    case = project.cases()[0]
    first_view = case.views()[0]
    second_view = case.create_view()

    assert first_view.set_polygon_visible(polygon=polygon, visible=False) is None
    assert second_view.set_polygon_visible(polygon=polygon, visible=True) is None
    assert first_view.set_polygon_visible(polygon=polygon, visible=True) is None

    with pytest.raises(rips.RipsError, match="Polygon is null"):
        first_view.set_polygon_visible(polygon=None, visible=True)


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


def test_add_folder_returns_polygon_collection(rips_instance, initialize_test):
    rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )

    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    folder = root.add_folder(folder_name="Folder A")
    assert type(folder) is rips.PolygonCollection
    assert folder.collection_name() == "Folder A"
    assert folder.polygon_collection_name == "Folder A"

    # The generated classes declare the concrete folder type on each collection class
    assert (
        typing.get_type_hints(rips.PolygonCollection.add_folder)["return"]
        is rips.PolygonCollection
    )


def _write_pol_file(path, polygons):
    # .pol files store depth, so z is written with the sign flipped
    lines = []
    for polygon in polygons:
        lines += [f"{x} {y} {-z}" for x, y, z in polygon]
        lines.append("999.0 999.0 999.0")
    path.write_text("\n".join(lines) + "\n")


def test_import_polygon_file(rips_instance, initialize_test, tmp_path):
    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    first = [[0.0, 0.0, -1000.0], [100.0, 0.0, -1000.0], [100.0, 100.0, -1500.0]]
    second = [[200.0, 0.0, -1000.0], [300.0, 0.0, -1000.0], [300.0, 100.0, -1000.0]]

    single_file = tmp_path / "single.pol"
    _write_pol_file(single_file, [first])

    polygon = root.import_polygon_file(file_name=str(single_file))
    assert polygon.name == "single"
    assert len(polygon.coordinates) == len(first)
    for expected, actual in zip(first, polygon.coordinates):
        for e, a in zip(expected, actual):
            assert math.isclose(e, a, rel_tol=1e-9, abs_tol=0.0)

    multi_file = tmp_path / "multi.pol"
    _write_pol_file(multi_file, [first, second])

    polygon = root.import_polygon_file(file_name=str(multi_file))
    assert polygon.name == "multi (1)"

    folder_names = [s.collection_name() for s in root.sub_collections()]
    assert folder_names == ["single", "multi"]


def test_import_polygon_file_errors(rips_instance, initialize_test, tmp_path):
    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    with pytest.raises(rips.RipsError, match="does not exist"):
        root.import_polygon_file(file_name=str(tmp_path / "missing.pol"))

    empty_file = tmp_path / "empty.pol"
    empty_file.write_text("")
    with pytest.raises(rips.RipsError, match="No polygons found"):
        root.import_polygon_file(file_name=str(empty_file))

    assert root.sub_collections() == []


def test_import_polygon_file_name_conflict(rips_instance, initialize_test, tmp_path):
    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    coordinates = [[0.0, 0.0, -1000.0], [100.0, 0.0, -1000.0], [100.0, 100.0, -1000.0]]
    pol_file = tmp_path / "fence.pol"
    _write_pol_file(pol_file, [coordinates])

    root.import_polygon_file(file_name=str(pol_file))

    # The default policy is to fail on a folder name already used in the same folder
    with pytest.raises(rips.RipsError, match="already exists"):
        root.import_polygon_file(file_name=str(pol_file))

    renamed = root.import_polygon_file(
        file_name=str(pol_file), on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME
    )
    assert renamed.name == "fence_1"

    overwritten = root.import_polygon_file(
        file_name=str(pol_file), on_name_conflict=rips.NameConflictPolicy.OVERWRITE
    )
    assert overwritten.name == "fence"

    folder_names = [s.collection_name() for s in root.sub_collections()]
    assert folder_names == ["fence_1", "fence"]


def test_import_polygon_file_with_name(rips_instance, initialize_test, tmp_path):
    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    first = [[0.0, 0.0, -1000.0], [100.0, 0.0, -1000.0], [100.0, 100.0, -1000.0]]
    second = [[200.0, 0.0, -1000.0], [300.0, 0.0, -1000.0], [300.0, 100.0, -1000.0]]

    # A single polygon is named after the folder
    single_file = tmp_path / "single.pol"
    _write_pol_file(single_file, [first])
    polygon = root.import_polygon_file(file_name=str(single_file), name="My Polygon")
    assert polygon.name == "My Polygon"

    # Several polygons get the folder name as prefix
    multi_file = tmp_path / "multi.pol"
    _write_pol_file(multi_file, [first, second])
    polygon = root.import_polygon_file(file_name=str(multi_file), name="Faults")
    assert polygon.name == "Faults (1)"

    folders = root.sub_collections()
    assert [s.collection_name() for s in folders] == ["My Polygon", "Faults"]
    polygon_names = [p.name for p in folders[1].descendants(rips.Polygon)]
    assert polygon_names == ["Faults (1)", "Faults (2)"]


def test_import_polygon_file_with_name_conflict(
    rips_instance, initialize_test, tmp_path
):
    root = rips_instance.project.descendants(rips.PolygonCollection)[0]

    coordinates = [[0.0, 0.0, -1000.0], [100.0, 0.0, -1000.0], [100.0, 100.0, -1000.0]]
    pol_file = tmp_path / "fence.pol"
    _write_pol_file(pol_file, [coordinates])

    root.import_polygon_file(file_name=str(pol_file), name="Boundary")

    # The conflict policy applies to the given name, not the file name
    with pytest.raises(rips.RipsError, match="already exists"):
        root.import_polygon_file(file_name=str(pol_file), name="Boundary")

    renamed = root.import_polygon_file(
        file_name=str(pol_file),
        name="Boundary",
        on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME,
    )
    assert renamed.name == "Boundary_1"

    overwritten = root.import_polygon_file(
        file_name=str(pol_file),
        name="Boundary",
        on_name_conflict=rips.NameConflictPolicy.OVERWRITE,
    )
    assert overwritten.name == "Boundary"

    # No conflict with a folder named after the file
    from_file = root.import_polygon_file(file_name=str(pol_file))
    assert from_file.name == "fence"

    folder_names = [s.collection_name() for s in root.sub_collections()]
    assert folder_names == ["Boundary_1", "Boundary", "fence"]
