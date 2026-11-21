import os
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def _camera_matrix(rips_instance, view):
    # The camera matrix is cached in the Python object, so the view is fetched again
    return list(rips_instance.project.view(view.id).camera_matrix)


def _load_view(rips_instance):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)
    return case.create_view()


@pytest.mark.parametrize(
    "kwargs",
    [
        {"azimuth": float("nan")},
        {"azimuth": float("inf")},
        {"azimuth": -1.0},
        {"azimuth": 361.0},
        {"tilt": float("nan")},
        {"tilt": -1.0},
        {"tilt": 181.0},
        {"zoom": 0.0},
        {"zoom": 1e-320},
        {"zoom": 1e6},
        {"zoom": -2.0},
        {"zoom": float("inf")},
        {"zoom": float("nan")},
    ],
)
def test_set_camera_invalid_values(rips_instance, initialize_test, kwargs):
    view = _load_view(rips_instance)
    with pytest.raises(rips.RipsError):
        view.set_camera(**kwargs)


def test_set_camera_and_view_from_need_a_viewer(rips_instance, initialize_test):
    if rips_instance.is_gui():
        pytest.skip("A viewer may exist in GUI mode")

    view = _load_view(rips_instance)
    with pytest.raises(rips.RipsError, match="no viewer"):
        view.set_camera(azimuth=200, tilt=60, zoom=1.0)
    with pytest.raises(rips.RipsError, match="no viewer"):
        view.set_view_from(rips.ViewFrom.TOP)
    with pytest.raises(rips.RipsError, match="no viewer"):
        view.zoom_all()


def test_fly_to_object_rejects_invalid_objects(rips_instance, initialize_test):
    view = _load_view(rips_instance)
    case = rips_instance.project.cases()[0]

    with pytest.raises(rips.RipsError):
        view.fly_to_object(None)
    with pytest.raises(rips.RipsError, match="bounding box"):
        view.fly_to_object(case)


def test_fly_to_object_moves_camera(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    view = _load_view(rips_instance)
    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]
    surface = surface_collection.new_regular_surface(name="S1", nx=2, ny=2)

    before = _camera_matrix(rips_instance, view)
    try:
        view.fly_to_object(surface)
    except rips.RipsError as error:
        if "no viewer" in str(error):
            pytest.skip("The view has no viewer (e.g. offscreen Qt platform)")
        raise
    assert _camera_matrix(rips_instance, view) != before


def _eye_position(rips_instance, view):
    # The camera matrix is a row-major view matrix: eye = -transpose(R) * t
    m = _camera_matrix(rips_instance, view)
    t = (m[3], m[7], m[11])
    return [-sum(m[4 * row + col] * t[row] for row in range(3)) for col in range(3)]


def test_set_camera_keeps_distance_constant_when_rotating(
    rips_instance, initialize_test
):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    view = _load_view(rips_instance)
    eyes = []
    try:
        view.zoom_all()
        for azimuth in (0, 90, 180, 270):
            view.set_camera(azimuth=azimuth, tilt=60, zoom=1.0)
            eyes.append(_eye_position(rips_instance, view))
    except rips.RipsError as error:
        if "no viewer" in str(error):
            pytest.skip("The view has no viewer (e.g. offscreen Qt platform)")
        raise

    # Eyes on a circle around the model center share the same distance to the circle center
    center = [sum(eye[i] for eye in eyes) / len(eyes) for i in range(3)]
    distances = [
        sum((eye[i] - center[i]) ** 2 for i in range(3)) ** 0.5 for eye in eyes
    ]
    assert distances[0] > 0
    assert distances == pytest.approx([distances[0]] * 4, rel=1e-6)


def test_set_camera_changes_camera(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    view = _load_view(rips_instance)
    try:
        view.set_camera(azimuth=200, tilt=60, zoom=1.0)
    except rips.RipsError as error:
        if "no viewer" in str(error):
            pytest.skip("The view has no viewer (e.g. offscreen Qt platform)")
        raise

    oblique = _camera_matrix(rips_instance, view)

    view.set_camera(azimuth=200, tilt=60, zoom=2.0)
    zoomed = _camera_matrix(rips_instance, view)
    assert zoomed != oblique

    view.set_camera(azimuth=20, tilt=60, zoom=1.0)
    assert _camera_matrix(rips_instance, view) != oblique

    view.set_view_from(rips.ViewFrom.NORTH)
    north = _camera_matrix(rips_instance, view)
    view.set_view_from(rips.ViewFrom.TOP)
    assert _camera_matrix(rips_instance, view) != north

    view.zoom_all()
