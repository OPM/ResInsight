import sys
import os
import tempfile
import typing
import pytest
from pathlib import Path

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_create_and_export_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    surface = surface_collection.new_surface(case, 5)
    assert surface

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        path = Path(tmpdirname, "mysurface.ts")
        print("Temporary folder: ", path.as_posix())

        surface.export_to_file(path.as_posix())

        assert path.exists()


def test_create_regular_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    bbox = c.reservoir_boundingbox()
    depth = bbox.max_z - ((bbox.max_z - bbox.min_z) / 2.0)

    origin_x = bbox.min_x
    origin_y = bbox.min_y
    name = "test surface name"
    nx = 20
    ny = 10
    increment_x = (bbox.max_x - bbox.min_x) / float(nx)
    increment_y = (bbox.max_y - bbox.min_y) / float(ny)
    rotation = 23.4

    s = surface_collection.new_regular_surface(
        name=name,
        origin_x=origin_x,
        origin_y=origin_y,
        depth=-depth,
        nx=nx,
        ny=ny,
        increment_x=increment_x,
        increment_y=increment_y,
        rotation=rotation,
    )

    assert s.origin_x == origin_x
    assert s.origin_y == origin_y
    assert s.increment_x == increment_x
    assert s.increment_y == increment_y
    assert s.rotation == rotation
    assert s.nx == nx
    assert s.ny == ny

    with pytest.raises(
        rips.RipsError, match="Failed to set property: incorrect dimensions."
    ):
        s.set_property("property_too_big", [i for i in range(nx * ny * 2)])

    # Successfully set the depth property
    s.set_property("depth", [i for i in range(nx * ny)])
    s.set_property_as_depth("depth")

    # Change the dimensions of the surface: should invalidate the properties
    s.nx = 30
    s.update()


def test_surface_settings_in_view(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]
    folder = surface_collection.add_folder(folder_name="View surfaces")
    surface = folder.new_regular_surface(name="View surface", nx=2, ny=2)
    surface.set_property("Property A", [1.0, 2.0, 3.0, 4.0])
    surface.set_property("Property B", [5.0, 6.0, 7.0, 8.0])

    first_view = case.create_view()
    second_view = case.create_view()

    assert first_view.set_surface_visible(surface=surface, visible=False) is None
    assert second_view.set_surface_visible(surface=surface, visible=True) is None
    assert (
        first_view.set_surface_property(surface=surface, property_name="Property B")
        is None
    )

    with pytest.raises(rips.RipsError, match="not available for surface"):
        first_view.set_surface_property(surface=surface, property_name="Missing")

    with pytest.raises(rips.RipsError, match="Surface is null"):
        first_view.set_surface_visible(surface=None, visible=True)


