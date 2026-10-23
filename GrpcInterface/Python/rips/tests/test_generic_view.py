import sys
import os

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_create_generic_view(rips_instance, initialize_test):
    project = rips_instance.project

    view = project.create_generic_view(name="Overview")

    generic_views = project.descendants(rips.GenericView)
    assert view.address() in [v.address() for v in generic_views]


def test_generic_view_surface_and_polygon_visibility(rips_instance, initialize_test):
    project = rips_instance.project

    view = project.create_generic_view(name="Overview")

    surface_collection = project.descendants(rips.SurfaceCollection)[0]
    surface = surface_collection.new_regular_surface(name="S1", nx=2, ny=2)
    surface.set_property("Property A", [1.0, 2.0, 3.0, 4.0])

    polygon_collection = project.descendants(rips.PolygonCollection)[0]
    polygon = polygon_collection.create_polygon(
        name="P1",
        coordinates=[
            [0.0, 0.0, -1000.0],
            [100.0, 0.0, -1000.0],
            [100.0, 100.0, -1000.0],
        ],
    )

    assert view.set_surface_visible(surface=surface, visible=False) is None
    assert view.set_surface_visible(surface=surface, visible=True) is None
    assert (
        view.set_surface_property(surface=surface, property_name="Property A") is None
    )

    assert view.set_polygon_visible(polygon=polygon, visible=False) is None
    assert view.set_polygon_visible(polygon=polygon, visible=True) is None


def test_generic_view_visibility_error_paths(rips_instance, initialize_test):
    project = rips_instance.project

    view = project.create_generic_view()

    with pytest.raises(rips.RipsError, match="Surface is null"):
        view.set_surface_visible(surface=None, visible=True)

    with pytest.raises(rips.RipsError, match="Polygon is null"):
        view.set_polygon_visible(polygon=None, visible=True)
