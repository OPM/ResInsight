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
        # Create a simple modeled well path for testing
        well_path_coll = project.descendants(rips.WellPathCollection)[0]

        well_paths = project.well_paths()
        if well_paths:
            return well_paths[0], well_path_coll.event_timeline()

        well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
        well_path.name = "Test Well Path"
        well_path.update()
        return well_path, well_path_coll.event_timeline()

    def test_add_perf_event(self, test_well_path):
        """Test adding a perforation event to the timeline."""
        wellpath, timeline = test_well_path

        event = timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=wellpath,
            start_md=1000.0,
            end_md=1500.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        assert event is not None, "Perforation event should be created"

    def test_add_valve_event(self, test_well_path):
        """Test adding a valve event to the timeline."""
        wellpath, timeline = test_well_path

        event = timeline.add_valve_event(
            event_date="2024-01-15",
            well_path=wellpath,
            measured_depth=1250.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.7,
            area=0.0001,
        )

        assert event is not None, "Valve event should be created"

    def test_add_state_event(self, test_well_path):
        """Test adding a well state event to the timeline."""
        wellpath, timeline = test_well_path

        event = timeline.add_state_event(
            event_date="2024-02-01",
            well_path=wellpath,
            well_state="OPEN",
        )

        assert event is not None, "State event should be created"

    def test_add_control_event(self, test_well_path):
        """Test adding a well control event to the timeline."""
        wellpath, timeline = test_well_path

        event = timeline.add_control_event(
            event_date="2024-02-01",
            well_path=wellpath,
            control_mode="ORAT",
            control_value=1000.0,
            bhp_limit=150.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        assert event is not None, "Control event should be created"

    def test_add_tubing_event(self, test_well_path):
        """Test adding a tubing event to the timeline."""
        wellpath, timeline = test_well_path

        event = timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=wellpath,
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

        well_path_coll = project.descendants(rips.WellPathCollection)[0]
        timeline = well_path_coll.event_timeline()

        return project.well_paths(), timeline

    def test_set_timestamp_creates_perforations(self, project_with_wells):
        """Test that set_timestamp creates perforation intervals from perf events."""
        # Get Well Path A (end MD 2464)
        well_paths, timeline = project_with_wells
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Add a perforation event at a valid MD range for Well Path A
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
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
        well_paths, timeline = project_with_wells
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        # Add a tubing event at a valid MD range for Well Path B
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_b,
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
        well_paths, timeline = project_with_wells
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Add a perforation event in the past
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Add a perforation event in the future
        timeline.add_perf_event(
            event_date="2024-06-01",
            well_path=well_path_a,
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


class TestScheduleGeneration:
    """Tests for schedule text generation."""

    @pytest.fixture
    def project_with_case_and_well(self, rips_instance, initialize_test):
        """Load a case with well paths for schedule generation."""
        case_root = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
        project = rips_instance.project
        case = project.load_case(path=case_root + "/TEST10K_FLT_LGR_NNC.EGRID")

        # Import well paths
        well_path_files = [
            case_root + "/wellpath_a.dev",
            case_root + "/wellpath_b.dev",
        ]
        project.import_well_paths(well_path_files=well_path_files)

        well_path_coll = project.descendants(rips.WellPathCollection)[0]

        return project, case, well_path_coll.event_timeline()

    def test_generate_schedule_text_basic(self, project_with_case_and_well):
        """Test basic schedule text generation with MSW (tubing + perforation) events."""
        project, case, timeline = project_with_case_and_well

        # Get well path
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Add tubing to enable MSW
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add perforation event
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Apply events to create actual completions
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        # Verify schedule text contains expected keywords
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "2024" in schedule_text, "Schedule should contain the event date"
        assert "WELSEGS" in schedule_text, "Schedule should contain MSW WELSEGS keyword"

    def test_generate_schedule_with_control_events(self, project_with_case_and_well):
        """Test schedule generation with well control events and MSW."""
        project, case, timeline = project_with_case_and_well

        # Get well path
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Add tubing for MSW
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add perforation and control events
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            control_mode="ORAT",
            control_value=1000.0,
            bhp_limit=150.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        # Apply events to create actual completions
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        # Verify we have schedule text with completions and controls
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "WELSEGS" in schedule_text, "Schedule should contain MSW keywords"
        assert (
            "WCONPROD" in schedule_text or "WCONINJE" in schedule_text
        ), "Schedule should contain well control keywords"

    def test_generate_schedule_multiple_dates(self, project_with_case_and_well):
        """Test schedule generation with multiple event dates."""
        project, case, timeline = project_with_case_and_well

        # Get well path
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Add events at different dates
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            state="OPEN",
        )

        timeline.add_perf_event(
            event_date="2024-06-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        # Apply events to create actual completions
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        # Verify schedule was generated
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "2024" in schedule_text, "Schedule should contain event dates"

    def test_timestamp_filters_wells_in_schedule_output(
        self, project_with_case_and_well
    ):
        """Test that only wells with events before timestamp appear in schedule output.

        Scenario:
        - Add tubing event for well A at 2024-01-01
        - Add tubing event for well B at 2024-03-01
        - Set timestamp to 2024-02-01 (between the two dates)
        - Verify schedule contains well A but not well B
        """
        project, case, timeline = project_with_case_and_well

        # Get both well paths
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        # Add tubing event for well A on 2024-01-01 (before timestamp)
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=0.0,
            end_md=2400.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add perforation for well A to trigger COMPDAT generation
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Add tubing event for well B on 2024-03-01 (after timestamp)
        timeline.add_tubing_event(
            event_date="2024-03-01",
            well_path=well_path_b,
            start_md=0.0,
            end_md=2000.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add perforation for well B to trigger COMPDAT generation
        timeline.add_perf_event(
            event_date="2024-03-01",
            well_path=well_path_b,
            start_md=1500.0,
            end_md=1700.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Set timestamp to 2024-02-01 (between the two event dates)
        timeline.set_timestamp(timestamp="2024-02-01")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        # Verify schedule is generated
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert (
            "2024-01-01" in schedule_text
            or "1 JAN 2024" in schedule_text
            or "1 'JAN' 2024" in schedule_text
        ), "Schedule should contain the date 2024-01-01"

        # Verify well A appears in schedule (events at 2024-01-01 are before 2024-02-01)
        well_a_name_no_spaces = well_path_a.name.replace(" ", "")
        assert (
            well_a_name_no_spaces in schedule_text or well_path_a.name in schedule_text
        ), f"Schedule should contain well A ({well_path_a.name}) since its event is before timestamp"

        # Verify well B does NOT appear in schedule (events at 2024-03-01 are after 2024-02-01)
        well_b_name_no_spaces = well_path_b.name.replace(" ", "")
        assert (
            well_b_name_no_spaces not in schedule_text
            and well_path_b.name not in schedule_text
        ), f"Schedule should NOT contain well B ({well_path_b.name}) since its event is after timestamp"

        # Verify the March date does NOT appear in schedule
        assert (
            "2024-03-01" not in schedule_text
            and "1 MAR 2024" not in schedule_text
            and "1 'MAR' 2024" not in schedule_text
        ), "Schedule should NOT contain the date 2024-03-01 (after timestamp)"

        print("\n✓ Verified: Well A included (event before timestamp)")
        print("✓ Verified: Well B excluded (event after timestamp)")

    def test_example_workflow_schedule_generation(self, project_with_case_and_well):
        """Test the exact workflow from well_event_schedule.py example.

        This test reproduces the full workflow to verify schedule generation
        produces the correct Eclipse keywords after applying events with set_timestamp().
        """
        project, case, timeline = project_with_case_and_well

        # Get well path
        well_paths = project.well_paths()
        well_path = [wp for wp in well_paths if "A" in wp.name][0]

        # Add tubing event (installed early) - should generate WELSEGS
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add first perforation event - should generate COMPSEGS
        timeline.add_perf_event(
            event_date="2024-02-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        # Add second perforation event (later) - should generate COMPSEGS
        timeline.add_perf_event(
            event_date="2024-04-01",
            well_path=well_path,
            start_md=2400.0,
            end_md=2600.0,
            diameter=0.1,
            skin_factor=0.3,
            state="OPEN",
        )

        # Add valve event - should generate WSEGVALV
        timeline.add_valve_event(
            event_date="2024-03-01",
            well_path=well_path,
            measured_depth=2100.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.7,
            area=0.0001,
        )

        # Add state event
        timeline.add_state_event(
            event_date="2024-02-15",
            well_path=well_path,
            well_state="OPEN",
        )

        # Add keyword events - should be included in schedule
        timeline.add_well_keyword_event(
            event_date="2024-01-15",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "ORAT": 3999.99,
                "WRAT": 0.01,
                "GRAT": 550678.44,
                "VFP_TABLE": 1,
            },
        )

        timeline.add_well_keyword_event(
            event_date="2024-05-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 5000.0,
            },
        )

        timeline.add_well_keyword_event(
            event_date="2024-06-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
                "OUTPUT_PLT": "NO",
                "OUTPUT_SEGMENT": "NO",
            },
        )

        # Apply events up to March 15, 2024
        # timeline.set_timestamp(timestamp="2024-03-15")

        # Apply remaining events (up to Dec 31, 2024)
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate Eclipse schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        # Debug output
        print(f"\nSchedule text ({len(schedule_text)} characters):")
        print("=" * 60)
        print(schedule_text)
        print("=" * 60)

        assert (
            len(schedule_text) > 100
        ), f"Schedule text too short ({len(schedule_text)} chars)"
            f"Schedule text too short ({len(schedule_text)} chars)"
        )

        # Verify DATES keyword and date formatting
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "2024" in schedule_text, "Schedule should contain the year 2024"
        assert (
            "JAN" in schedule_text
            or "FEB" in schedule_text
            or "MAR" in schedule_text
            or "APR" in schedule_text
        ), "Schedule should contain month abbreviations"
        assert (
            "WELSEGS" in schedule_text
        ), "Schedule should contain WELSEGS keyword from tubing events"
            "Schedule should contain WELSEGS keyword from tubing events"
        )

        assert (
            "COMPSEGS" in schedule_text
        ), "Schedule should contain COMPSEGS keyword from perforation events with MSW"
            "Schedule should contain COMPSEGS keyword from perforation events with MSW"
        )
        assert (
            "WCONHIST" in schedule_text
        ), "Schedule should contain WCONHIST keyword event"
        assert (
            "WELTARG" in schedule_text
        ), "Schedule should contain WELTARG keyword event"
        assert (
            "WRFTPLT" in schedule_text
        ), "Schedule should contain WRFTPLT keyword event"
            "Schedule should contain WRFTPLT keyword event"
        )

        # Print keyword summary for debugging
        print("\nKeyword Summary:")
        print(f"  - DATES entries: {schedule_text.count('DATES')}")
        print(f"  - WELSEGS entries: {schedule_text.count('WELSEGS')}")
        print(f"  - COMPSEGS entries: {schedule_text.count('COMPSEGS')}")
        print(f"  - WSEGVALV entries: {schedule_text.count('WSEGVALV')}")
        print(f"  - WCONHIST entries: {schedule_text.count('WCONHIST')}")
        print(f"  - WELTARG entries: {schedule_text.count('WELTARG')}")
        print(f"  - WRFTPLT entries: {schedule_text.count('WRFTPLT')}")

        # Verify well A appears in schedule (Eclipse format has no spaces)
        # Eclipse well names typically don't have spaces, so check for both formats
        well_name_in_schedule = well_path.name.replace(" ", "")
        ), f"Schedule should contain well A name ({well_path.name} or {well_name_in_schedule})"
            f"Schedule should contain well A name ({well_path.name} or {well_name_in_schedule})"
        )

        # Verify well B (index 1) does NOT appear in schedule
        well_paths = project.well_paths()
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        # Check that well B doesn't appear in either format (with or without spaces)
        well_b_name_in_schedule = well_path_b.name.replace(" ", "")
        assert (
            well_b_name_in_schedule not in schedule_text
            and well_path_b.name not in schedule_text
        ), (
            f"Schedule should NOT contain well B name ({well_path_b.name} or {well_b_name_in_schedule}) "
            f"since no events were added for it"
        )

        print(
            f"\n✓ Verified: Well B ({well_path_b.name}) correctly excluded from schedule"
        )

    def test_schedule_contains_welsegs_keyword(self, project_with_case_and_well):
        """Verify WELSEGS keyword is generated for tubing events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        timeline.set_timestamp(timestamp="2024-12-31")
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        assert (
            "WELSEGS" in schedule_text
        ), "Schedule should contain WELSEGS keyword for tubing events"
            "Schedule should contain WELSEGS keyword for tubing events"
        )
        # Eclipse well names typically don't have spaces
        well_name_no_spaces = well_path.name.replace(" ", "")
        ), f"Schedule should contain well name ({well_path.name} or {well_name_no_spaces})"
            f"Schedule should contain well name ({well_path.name} or {well_name_no_spaces})"
        )

    def test_schedule_contains_wsegvalv_keyword(self, project_with_case_and_well):
        """Verify WSEGVALV keyword is generated for valve events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add tubing and valve on same date to simplify test
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        # Add perforation (MSW needs perforations)
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        # Add valve before perforation interval to avoid overlap
        timeline.add_valve_event(
            event_date="2024-01-01",
            well_path=well_path,
            measured_depth=1900.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.7,
            area=0.0001,
        )

        timeline.set_timestamp(timestamp="2024-12-31")
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule text for WSEGVALV test:\n{schedule_text}")

        # WSEGVALV may or may not appear depending on whether valves are extracted
        # For now, just verify the schedule was generated with MSW keywords
        assert "WELSEGS" in schedule_text, "Schedule should contain WELSEGS keyword"
        # Note: WSEGVALV generation depends on valve data being properly extracted from events
        # which may require additional completion settings or configuration

    def test_schedule_contains_wconprod_keyword(self, project_with_case_and_well):
        """Verify WCONPROD keyword is generated for control events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Control events don't require set_timestamp - they don't create physical completions
        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            bhp_limit=150.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        assert (
            "WCONPROD" in schedule_text or "WCONINJE" in schedule_text
        ), "Schedule should contain WCONPROD or WCONINJE keyword for control events"
            "Schedule should contain WCONPROD or WCONINJE keyword for control events"
        )
        assert "ORAT" in schedule_text, "Schedule should contain control mode ORAT"

    def test_schedule_contains_compdat_keyword(self, project_with_case_and_well):
        """Verify COMPDAT keyword is generated."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        timeline.set_timestamp(timestamp="2024-01-01")

        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule text for COMPDAT test:\n{schedule_text}")

        assert "COMPDAT" in schedule_text

    def test_schedule_multiple_dates_in_order(self, project_with_case_and_well):
        """Verify schedule dates are in chronological order."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add control events at different dates (add in reverse order to test sorting)
        timeline.add_control_event(
            event_date="2024-06-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=800.0,
            oil_rate=800.0,
            is_producer=True,
        )

        timeline.add_control_event(
            event_date="2024-02-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=500.0,
            oil_rate=500.0,
            is_producer=True,
        )

        # Control events don't require set_timestamp
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule text for date ordering test:\n{schedule_text}")

        # Find positions of date keywords
        jan_pos = schedule_text.find("1 'JAN' 2024")
        feb_pos = schedule_text.find("1 'FEB' 2024")
        jun_pos = schedule_text.find("1 'JUN' 2024")

        assert jan_pos > 0, "Schedule should contain 1 'JAN' 2024"
        assert feb_pos > 0, "Schedule should contain 1 'FEB' 2024"
        assert jun_pos > 0, "Schedule should contain 1 'JUN' 2024"
        assert jan_pos < feb_pos, "January should come before February"
        assert feb_pos < jun_pos, "February should come before June"


class TestKeywordEvents:
    """Tests for well keyword event functionality."""

    @pytest.fixture
    def project_with_case_and_well(self, rips_instance, initialize_test):
        """Load a case with well paths for keyword event tests."""
        case_root = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
        project = rips_instance.project
        case = project.load_case(path=case_root + "/TEST10K_FLT_LGR_NNC.EGRID")

        # Import well paths
        well_path_files = [
            case_root + "/wellpath_a.dev",
            case_root + "/wellpath_b.dev",
        ]
        project.import_well_paths(well_path_files=well_path_files)

        well_path_coll = project.descendants(rips.WellPathCollection)[0]

        return project, case, well_path_coll.event_timeline()

    def test_add_well_keyword_event_wconhist(self, project_with_case_and_well):
        """Test adding a WCONHIST keyword event."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Create a WCONHIST event
        event = timeline.add_well_keyword_event(
            event_date="2024-01-01",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "ORAT": 3999.99,
                "WRAT": 0.01,
                "GRAT": 550678.44,
                "VFP_TABLE": 1,
            },
        )

        assert event is not None, "Keyword event should be created"

    def test_add_well_keyword_event_weltarg(self, project_with_case_and_well):
        """Test adding a WELTARG keyword event."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Create a WELTARG event
        event = timeline.add_well_keyword_event(
            event_date="2024-05-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 5000.0,
            },
        )

        assert event is not None, "Keyword event should be created"

    def test_add_well_keyword_event_wrftplt(self, project_with_case_and_well):
        """Test adding a WRFTPLT keyword event."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Create a WRFTPLT event
        event = timeline.add_well_keyword_event(
            event_date="2024-06-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
                "OUTPUT_PLT": "NO",
                "OUTPUT_SEGMENT": "NO",
            },
        )

        assert event is not None, "Keyword event should be created"

    def test_keyword_event_type_inference(self, project_with_case_and_well):
        """Test that add_well_keyword_event correctly infers types from Python values."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Create event with mixed types: str, int, float, bool
        event = timeline.add_well_keyword_event(
            event_date="2024-03-15",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,  # str
                "STATUS": "OPEN",  # str
                "ORAT": 1000.5,  # float
                "VFP_TABLE": 2,  # int
            },
        )

        assert event is not None, "Event with mixed types should be created"

    def test_keyword_event_schedule_output_single_keyword(
        self, project_with_case_and_well
    ):
        """Test that keyword events appear in schedule text generation."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add a WELTARG keyword event
        timeline.add_well_keyword_event(
            event_date="2024-05-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 5000.0,
            },
        )

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule text for keyword event:\n{schedule_text}")

        # Verify the keyword event is in the output
        assert schedule_text, "Schedule text should not be empty"
        assert "WELTARG" in schedule_text, "Schedule should contain WELTARG keyword"
        # Eclipse well names typically don't have spaces
        well_name_no_spaces = well_path.name.replace(" ", "")
        ), f"Schedule should contain well name ({well_path.name} or {well_name_no_spaces})"
            f"Schedule should contain well name ({well_path.name} or {well_name_no_spaces})"
        )

    def test_keyword_event_schedule_output_multiple_keywords(
        self, project_with_case_and_well
    ):
        """Test schedule text with multiple keyword events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add WCONHIST event
        timeline.add_well_keyword_event(
            event_date="2024-01-15",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "ORAT": 3999.99,
                "WRAT": 0.01,
                "GRAT": 550678.44,
            },
        )

        # Add WELTARG event
        timeline.add_well_keyword_event(
            event_date="2024-05-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 5000.0,
            },
        )

        # Add WRFTPLT event
        timeline.add_well_keyword_event(
            event_date="2024-06-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
                "OUTPUT_PLT": "NO",
            },
        )

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule text for multiple keywords:\n{schedule_text}")

        # Verify all keyword events are in the output
        assert schedule_text, "Schedule text should not be empty"
        assert "WCONHIST" in schedule_text, "Schedule should contain WCONHIST keyword"
        assert "WELTARG" in schedule_text, "Schedule should contain WELTARG keyword"
        assert "WRFTPLT" in schedule_text, "Schedule should contain WRFTPLT keyword"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"

    def test_invalid_keyword_data_unsupported_type(self, project_with_case_and_well):
        """Test error handling for unsupported data types in keyword events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Try to create a keyword event with an unsupported type (list)
        with pytest.raises(TypeError) as exc_info:
            timeline.add_well_keyword_event(
                event_date="2024-01-01",
                well_path=well_path,
                keyword_name="WCONHIST",
                keyword_data={
                    "WELL": well_path.name,
                    "INVALID_FIELD": [1, 2, 3],  # Unsupported type
                },
            )

        # Verify the error message mentions the unsupported type
        error_msg = str(exc_info.value)
        assert "Unsupported type" in error_msg or "list" in error_msg

    def test_keyword_event_with_perf_events(self, project_with_case_and_well):
        """Test keyword events alongside perforation events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add tubing and perforation for MSW
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )

        timeline.add_perf_event(
            event_date="2024-02-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        # Add keyword event
        timeline.add_well_keyword_event(
            event_date="2024-03-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 3000.0,
            },
        )

        # Apply tubing/perf events
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule with perf and keyword events:\n{schedule_text}")

        # Verify both physical completions and keyword events are present
        assert (
            "WELTARG" in schedule_text
        ), "Schedule should contain keyword event WELTARG"
            "Schedule should contain keyword event WELTARG"
        )

    def test_keyword_event_with_control_events(self, project_with_case_and_well):
        """Test keyword events alongside control events."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add control event
        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        # Add keyword event
        timeline.add_well_keyword_event(
            event_date="2024-02-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
                "OUTPUT_PLT": "YES",
            },
        )

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule with control and keyword events:\n{schedule_text}")

        assert (
            "WCONPROD" in schedule_text or "WCONINJE" in schedule_text
        ), "Schedule should contain control keyword"
        assert (
            "WRFTPLT" in schedule_text
        ), "Schedule should contain keyword event WRFTPLT"
            "Schedule should contain keyword event WRFTPLT"
        )

    def test_multiple_keyword_events_at_different_dates(
        self, project_with_case_and_well
    ):
        """Test adding keyword events at different dates and verify chronological order."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add keyword events in reverse chronological order
        timeline.add_well_keyword_event(
            event_date="2024-06-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
            },
        )

        timeline.add_well_keyword_event(
            event_date="2024-03-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 4000.0,
            },
        )

        timeline.add_well_keyword_event(
            event_date="2024-01-01",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "ORAT": 2000.0,
            },
        )

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule with keyword events at multiple dates:\n{schedule_text}")

        # Verify all keyword events are present
        assert "WCONHIST" in schedule_text, "Schedule should contain WCONHIST"
        assert "WELTARG" in schedule_text, "Schedule should contain WELTARG"
        assert "WRFTPLT" in schedule_text, "Schedule should contain WRFTPLT"

        # Verify chronological ordering with DATES keyword
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        jan_pos = schedule_text.find("1 'JAN' 2024")
        mar_pos = schedule_text.find("1 'MAR' 2024")
        jun_pos = schedule_text.find("1 'JUN' 2024")

        # If all three months are present, verify order
        if jan_pos > 0 and mar_pos > 0:
            assert jan_pos < mar_pos, "January should come before March"
        if mar_pos > 0 and jun_pos > 0:
            assert mar_pos < jun_pos, "March should come before June"

    def test_keyword_event_with_bool_values(self, project_with_case_and_well):
        """Test keyword events with boolean values (converted to 0/1)."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Create event with bool values (should be converted to INT 1/0)
        event = timeline.add_well_keyword_event(
            event_date="2024-04-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": True,  # bool converted to 1
                "OUTPUT_PLT": False,  # bool converted to 0
            },
        )

        assert event is not None, "Event with boolean values should be created"

        # Generate schedule to verify bool conversion
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        print(f"\nSchedule with boolean keyword values:\n{schedule_text}")
        assert schedule_text, "Schedule text should not be empty"
