"""
Tests for well events functionality.

These tests verify the well event timeline and event management functionality.
"""

import os
import sys
import pytest
from datetime import date

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import rips
import dataroot

from rips.well_events import parse_well_events_config


@pytest.fixture
def project(rips_instance):
    """Get the current project."""
    return rips_instance.project


class TestWellEventTimeline:
    """Tests for WellEventTimeline functionality."""

    @pytest.fixture
    def test_well_path(self, project):
        """Create or get a test well path."""
        # Try to find existing well paths
        well_paths = project.well_paths()
        if well_paths:
            return well_paths[0]

        # Create a simple modeled well path for testing
        well_path_coll = project.descendants(rips.WellPathCollection)[0]
        well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
        well_path.name = "Test Well Path"
        well_path.update()
        return well_path

    def test_get_event_timeline(self, test_well_path):
        """Test getting an event timeline from a well path."""
        timeline = test_well_path.event_timeline()
        assert timeline is not None, "Event timeline should be accessible"

    def test_add_perf_event(self, test_well_path):
        """Test adding a perforation event to the timeline."""
        timeline = test_well_path.event_timeline()

        event = timeline.add_perf_event(
            event_date="2024-01-01",
            well_name="Test Well Path",
            start_md=1000.0,
            end_md=1500.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        assert event is not None, "Perforation event should be created"

    def test_add_valve_event(self, test_well_path):
        """Test adding a valve event to the timeline."""
        timeline = test_well_path.event_timeline()

        event = timeline.add_valve_event(
            event_date="2024-01-15",
            well_name="Test Well Path",
            measured_depth=1250.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.7,
            area=0.0001,
        )

        assert event is not None, "Valve event should be created"

    def test_add_state_event(self, test_well_path):
        """Test adding a well state event to the timeline."""
        timeline = test_well_path.event_timeline()

        event = timeline.add_state_event(
            event_date="2024-02-01",
            well_name="Test Well Path",
            well_state="OPEN",
        )

        assert event is not None, "State event should be created"

    def test_add_control_event(self, test_well_path):
        """Test adding a well control event to the timeline."""
        timeline = test_well_path.event_timeline()

        event = timeline.add_control_event(
            event_date="2024-02-01",
            well_name="Test Well Path",
            control_mode="ORAT",
            control_value=1000.0,
            bhp_limit=150.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        assert event is not None, "Control event should be created"

    def test_add_tubing_event(self, test_well_path):
        """Test adding a tubing event to the timeline."""
        timeline = test_well_path.event_timeline()

        event = timeline.add_tubing_event(
            event_date="2024-01-01",
            well_name="Test Well Path",
            start_md=500.0,
            end_md=1000.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        assert event is not None, "Tubing event should be created"


class TestWellEventScheduleApplication:
    """Tests for applying well events to create completions."""

    @pytest.fixture
    def project_with_wells(self, rips_instance, initialize_test):
        """Load TEST10K case with well paths from .dev files."""

        case_root = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
        project = rips_instance.project
        project.load_case(path=case_root + "/TEST10K_FLT_LGR_NNC.EGRID")

        # Import well paths from .dev files
        well_path_files = [
            case_root + "/wellpath_a.dev",
            case_root + "/wellpath_b.dev",
        ]
        project.import_well_paths(well_path_files=well_path_files)

        return project

    def test_set_timestamp_creates_perforations(self, project_with_wells):
        """Test that set_timestamp creates perforation intervals from perf events."""
        # Get Well Path A (end MD 2464)
        well_paths = project_with_wells.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline = well_path_a.event_timeline()

        # Add a perforation event at a valid MD range for Well Path A
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_name=well_path_a.name,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Apply events up to the date
        timeline.set_timestamp(timestamp="2024-01-15")

        # Verify perforation was created
        perforation_coll = well_path_a.completions().perforations()
        perforations = perforation_coll.perforations()
        assert len(perforations) > 0, "Perforation should be created from event"
        # Verify the perforation has the correct MD range
        perf = perforations[0]
        assert (
            abs(perf.start_measured_depth - 2000.0) < 1.0
        ), "Perforation start MD should match"
        assert (
            abs(perf.end_measured_depth - 2200.0) < 1.0
        ), "Perforation end MD should match"

    def test_set_timestamp_creates_tubing_intervals(self, project_with_wells):
        """Test that set_timestamp creates tubing intervals from tubing events."""
        # Get Well Path B (end MD 2112)
        well_paths = project_with_wells.well_paths()
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        timeline = well_path_b.event_timeline()

        # Add a tubing event at a valid MD range for Well Path B
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_name=well_path_b.name,
            start_md=1000.0,
            end_md=2000.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Apply events up to the date
        timeline.set_timestamp(timestamp="2024-01-15")

        # Verify MSW settings were updated
        msw_settings = well_path_b.msw_settings()
        assert msw_settings is not None, "MSW settings should be available"
        # Check that diameter roughness mode was set to intervals
        assert (
            msw_settings.diameter_roughness_mode == "Intervals"
        ), "Diameter roughness mode should be set to Intervals"

    def test_set_timestamp_ignores_future_events(self, project_with_wells):
        """Test that events after the timestamp are not applied."""
        # Get Well Path A (end MD 2464)
        well_paths = project_with_wells.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline = well_path_a.event_timeline()

        # Add a perforation event in the past
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_name=well_path_a.name,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Add a perforation event in the future
        timeline.add_perf_event(
            event_date="2024-06-01",
            well_name=well_path_a.name,
            start_md=2200.0,
            end_md=2400.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Apply events only up to March
        timeline.set_timestamp(timestamp="2024-03-01")

        # Verify only one perforation was created (the one before March)
        perforation_coll = well_path_a.completions().perforations()
        perforations = perforation_coll.perforations()
        # Count perforations with start_measured_depth around 1800 (the past event)
        past_perfs = [
            p for p in perforations if abs(p.start_measured_depth - 1800.0) < 10
        ]
        future_perfs = [
            p for p in perforations if abs(p.start_measured_depth - 2200.0) < 10
        ]
        assert len(past_perfs) > 0, "Past perforation event should be applied"
        assert len(future_perfs) == 0, "Future perforation event should not be applied"


class TestWellEventsYamlConfig:
    """Tests for YAML configuration parsing."""

    def test_parse_yaml_config(self, tmp_path):
        """Test parsing a YAML configuration file."""
        # Create a test YAML file
        yaml_content = """
A-1H:
  2024-01-01:
    PERF:
      - START_MD: 1000
        END_MD: 1500
        DIAMETER: 0.1
        STATE: OPEN
    WSTATE: OPEN
    WCONTROL:
      MODE: ORAT
      VALUE: 1000
      BHP_LIMIT: 150
"""
        config_file = tmp_path / "test_events.yaml"
        config_file.write_text(yaml_content)

        config = parse_well_events_config(str(config_file))
        assert "A-1H" in config
        # PyYAML parses date strings as date objects
        assert date(2024, 1, 1) in config["A-1H"] or "2024-01-01" in config["A-1H"]

        # Verify event data structure
        well_events = config["A-1H"]
        first_date = list(well_events.keys())[0]
        events = well_events[first_date]
        assert "PERF" in events
        assert "WSTATE" in events
        assert "WCONTROL" in events
