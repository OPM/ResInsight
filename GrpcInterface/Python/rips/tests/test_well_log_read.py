import pytest


@pytest.fixture
def project(rips_instance):
    """Create a test project with a well path."""
    coordinates = [
        [1000, 2000, 0],
        [1000, 2000, 1000],
        [1100, 2100, 2000],
        [1200, 2200, 3000],
    ]

    well_path = (
        rips_instance.project.well_path_collection().import_well_path_from_points(
            name="TestWell", coordinates=coordinates
        )
    )

    return rips_instance.project, well_path


def _approx(expected):
    return pytest.approx(expected, rel=1e-5, abs=1e-5)


def test_well_logs_empty(project):
    """A fresh well path has no well logs."""
    _, well_path = project
    assert well_path.well_logs() == []


def test_well_logs_lists_added_log(project):
    """Adding a well log makes it visible via well_logs()."""
    _, well_path = project
    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    well_path.add_well_log(
        name="Log1", measured_depth=measured_depth, channel_data=channel_data
    )

    logs = well_path.well_logs()
    assert len(logs) == 1
    assert logs[0].name == "Log1"


def test_channel_names(project):
    """channel_names() returns exactly the channels that were added."""
    _, well_path = project
    channel_data = {"porosity": [0.1, 0.2, 0.3], "gamma_ray": [70.0, 75.0, 80.0]}
    well_path.add_well_log(
        name="Log1", measured_depth=[1000, 1500, 2000], channel_data=channel_data
    )

    log = well_path.well_logs()[0]
    assert sorted(log.channel_names()) == sorted(channel_data.keys())


def test_well_log_data_round_trip_no_tvd(project):
    """Write a well log and read it back; measured depth and channels match."""
    _, well_path = project
    measured_depth = [1000.0, 1500.0, 2000.0, 2500.0, 3000.0]
    channel_data = {
        "porosity": [0.10, 0.15, 0.20, 0.18, 0.12],
        "permeability": [10.5, 25.3, 45.8, 35.2, 18.7],
    }
    well_path.add_well_log(
        name="RoundTrip", measured_depth=measured_depth, channel_data=channel_data
    )

    data = well_path.well_logs()[0].well_log_data()

    assert set(data.keys()) == {"measured_depth", "porosity", "permeability"}
    assert data["measured_depth"] == _approx(measured_depth)
    assert data["porosity"] == _approx(channel_data["porosity"])
    assert data["permeability"] == _approx(channel_data["permeability"])
    assert "tvd_msl" not in data
    assert "tvd_rkb" not in data


def test_well_log_data_with_tvd_msl(project):
    """TVD MSL values are returned when the well log has them."""
    _, well_path = project
    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_msl = [980, 1480, 1980]
    well_path.add_well_log(
        name="WithMsl",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_msl=tvd_msl,
    )

    data = well_path.well_logs()[0].well_log_data()

    assert "tvd_msl" in data
    assert data["tvd_msl"] == _approx(tvd_msl)
    assert "tvd_rkb" not in data


def test_well_log_data_with_tvd_rkb(project):
    """TVD RKB values are returned when the well log has them."""
    _, well_path = project
    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_rkb = [1020, 1520, 2020]
    well_path.add_well_log(
        name="WithRkb",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_rkb=tvd_rkb,
    )

    data = well_path.well_logs()[0].well_log_data()

    assert "tvd_rkb" in data
    assert data["tvd_rkb"] == _approx(tvd_rkb)
    assert "tvd_msl" not in data


def test_well_log_data_with_both_tvd(project):
    """Both TVD MSL and TVD RKB values are returned when both are present."""
    _, well_path = project
    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_msl = [980, 1480, 1980]
    tvd_rkb = [1020, 1520, 2020]
    well_path.add_well_log(
        name="Both",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_msl=tvd_msl,
        tvd_rkb=tvd_rkb,
    )

    data = well_path.well_logs()[0].well_log_data()

    assert data["tvd_msl"] == _approx(tvd_msl)
    assert data["tvd_rkb"] == _approx(tvd_rkb)


def test_multiple_well_logs(project):
    """Each well log is readable independently after adding multiple logs."""
    _, well_path = project
    well_path.add_well_log(
        name="LogA",
        measured_depth=[1000, 1500, 2000],
        channel_data={"porosity": [0.1, 0.2, 0.3]},
    )
    well_path.add_well_log(
        name="LogB",
        measured_depth=[1200, 1700, 2200, 2700],
        channel_data={"gamma_ray": [70.0, 75.0, 80.0, 85.0]},
    )

    logs = well_path.well_logs()
    assert len(logs) == 2

    by_name = {log.name: log for log in logs}
    assert set(by_name.keys()) == {"LogA", "LogB"}

    data_a = by_name["LogA"].well_log_data()
    data_b = by_name["LogB"].well_log_data()

    assert data_a["measured_depth"] == _approx([1000, 1500, 2000])
    assert data_a["porosity"] == _approx([0.1, 0.2, 0.3])
    assert data_b["measured_depth"] == _approx([1200, 1700, 2200, 2700])
    assert data_b["gamma_ray"] == _approx([70.0, 75.0, 80.0, 85.0])


def test_well_log_single_value(project):
    """1-element arrays survive a round-trip."""
    _, well_path = project
    well_path.add_well_log(
        name="Single",
        measured_depth=[1234.5],
        channel_data={"porosity": [0.17]},
    )

    data = well_path.well_logs()[0].well_log_data()
    assert data["measured_depth"] == _approx([1234.5])
    assert data["porosity"] == _approx([0.17])