def test_get_property(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    nx = 5
    ny = 4
    s = surface_collection.new_regular_surface(nx=nx, ny=ny)

    input_values = [float(i) for i in range(nx * ny)]
    s.set_property("test_prop", input_values)

    output_values = s.get_property("test_prop")
    assert len(output_values) == nx * ny
    for i in range(len(input_values)):
        assert abs(output_values[i] - input_values[i]) < 1e-6

    # Getting a non-existent property should raise an error
    with pytest.raises(rips.RipsError, match="not found"):
        s.get_property("non_existent_property")


def test_available_properties(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    nx = 5
    ny = 4
    s = surface_collection.new_regular_surface(nx=nx, ny=ny)

    # Empty property list initially
    props = s.available_properties()
    assert props == []

    # Property names after setting one property
    s.set_property("prop_a", [float(i) for i in range(nx * ny)])
    props = s.available_properties()
    assert "prop_a" in props
    assert len(props) == 1

    # Property names persist after setting multiple properties
    s.set_property("prop_b", [float(i) for i in range(nx * ny)])
    props = s.available_properties()
    assert len(props) == 2
    assert "prop_a" in props
    assert "prop_b" in props


def test_create_regular_surface_invalid_values(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    c = rips_instance.project.load_case(path=case_path)
    assert len(c.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    with pytest.raises(rips.RipsError, match="Invalid nx"):
        surface_collection.new_regular_surface(nx=-1)

    with pytest.raises(rips.RipsError, match="Invalid ny"):
        surface_collection.new_regular_surface(ny=-1)

    with pytest.raises(rips.RipsError, match="Invalid increment X. Must be positive."):
        surface_collection.new_regular_surface(increment_x=-1.0)

    with pytest.raises(rips.RipsError, match="Invalid increment Y. Must be positive."):
        surface_collection.new_regular_surface(increment_y=-1.0)

    with pytest.raises(rips.RipsError, match="Invalid rotation."):
        surface_collection.new_regular_surface(rotation=-1.0)


def test_surface_name_uniqueness(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    surface_collection.new_regular_surface(name="Top")

    # The default policy is to fail on a name already used in the same folder
    with pytest.raises(rips.RipsError, match="already exists"):
        surface_collection.new_regular_surface(name="Top")

    renamed = surface_collection.new_regular_surface(
        name="Top", on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME
    )
    assert renamed.surface_user_description == "Top_1"

    # Names are case sensitive, so this is not a conflict
    other_case = surface_collection.new_regular_surface(name="top")
    assert other_case.surface_user_description == "top"

    overwritten = surface_collection.new_regular_surface(
        name="Top", on_name_conflict=rips.NameConflictPolicy.OVERWRITE
    )
    assert overwritten.surface_user_description == "Top"

    names = [s.surface_user_description for s in surface_collection.surfaces_field()]
    assert names.count("Top") == 1

    # A different folder is a separate namespace
    folder = surface_collection.add_folder(folder_name="Folder A")
    in_folder = folder.new_regular_surface(name="Top")
    assert in_folder.surface_user_description == "Top"


def test_create_triangulated_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    s = surface_collection.new_triangulated_surface(name="Top Brent")
    assert isinstance(s, rips.TriangulatedSurface)
    assert s.surface_user_description == "Top Brent"

    vertices = [
        [0.0, 0.0, 1000.0],
        [100.0, 0.0, 1010.0],
        [0.0, 100.0, 1020.0],
        [100.0, 100.0, 1030.0],
    ]
    s.set_geometry(vertices, [[0, 1, 2], [1, 3, 2]])
    assert s.available_properties() == []

    input_values = [0.1, 0.2, 0.3, 0.4]
    s.set_property("porosity", input_values)
    assert s.available_properties() == ["porosity"]

    output_values = s.get_property("porosity")
    assert len(output_values) == len(input_values)
    for i in range(len(input_values)):
        assert abs(output_values[i] - input_values[i]) < 1e-6

    # Flat lists are accepted, and a new geometry removes the properties
    s.set_geometry([0, 0, 1000, 100, 0, 1000, 0, 100, 1000], [0, 1, 2])
    assert s.available_properties() == []

    with pytest.raises(rips.RipsError, match="not found"):
        s.get_property("porosity")


def test_triangulated_surface_invalid_values(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]
    s = surface_collection.new_triangulated_surface(name="Invalid")

    vertices = [[0.0, 0.0, 1000.0], [100.0, 0.0, 1000.0], [0.0, 100.0, 1000.0]]

    with pytest.raises(rips.RipsError, match="Invalid vertices"):
        s.set_geometry([], [[0, 1, 2]])

    with pytest.raises(rips.RipsError, match="Invalid vertices"):
        s.set_geometry([0.0, 0.0], [[0, 1, 2]])

    with pytest.raises(rips.RipsError, match="Invalid triangles"):
        s.set_geometry(vertices, [0, 1])

    with pytest.raises(rips.RipsError, match="Invalid triangle index 3"):
        s.set_geometry(vertices, [[0, 1, 3]])

    s.set_geometry(vertices, [[0, 1, 2]])
    with pytest.raises(rips.RipsError, match="incorrect dimensions"):
        s.set_property("too_short", [1.0, 2.0])

    # The property name is written space separated to the surface file
    with pytest.raises(rips.RipsError, match="Invalid property name"):
        s.set_property("net to gross", [1.0, 2.0, 3.0])


def test_triangulated_surface_name_uniqueness(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    surface_collection.new_triangulated_surface(name="Top")
    with pytest.raises(rips.RipsError, match="already exists"):
        surface_collection.new_triangulated_surface(name="Top")

    renamed = surface_collection.new_triangulated_surface(
        name="Top", on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME
    )
    assert renamed.surface_user_description == "Top_1"


def test_triangulated_surface_persistence(rips_instance, initialize_test, tmp_path):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    project = rips_instance.project
    project.load_case(path=case_path)

    surface_collection = project.descendants(rips.SurfaceCollection)[0]
    s = surface_collection.new_triangulated_surface(name="Top Brent")

    # Coordinates that float32 holds exactly, so the file can be compared as text
    vertices = [
        [456333.25, 7319649.5, 2494.125],
        [456433.25, 7319649.5, 2495.125],
        [456333.25, 7319749.5, 2496.125],
        [456433.25, 7319749.5, 2497.125],
    ]
    s.set_geometry(vertices, [[0, 1, 2], [1, 3, 2]])
    s.set_property("porosity", [0.25, 0.5, 0.75, 1.0])

    # tmp_path, as ResInsight keeps the project backup database open on Windows
    project_file = tmp_path / "persisted.rsp"
    project.save(project_file.as_posix())

    # Written to a project specific folder, with a file id that keeps names apart
    surface_files = list((tmp_path / "persisted_surfaces").glob("Top_Brent_*.ts"))
    assert len(surface_files) == 1
    written = surface_files[0].read_text()
    assert "PROPERTIES porosity" in written
    assert "PVRTX 1 456333.25 7319649.5 2494.125 0.25" in written
    assert "TRGL  2 4 3" in written

    project.close()
    reopened = rips_instance.project.open(project_file.as_posix())

    surface = reopened.surface_by_name("Top Brent", rips.TriangulatedSurface)
    assert surface is not None
    assert surface.available_properties() == ["porosity"]
    assert surface.get_property("porosity") == [0.25, 0.5, 0.75, 1.0]

    # Saving the reopened project writes the same file name, geometry and values again
    copy_folder = tmp_path / "copy"
    copy_folder.mkdir()
    reopened.save((copy_folder / "persisted.rsp").as_posix())
    copied_file = copy_folder / "persisted_surfaces" / surface_files[0].name
    assert copied_file.read_text() == written


def test_triangulated_surface_persistence_name_clash(
    rips_instance, initialize_test, tmp_path
):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    project = rips_instance.project
    project.load_case(path=case_path)

    surface_collection = project.descendants(rips.SurfaceCollection)[0]
    vertices = [[0.0, 0.0, 1000.0], [100.0, 0.0, 1000.0], [0.0, 100.0, 1000.0]]

    # Both names sanitize to Top_Brent, and a folder does not change the file name
    folder = surface_collection.add_folder(folder_name="Brent")
    # Values that float32 holds exactly
    names_and_values = [
        (surface_collection, "Top Brent", [0.125, 0.25, 0.375]),
        (surface_collection, "Top_Brent", [0.5, 0.625, 0.75]),
        (folder, "Top Brent", [0.875, 1.0, 1.125]),
    ]
    for collection, name, values in names_and_values:
        s = collection.new_triangulated_surface(name=name)
        s.set_geometry(vertices, [[0, 1, 2]])
        s.set_property("porosity", values)

    project_file = tmp_path / "clash.rsp"
    project.save(project_file.as_posix())
    assert len(list((tmp_path / "clash_surfaces").glob("Top_Brent_*.ts"))) == 3

    project.close()
    reopened = rips_instance.project.open(project_file.as_posix())
    surfaces = reopened.descendants(rips.TriangulatedSurface)
    assert sorted(s.get_property("porosity") for s in surfaces) == [
        [0.125, 0.25, 0.375],
        [0.5, 0.625, 0.75],
        [0.875, 1.0, 1.125],
    ]


def test_triangulated_surface_save_failure(rips_instance, initialize_test, tmp_path):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    project = rips_instance.project
    project.load_case(path=case_path)

    surface_collection = project.descendants(rips.SurfaceCollection)[0]
    s = surface_collection.new_triangulated_surface(name="Top Brent")
    s.set_geometry(
        [[0.0, 0.0, 1000.0], [100.0, 0.0, 1000.0], [0.0, 100.0, 1000.0]], [[0, 1, 2]]
    )

    # A file where the surface folder should go makes the surface write fail, and the save must fail with it
    (tmp_path / "blocked_surfaces").write_text("not a folder")
    with pytest.raises(rips.RipsError, match="Could not write surface 'Top Brent'"):
        project.save((tmp_path / "blocked.rsp").as_posix())
    assert not (tmp_path / "blocked.rsp").exists()


def test_import_surface_errors(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    with pytest.raises(rips.RipsError, match="File name is empty"):
        surface_collection.import_surface(file_name="")

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        missing_path = Path(tmpdirname, "missing.irap").as_posix()
        with pytest.raises(rips.RipsError, match="missing.irap"):
            surface_collection.import_surface(file_name=missing_path)

        corrupt_path = Path(tmpdirname, "corrupt.irap")
        corrupt_path.write_text("this is not a surface file")
        with pytest.raises(
            rips.RipsError, match="Could not import surface from file .*corrupt.irap"
        ):
            surface_collection.import_surface(file_name=corrupt_path.as_posix())

        surface_path = Path(tmpdirname, "mysurface.ts")
        surface_collection.new_surface(case, 5).export_to_file(surface_path.as_posix())
        valid_content = surface_path.read_bytes()
        imported = surface_collection.import_surface(file_name=surface_path.as_posix())
        assert imported.surface_user_description == "mysurface.ts"

        def surface_names():
            return [
                s.surface_user_description for s in surface_collection.surfaces_field()
            ]

        # A failed overwrite must keep the existing surface
        surface_path.unlink()
        with pytest.raises(rips.RipsError, match="mysurface.ts"):
            surface_collection.import_surface(
                file_name=surface_path.as_posix(),
                on_name_conflict=rips.NameConflictPolicy.OVERWRITE,
            )
        assert surface_names().count("mysurface.ts") == 1

        surface_path.write_text("this is not a surface file")
        with pytest.raises(rips.RipsError, match="mysurface.ts"):
            surface_collection.import_surface(
                file_name=surface_path.as_posix(),
                on_name_conflict=rips.NameConflictPolicy.OVERWRITE,
            )
        assert surface_names().count("mysurface.ts") == 1

        # A successful overwrite replaces the surface and keeps the name
        surface_path.write_bytes(valid_content)
        overwritten = surface_collection.import_surface(
            file_name=surface_path.as_posix(),
            on_name_conflict=rips.NameConflictPolicy.OVERWRITE,
        )
        assert overwritten.surface_user_description == "mysurface.ts"
        assert surface_names().count("mysurface.ts") == 1

        renamed = surface_collection.import_surface(
            file_name=surface_path.as_posix(),
            on_name_conflict=rips.NameConflictPolicy.AUTO_RENAME,
        )
        assert renamed.surface_user_description == "mysurface.ts_1"


def test_list_surfaces(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    root = rips_instance.project.surface_folder()
    assert root is not None
    assert len(rips_instance.project.surfaces()) == 0

    root.new_regular_surface(name="Top")
    grid_surface = root.new_surface(case, 5)
    folder = root.add_folder(folder_name="Folder A")
    folder.new_regular_surface(name="Top")

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        path = Path(tmpdirname, "k_layer.ts")
        grid_surface.export_to_file(path.as_posix())
        file_surface = folder.import_surface(path.as_posix())
        assert file_surface is not None

    # Non-recursive on a folder, recursive from the project
    assert len(root.surfaces()) == 2
    assert len(root.surfaces(recursive=True)) == 4
    assert len(folder.surfaces()) == 2
    assert len(rips_instance.project.surfaces()) == 4

    # Filter by class; rips.Surface is the file surface class
    assert len(rips_instance.project.surfaces(rips.RegularSurface)) == 2
    assert len(rips_instance.project.surfaces(rips.GridCaseSurface)) == 1
    assert len(rips_instance.project.surfaces(rips.Surface)) == 1
    assert len(root.surfaces(rips.Surface)) == 0
    assert len(folder.surfaces(rips.Surface)) == 1
    assert all(
        isinstance(s, rips.RegularSurface)
        for s in rips_instance.project.surfaces(rips.RegularSurface)
    )

    with pytest.raises(rips.RipsError, match="surface class"):
        rips_instance.project.surfaces(rips.SurfaceCollection)


def test_surface_by_name(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    root = rips_instance.project.surface_folder()
    folder = root.add_folder(folder_name="Folder A")
    root_top = root.new_regular_surface(name="Top")
    folder_top = folder.new_regular_surface(name="Top")
    folder.new_regular_surface(name="Base")

    # First match is the one in the top folder; a folder argument restricts the search
    assert rips_instance.project.surface_by_name("Top").address() == root_top.address()
    assert (
        rips_instance.project.surface_by_name("Top", folder=folder).address()
        == folder_top.address()
    )
    assert rips_instance.project.surface_by_name("Base", folder=root) is None
    assert rips_instance.project.surface_by_name("Base") is not None
    assert rips_instance.project.surface_by_name("Missing") is None
    assert rips_instance.project.surface_by_name("Top", rips.GridCaseSurface) is None

    # The displayed name gets the depth offset appended, the match does not
    root_top.depth_offset = 10.0
    root_top.update()
    assert rips_instance.project.surface_by_name("Top").address() == root_top.address()


def test_surface_folder_lookup(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    root = rips_instance.project.surface_folder()
    assert (
        root.address()
        == rips_instance.project.descendants(rips.SurfaceCollection)[0].address()
    )

    folder = root.add_folder(folder_name="Folder A")
    assert (
        rips_instance.project.surface_folder("Folder A").address() == folder.address()
    )
    assert rips_instance.project.surface_folder("Missing") is None


def test_add_folder_returns_surface_collection(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)
    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    folder = surface_collection.add_folder(folder_name="Folder A")
    assert type(folder) is rips.SurfaceCollection
    assert folder.collection_name() == "Folder A"
    assert folder.surface_user_description == "Folder A"

    # The generated classes declare the concrete folder type on each collection class
    assert (
        typing.get_type_hints(rips.SurfaceCollection.add_folder)["return"]
        is rips.SurfaceCollection
    )
    assert (
        typing.get_type_hints(rips.EnsembleSurface.add_folder)["return"]
        is rips.SurfaceCollection
    )
    assert (
        typing.get_type_hints(rips.PdmNestedCollectionBase.add_folder)["return"]
        is rips.PdmNestedCollectionBase
    )
