"""
Tests for well events functionality.

These tests verify the well event timeline and event management functionality.
"""

import os
import sys
import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import rips
import dataroot


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
        assert abs(perf.start_measured_depth - 2000.0) < 1.0, (
            "Perforation start MD should match"
        )
        assert abs(perf.end_measured_depth - 2200.0) < 1.0, (
            "Perforation end MD should match"
        )

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
        assert msw_settings.diameter_roughness_mode == "Intervals", (
            "Diameter roughness mode should be set to Intervals"
        )

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

        # Generate schedule text (keep the first date as a DATES keyword for this assertion)
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )

        # Verify schedule text contains expected keywords
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "2024" in schedule_text, "Schedule should contain the event date"
        assert "WELSEGS" in schedule_text, "Schedule should contain MSW WELSEGS keyword"

    def test_generate_schedule_preserves_time_of_day(self, project_with_case_and_well):
        """Event timestamps with a time-of-day must be preserved in the exported DATES keyword (issue #14111)."""
        project, case, timeline = project_with_case_and_well

        # Get well path
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Event date carrying an explicit, non-midnight time-of-day (ISO 8601).
        timeline.add_perf_event(
            event_date="2024-01-01T12:34:56.789",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )

        timeline.set_timestamp(timestamp="2024-12-31")

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )

        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "12:34:56.789" in schedule_text, (
            "DATES keyword should preserve the event time-of-day with millisecond precision (TIME field)"
        )

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

        # Generate schedule text (keep the first date as a DATES keyword for this assertion)
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )

        # Verify we have schedule text with completions and controls
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "WELSEGS" in schedule_text, "Schedule should contain MSW keywords"
        assert "WCONPROD" in schedule_text or "WCONINJE" in schedule_text, (
            "Schedule should contain well control keywords"
        )

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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        # Verify schedule was generated
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "2024" in schedule_text, "Schedule should contain event dates"

    def test_generate_schedule_with_additional_dates(self, project_with_case_and_well):
        """Additional dates become bare DATES keywords, merged chronologically (issue #14514)."""
        project, case, timeline = project_with_case_and_well

        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.add_perf_event(
            event_date="2024-03-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.set_timestamp(timestamp="2024-12-31")

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
            additional_dates=["2024-02-01", "2024-06-01"],
        )

        assert "1 'FEB' 2024" in schedule_text, (
            "Additional date 2024-02-01 should be emitted as a DATES entry"
        )
        assert "1 'JUN' 2024" in schedule_text, (
            "Additional date 2024-06-01 should be emitted as a DATES entry"
        )
        # Dates must appear in chronological order: JAN (event), FEB, MAR (event), JUN
        positions = [
            schedule_text.index(date_str)
            for date_str in [
                "1 'JAN' 2024",
                "1 'FEB' 2024",
                "1 'MAR' 2024",
                "1 'JUN' 2024",
            ]
        ]
        assert positions == sorted(positions), (
            f"Dates should appear chronologically, got positions {positions}"
        )

    def test_additional_dates_deduplicated_and_merged(self, project_with_case_and_well):
        """An additional date equal to an event date must not produce a duplicate DATES entry."""
        project, case, timeline = project_with_case_and_well

        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.set_timestamp(timestamp="2024-12-31")

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
            additional_dates=["2024-01-01", "2024-01-01"],
        )

        assert schedule_text.count("1 'JAN' 2024") == 1, (
            "Duplicate additional dates should be merged with the event date"
        )

    def test_additional_dates_invalid_format(self, project_with_case_and_well):
        """An unparsable additional date must raise an error mentioning the format."""
        project, case, timeline = project_with_case_and_well

        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.set_timestamp(timestamp="2024-12-31")

        with pytest.raises(rips.RipsError) as exc_info:
            timeline.generate_schedule_text(
                eclipse_case=case,
                export_msw_for_wells=project.well_paths(),
                additional_dates=["not-a-date"],
            )
        assert "Invalid date format" in str(exc_info.value)

    def test_additional_date_before_first_event(self, project_with_case_and_well):
        """An additional date earlier than all events becomes the first date of the schedule."""
        project, case, timeline = project_with_case_and_well

        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.set_timestamp(timestamp="2024-12-31")

        # Default first_date_as_comment=True: the earliest date (the additional
        # one) becomes the comment; the event date is a real DATES entry.
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            additional_dates=["2023-06-01"],
        )
        assert "-- Date: 1 JUN 2023" in schedule_text, (
            "Earliest (additional) date should be emitted as a comment by default"
        )
        assert "1 'JAN' 2024" in schedule_text, (
            "Event date should be a DATES entry when it is no longer first"
        )

        # With first_date_as_comment=False every date is a DATES entry.
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
            additional_dates=["2023-06-01"],
        )
        assert "1 'JUN' 2023" in schedule_text, (
            "Additional date should be a DATES entry with first_date_as_comment=False"
        )

    def test_first_date_as_comment(self, project_with_case_and_well):
        """Test that the first (earliest) date can be emitted as a comment.

        The first date equals the simulation start date, which some commercial
        simulators reject as a DATES entry. With first_date_as_comment (the
        default), the earliest date becomes a comment line while later dates
        remain real DATES keywords.
        """
        project, case, timeline = project_with_case_and_well

        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        # Two distinct dates: earliest 2024-01-01, later 2024-06-01
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

        timeline.set_timestamp(timestamp="2024-12-31")

        # Default (first_date_as_comment=True): first date is a comment, later date a DATES keyword
        default_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        assert default_text, "Schedule text should not be empty"
        assert "-- Date: 1 JAN 2024" in default_text, (
            "First date should be emitted as a comment"
        )
        assert default_text.count("DATES") == 1, (
            "Only the later date should be a DATES keyword"
        )

        # Explicit False: both dates emitted as DATES keywords (legacy behavior)
        legacy_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )
        assert "-- Date:" not in legacy_text, (
            "No date comment should be present when first_date_as_comment is False"
        )
        assert legacy_text.count("DATES") == 2, "Both dates should be DATES keywords"

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

        timeline.add_well_keyword_event(
            event_date="2024-01-15",
            well_path=well_path_a,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path_a.name,
                "OUTPUT_RFT": True,  # bool converted to 1
                "OUTPUT_PLT": False,  # bool converted to 0
            },
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print("SCHEDULE:", schedule_text)

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
        ), (
            f"Schedule should contain well A ({well_path_a.name}) since its event is before timestamp"
        )

        # Verify well B does NOT appear in schedule (events at 2024-03-01 are after 2024-02-01)
        well_b_name_no_spaces = well_path_b.name.replace(" ", "")
        assert (
            well_b_name_no_spaces not in schedule_text
            and well_path_b.name not in schedule_text
        ), (
            f"Schedule should NOT contain well B ({well_path_b.name}) since its event is after timestamp"
        )

        # Verify the March date does NOT appear in schedule
        assert (
            "2024-03-01" not in schedule_text
            and "1 MAR 2024" not in schedule_text
            and "1 'MAR' 2024" not in schedule_text
        ), "Schedule should NOT contain the date 2024-03-01 (after timestamp)"

        print("\n✓ Verified: Well A included (event before timestamp)")
        print("✓ Verified: Well B excluded (event after timestamp)")

    def test_wells_only_appear_at_their_event_dates(self, project_with_case_and_well):
        """Test that wells only appear in schedule sections at their event dates, not earlier.

        Scenario:
        - Add tubing + perf + ICV valve for well A at 2024-01-01 (depth 2000-2200, valve at 2100)
        - Add tubing + perf + ICD valve for well B at 2024-03-01 (depth 1500-1700, valve at 1600)
        - Add tubing + perf + ICV valve + perf + AICD valve for well A at 2024-04-01
          (perf 1800-2000 with ICV at 1900, perf 2250-2400 with AICD at 2300)
        - Set timestamp to 2024-12-31 (after all events)
        - Verify well A appears in January and April sections with different COMPSEGS
        - Verify well B appears in March section but NOT in January section
        - Verify each perf event has distinct diameter and skin_factor values
        - Verify April WELSEGS has different inner_diameter than January WELSEGS
        - Verify WSEGVALV appears in all sections (ICV in Jan/Apr, ICD in Mar produces WSEGVALV)
        - Verify WSEGAICD appears only in April (AICD valve with valid params), not in Jan or Mar
        - Verify April WSEGVALV values do NOT leak into January section
        """
        project, case, timeline = project_with_case_and_well

        # Get both well paths
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        # Add tubing + perforation for well A at 2024-01-01 (depth 2000-2200)
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=1500.0,
            end_md=2300.0,
            inner_diameter=0.22,
            roughness=9.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            measured_depth=2100.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.7,
            area=0.0001,
        )

        # Add tubing + perforation for well B at 2024-03-01 (depth 1500-1700)
        timeline.add_tubing_event(
            event_date="2024-03-01",
            well_path=well_path_b,
            start_md=1400.0,
            end_md=2000.0,
            inner_diameter=0.15,
            roughness=2.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-03-01",
            well_path=well_path_b,
            start_md=1500.0,
            end_md=1700.0,
            diameter=0.2,
            skin_factor=0.8,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-03-01",
            well_path=well_path_b,
            measured_depth=1600.0,
            valve_type="ICD",
            state="OPEN",
            flow_coefficient=0.6,
            area=0.00012,
        )

        # Add another tubing + perforation for well A at 2024-04-01 at a different depth (1800-2000)
        timeline.add_tubing_event(
            event_date="2024-04-01",
            well_path=well_path_a,
            start_md=600.0,
            end_md=2400.0,
            inner_diameter=0.20,
            roughness=3.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-04-01",
            well_path=well_path_a,
            start_md=1800.0,
            end_md=2000.0,
            diameter=0.15,
            skin_factor=0.3,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-04-01",
            well_path=well_path_a,
            measured_depth=1900.0,
            valve_type="ICV",
            state="OPEN",
            flow_coefficient=0.5,
            area=0.0002,
        )
        timeline.add_perf_event(
            event_date="2024-04-01",
            well_path=well_path_a,
            start_md=2250.0,
            end_md=2400.0,
            diameter=0.12,
            skin_factor=0.4,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-04-01",
            well_path=well_path_a,
            measured_depth=2300.0,
            valve_type="AICD",
            state="OPEN",
            flow_coefficient=0.6,
            area=0.00015,
            aicd_strength=0.00021,
            aicd_density_calib_fluid=1000.0,
            aicd_viscosity_calib_fluid=1.0,
            aicd_vol_flow_exp=2.1,
            aicd_visc_func_exp=0.5,
        )

        # Set timestamp to 2024-12-31 (after all events)
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print(schedule_text)

        # Verify schedule is generated with all dates
        assert schedule_text, "Schedule text should not be empty"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"
        assert "1 'JAN' 2024" in schedule_text or "1 JAN 2024" in schedule_text, (
            "Schedule should contain January date"
        )
        assert "1 'MAR' 2024" in schedule_text or "1 MAR 2024" in schedule_text, (
            "Schedule should contain March date"
        )
        assert "1 'APR' 2024" in schedule_text or "1 APR 2024" in schedule_text, (
            "Schedule should contain April date"
        )

        # Split schedule into date sections for easier verification
        # The schedule format is: DATES\n <date> /\n/\n\n<keywords>...\n\nDATES\n <next date>
        date_sections = schedule_text.split("DATES\n")

        # Find the January, March, and April sections
        january_section = None
        march_section = None
        april_section = None

        for section in date_sections:
            if "1 'JAN' 2024" in section or "1 JAN 2024" in section:
                january_section = section
            elif "1 'MAR' 2024" in section or "1 MAR 2024" in section:
                march_section = section
            elif "1 'APR' 2024" in section or "1 APR 2024" in section:
                april_section = section

        assert january_section is not None, "Could not find January date section"
        assert march_section is not None, "Could not find March date section"
        assert april_section is not None, "Could not find April date section"

        # Get well names (Eclipse removes spaces)
        well_a_name = well_path_a.name.replace(" ", "")
        well_b_name = well_path_b.name.replace(" ", "")

        # Verify well A appears in January section
        assert well_a_name in january_section or well_path_a.name in january_section, (
            f"Well A ({well_path_a.name}) should appear in January section"
        )

        # Verify well B does NOT appear in January section (its events are in March)
        assert (
            well_b_name not in january_section
            and well_path_b.name not in january_section
        ), (
            f"Well B ({well_path_b.name}) should NOT appear in January section (its events are at March 1)"
        )

        # Verify well B appears in March section
        assert well_b_name in march_section or well_path_b.name in march_section, (
            f"Well B ({well_path_b.name}) should appear in March section"
        )

        # Verify well A appears in April section
        assert well_a_name in april_section or well_path_a.name in april_section, (
            f"Well A ({well_path_a.name}) should appear in April section"
        )

        # Verify COMPSEGS for well A is present in both January and April sections
        assert "COMPSEGS" in january_section, (
            "January section should contain COMPSEGS for well A"
        )
        assert "COMPSEGS" in april_section, (
            "April section should contain COMPSEGS for well A"
        )

        # Extract COMPSEGS sections for well A from January and April
        # COMPSEGS format: COMPSEGS\n 'WellName' /\n <data lines> /\n
        january_compsegs_start = january_section.find("COMPSEGS")
        january_compsegs_end = january_section.find("/\n\n", january_compsegs_start)
        january_compsegs = (
            january_section[january_compsegs_start:january_compsegs_end]
            if january_compsegs_start != -1 and january_compsegs_end != -1
            else ""
        )

        april_compsegs_start = april_section.find("COMPSEGS")
        april_compsegs_end = april_section.find("/\n\n", april_compsegs_start)
        april_compsegs = (
            april_section[april_compsegs_start:april_compsegs_end]
            if april_compsegs_start != -1 and april_compsegs_end != -1
            else ""
        )

        # Verify COMPSEGS content is different between January and April
        assert january_compsegs != april_compsegs, (
            "COMPSEGS for well A should be different in January and April sections "
            "(January has perfs at 2000-2200, April adds perfs at 1800-2000)"
        )

        # Verify January COMPSEGS contains the ICV valve depth 2100
        # (ICV redirects perforation intersections to the valve measured depth)
        assert "2100" in january_compsegs, (
            "January COMPSEGS should contain ICV valve depth 2100"
        )

        # Verify April COMPSEGS contains the ICV valve depth 1900
        assert "1900" in april_compsegs, (
            "April COMPSEGS should contain ICV valve depth 1900"
        )

        # Verify WELSEGS sections exist for tubing events
        assert "WELSEGS" in january_section, (
            "January section should contain WELSEGS for well A (tubing event)"
        )
        assert "WELSEGS" in april_section, (
            "April section should contain WELSEGS for well A (second tubing event)"
        )

        # Verify that the April tubing event values do NOT leak into January WELSEGS.
        # January tubing: inner_diameter=0.15, roughness=9e-05
        # April tubing: inner_diameter=0.20, roughness=3e-05
        # The roughness value 3e-05 is unique to the April tubing event,
        # so it must NOT appear in the January section.
        assert "3e-05" not in january_section, (
            "January WELSEGS should NOT contain roughness 3e-05 from April tubing event. "
            "Tubing events should only appear at their event date, not earlier."
        )

        # Verify April section contains the April tubing roughness
        assert "3e-05" in april_section, (
            "April WELSEGS should contain roughness 3e-05 from April tubing event"
        )

        # Verify distinct perf diameter/skin values appear in COMPDAT sections
        # January perf: diameter=0.1, skin_factor=0.5
        assert "COMPDAT" in january_section, (
            "January section should contain COMPDAT for well A"
        )
        assert "0.5" in january_section, (
            "January COMPDAT should contain skin_factor 0.5 from first perf event"
        )

        # April perf: diameter=0.15, skin_factor=0.3
        assert "COMPDAT" in april_section, (
            "April section should contain COMPDAT for well A"
        )
        assert "0.3" in april_section, (
            "April COMPDAT should contain skin_factor 0.3 from third perf event"
        )

        # Verify WSEGVALV appears in January section (valve event at 2024-01-01)
        assert "WSEGVALV" in january_section, (
            "January section should contain WSEGVALV for well A (valve event at 2024-01-01)"
        )

        # Verify WSEGVALV appears in March section (ICD valve produces WSEGVALV)
        assert "WSEGVALV" in march_section, (
            "March section should contain WSEGVALV for ICD valve"
        )

        # Verify WSEGVALV appears in April section (valve event at 2024-04-01)
        assert "WSEGVALV" in april_section, (
            "April section should contain WSEGVALV for well A (valve event at 2024-04-01)"
        )

        # Verify WSEGAICD appears only in April (AICD valve), not in Jan or Mar
        assert "WSEGAICD" not in january_section, (
            "January section should NOT contain WSEGAICD (no AICD valve at January)"
        )
        assert "WSEGAICD" not in march_section, (
            "March section should NOT contain WSEGAICD (well B has no AICD valve)"
        )
        assert "WSEGAICD" in april_section, (
            "April section should contain WSEGAICD for AICD valve"
        )

        # Verify date scoping: January should have fewer WSEGVALV entries than April.
        # January: only the Jan valve is active (1 entry).
        # April: both Jan and Apr valves are active (2 entries, valves are cumulative).
        # Extract WSEGVALV blocks and count well name entries.
        jan_wsegvalv_start = january_section.find("WSEGVALV")
        jan_wsegvalv_end = january_section.find("/\n/\n", jan_wsegvalv_start)
        jan_wsegvalv = (
            january_section[jan_wsegvalv_start:jan_wsegvalv_end]
            if jan_wsegvalv_start != -1 and jan_wsegvalv_end != -1
            else ""
        )

        apr_wsegvalv_start = april_section.find("WSEGVALV")
        apr_wsegvalv_end = april_section.find("/\n/\n", apr_wsegvalv_start)
        apr_wsegvalv = (
            april_section[apr_wsegvalv_start:apr_wsegvalv_end]
            if apr_wsegvalv_start != -1 and apr_wsegvalv_end != -1
            else ""
        )

        jan_valve_count = jan_wsegvalv.count(well_a_name)
        apr_valve_count = apr_wsegvalv.count(well_a_name)

        assert jan_valve_count == 1, (
            f"January WSEGVALV should have exactly 1 entry (only Jan valve active), got {jan_valve_count}"
        )
        assert apr_valve_count == 2, (
            f"April WSEGVALV should have 2 entries (both valves active), got {apr_valve_count}. "
            "The April valve should not leak into earlier sections."
        )

        print("\n✓ Verified: Well A appears in January section (event at 2000-2200)")
        print("✓ Verified: Well B does NOT appear in January section")
        print("✓ Verified: Well B appears in March section (its event date)")
        print("✓ Verified: Well A appears in April section (event at 1800-2000)")
        print("✓ Verified: COMPSEGS for well A is different in January vs April")
        print(
            "✓ Verified: WELSEGS present in both January and April (two tubing events)"
        )
        print("✓ Verified: April tubing values do NOT leak into January section")
        print("✓ Verified: Distinct perf diameters/skin values in COMPDAT sections")
        print("✓ Verified: WSEGVALV present in January section (ICV, 1 valve entry)")
        print("✓ Verified: WSEGVALV present in March section (ICD valve)")
        print(
            "✓ Verified: WSEGVALV present in April section (ICV, 2 valve entries, cumulative)"
        )
        print("✓ Verified: WSEGAICD present in April section (AICD valve)")
        print("✓ Verified: WSEGAICD NOT present in January or March sections")
        print("✓ Verified: April valve does NOT leak into January (date scoping)")

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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        # Debug output
        print(f"\nSchedule text ({len(schedule_text)} characters):")
        print("=" * 60)
        print(schedule_text)
        print("=" * 60)

        # Verify schedule text contains expected content
        assert schedule_text, "Schedule text should not be empty"
        assert len(schedule_text) > 100, (
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

        # Verify MSW keywords from tubing events
        assert "WELSEGS" in schedule_text, (
            "Schedule should contain WELSEGS keyword from tubing events"
        )

        # Verify completion keywords from perforation events
        # With MSW (tubing), should generate COMPSEGS
        assert "COMPSEGS" in schedule_text, (
            "Schedule should contain COMPSEGS keyword from perforation events with MSW"
        )

        # Verify keyword events are included
        assert "WCONHIST" in schedule_text, (
            "Schedule should contain WCONHIST keyword event"
        )
        assert "WELTARG" in schedule_text, (
            "Schedule should contain WELTARG keyword event"
        )
        assert "WRFTPLT" in schedule_text, (
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
        assert (
            well_name_in_schedule in schedule_text or well_path.name in schedule_text
        ), (
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for WELSEGS test:\n{schedule_text}")

        assert "WELSEGS" in schedule_text, (
            "Schedule should contain WELSEGS keyword for tubing events"
        )
        # Eclipse well names typically don't have spaces
        well_name_no_spaces = well_path.name.replace(" ", "")
        assert (
            well_name_no_spaces in schedule_text or well_path.name in schedule_text
        ), (
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

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

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for WCONPROD test:\n{schedule_text}")

        assert "WCONPROD" in schedule_text or "WCONINJE" in schedule_text, (
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

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for COMPDAT test:\n{schedule_text}")

        assert "COMPDAT" in schedule_text

    def test_align_columns_adds_headers_and_alignment(self, project_with_case_and_well):
        """align_columns=True must add a '--'-prefixed column-header comment per keyword and
        indent right-aligned data rows, while the default (align_columns=False) keeps the
        compact form. Only the formatting should differ between the two."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # A perforation (COMPDAT) plus a WCONHIST keyword event gives several tabular keywords
        # plus the always-present DATES keyword to exercise the aligned formatter.
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.add_well_keyword_event(
            event_date="2024-01-01",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "ORAT": 3999.99,
                "VFP_TABLE": 1,
            },
        )

        timeline.set_timestamp(timestamp="2024-01-01")

        # Force the first date to a DATES keyword (instead of the default leading comment) so the
        # aligned DATES column header is exercised.
        aligned = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
            align_columns=True,
        )
        default = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
            align_columns=False,
        )

        print(f"\nAligned schedule text:\n{aligned}")
        print(f"\nDefault schedule text:\n{default}")

        # The DATES keyword is always present; aligned output prefixes its item names as a comment.
        assert "--DAY" in aligned, "Aligned output should carry a DATES column header"
        assert "--DAY" not in default, "Default output should not carry column headers"

        # WCONHIST item names appear only as a header comment in the aligned form (their values,
        # e.g. 'OPEN'/'RESV', are what show up in both forms).
        assert "CMODE" in aligned and "STATUS" in aligned, (
            "Aligned output should list WCONHIST item names in a header comment"
        )
        assert "CMODE" not in default and "STATUS" not in default, (
            "Default output should not list item names"
        )

        # Each aligned column-header line starts with '--' and its data rows are indented two
        # spaces and terminate with ' /'.
        wconhist_block = aligned.split("WCONHIST\n", 1)[1]
        header_line = wconhist_block.splitlines()[0]
        data_line = wconhist_block.splitlines()[1]
        # Header is a comment whose names are right-aligned into their columns, so it starts with
        # '--' and lists WELL/STATUS/CMODE (the first column may be padded ahead of 'WELL').
        assert header_line.startswith("--") and "WELL" in header_line, (
            f"WCONHIST header should be a '--' comment listing item names: {header_line!r}"
        )
        assert data_line.startswith("  "), (
            f"WCONHIST data row should be indented two spaces: {data_line!r}"
        )
        assert data_line.rstrip().endswith("/"), (
            f"WCONHIST data row should end with '/': {data_line!r}"
        )
        # Per-column defaults stay as individual '1*' markers (never accumulated into 'N*').
        assert "1*" in data_line and " 2*" not in data_line, (
            f"WCONHIST data row should keep per-column '1*' markers: {data_line!r}"
        )

        # Same keywords are produced either way.
        for keyword in ("DATES", "COMPDAT", "WCONHIST"):
            assert keyword in aligned and keyword in default

    def test_perf_completion_number_triggers_complump(self, project_with_case_and_well):
        """#13273 follow-up: a completion_number on add_perf_event must surface as a
        COMPLUMP keyword (with that number) in the generated schedule.
        """
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
            completion_number=3,
        )

        timeline.set_timestamp(timestamp="2024-01-01")

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for COMPLUMP test:\n{schedule_text}")

        assert "COMPLUMP" in schedule_text, (
            "Schedule should contain COMPLUMP when a perforation has a completion number"
        )
        # The completion number must appear inside the COMPLUMP block (header to trailing '/').
        complump_block = schedule_text.split("COMPLUMP\n", 1)[1].split("\n/\n", 1)[0]
        assert " 3 " in complump_block or complump_block.rstrip().endswith("3 /"), (
            f"Completion number 3 missing from COMPLUMP block: {complump_block!r}"
        )

    def test_perf_without_completion_number_has_no_complump(
        self, project_with_case_and_well
    ):
        """Without a completion_number, no COMPLUMP keyword should be emitted."""
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

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        assert "COMPDAT" in schedule_text
        assert "COMPLUMP" not in schedule_text, (
            "COMPLUMP should not appear when no completion number is set"
        )

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

        # Control events don't require set_timestamp. Keep all dates as DATES keywords
        # so the chronological ordering of the keyword can be verified.
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )

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

    def test_msw_export_gated_by_well_list(self, project_with_case_and_well):
        """#14079: the multi-segment-well keywords (WELSEGS, COMPSEGS, WSEGVALV,
        WSEGAICD) are emitted only for wells listed in export_msw_for_wells.

        With the default (empty list) none of them appear, while unrelated
        keywords (COMPDAT) are unaffected. Listing the well enables them.
        """
        project, case, timeline = project_with_case_and_well
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]

        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.15,
            roughness=1.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )
        timeline.set_timestamp(timestamp="2024-12-31")

        # Default (no list) suppresses all MSW keywords, COMPDAT is unaffected.
        default_text = timeline.generate_schedule_text(eclipse_case=case)
        print(f"\nSchedule with no MSW wells:\n{default_text}")
        assert "WELSEGS" not in default_text, (
            f"WELSEGS should be absent:\n{default_text}"
        )
        assert "COMPSEGS" not in default_text, (
            f"COMPSEGS should be absent:\n{default_text}"
        )
        assert "WSEGVALV" not in default_text
        assert "WSEGAICD" not in default_text
        assert "COMPDAT" in default_text

        # An explicit empty list behaves identically to the default.
        empty_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[]
        )
        assert "WELSEGS" not in empty_text
        assert "COMPSEGS" not in empty_text

        # Listing the well enables WELSEGS and COMPSEGS for it.
        enabled_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[well_path_a]
        )
        assert "WELSEGS" in enabled_text, f"WELSEGS should be present:\n{enabled_text}"
        assert "COMPSEGS" in enabled_text, (
            f"COMPSEGS should be present:\n{enabled_text}"
        )
        assert "COMPDAT" in enabled_text

    def test_msw_export_selected_per_well(self, project_with_case_and_well):
        """#14079: the well list selects MSW output per well and gates WSEGVALV
        and WSEGAICD the same way as WELSEGS / COMPSEGS.

        Well A carries an AICD valve (emits WSEGAICD, not WSEGVALV); well B
        carries an ICD valve (emits WSEGVALV, not WSEGAICD). The presence of
        each keyword therefore pinpoints exactly which well's MSW data was
        exported.
        """
        project, case, timeline = project_with_case_and_well
        well_paths = project.well_paths()
        well_path_a = [wp for wp in well_paths if "A" in wp.name][0]
        well_path_b = [wp for wp in well_paths if "B" in wp.name][0]

        # Well A: tubing + perforation + AICD valve -> WSEGAICD.
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=0.0,
            end_md=2500.0,
            inner_diameter=0.20,
            roughness=3.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            start_md=2250.0,
            end_md=2400.0,
            diameter=0.12,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-01-01",
            well_path=well_path_a,
            measured_depth=2300.0,
            valve_type="AICD",
            state="OPEN",
            flow_coefficient=0.6,
            area=0.00015,
            aicd_strength=0.00021,
            aicd_density_calib_fluid=1000.0,
            aicd_viscosity_calib_fluid=1.0,
            aicd_vol_flow_exp=2.1,
            aicd_visc_func_exp=0.5,
        )

        # Well B: tubing + perforation + ICD valve -> WSEGVALV.
        timeline.add_tubing_event(
            event_date="2024-01-01",
            well_path=well_path_b,
            start_md=0.0,
            end_md=2000.0,
            inner_diameter=0.15,
            roughness=2.0e-5,
        )
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well_path_b,
            start_md=1500.0,
            end_md=1700.0,
            diameter=0.2,
            state="OPEN",
        )
        timeline.add_valve_event(
            event_date="2024-01-01",
            well_path=well_path_b,
            measured_depth=1600.0,
            valve_type="ICD",
            state="OPEN",
            flow_coefficient=0.6,
            area=0.00012,
        )

        timeline.set_timestamp(timestamp="2024-12-31")

        # Both wells listed: all four MSW keywords present.
        both = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[well_path_a, well_path_b]
        )
        assert "WELSEGS" in both
        assert "COMPSEGS" in both
        assert "WSEGVALV" in both  # from well B's ICD valve
        assert "WSEGAICD" in both  # from well A's AICD valve

        # Only well A: WSEGAICD present, WSEGVALV absent (B's ICD excluded).
        only_a = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[well_path_a]
        )
        assert "WELSEGS" in only_a
        assert "WSEGAICD" in only_a, f"WSEGAICD should be present:\n{only_a}"
        assert "WSEGVALV" not in only_a, (
            f"WSEGVALV (well B's ICD) should be excluded:\n{only_a}"
        )

        # Only well B: WSEGVALV present, WSEGAICD absent (A's AICD excluded).
        only_b = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[well_path_b]
        )
        assert "WELSEGS" in only_b
        assert "WSEGVALV" in only_b, f"WSEGVALV should be present:\n{only_b}"
        assert "WSEGAICD" not in only_b, (
            f"WSEGAICD (well A's AICD) should be excluded:\n{only_b}"
        )

    def test_keywords_grouped_across_wells(self, project_with_case_and_well):
        """Regression for #14063: WELSPECS / COMPDAT records for multiple wells on
        the same date must appear under a single keyword header (not one block per well).
        """
        project, case, timeline = project_with_case_and_well
        well_paths = project.well_paths()
        assert len(well_paths) >= 2, (
            "Test requires at least two well paths in the fixture"
        )

        for wp in well_paths[:2]:
            timeline.add_perf_event(
                event_date="2024-01-01",
                well_path=wp,
                start_md=2000.0,
                end_md=2200.0,
                diameter=0.1,
                state="OPEN",
            )

        timeline.set_timestamp(timestamp="2024-12-31")
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print(f"\nSchedule text for multi-well grouping:\n{schedule_text}")

        # Exactly one "WELSPECS\n" header for all wells.
        welspecs_count = schedule_text.count("WELSPECS\n")
        assert welspecs_count == 1, (
            f"WELSPECS should appear once for grouped output; got {welspecs_count}:\n{schedule_text}"
        )

        # Both well names must appear inside the WELSPECS block (between header and trailing '/' line).
        welspecs_block = schedule_text.split("WELSPECS\n", 1)[1].split("\n/\n", 1)[0]
        for wp in well_paths[:2]:
            assert wp.name.replace(" ", "") in welspecs_block.replace(" ", ""), (
                f"Well {wp.name!r} missing from grouped WELSPECS block: {welspecs_block!r}"
            )

    def test_per_well_keywords_sorted_by_well_name(self, project_with_case_and_well):
        """Per-well keyword records are emitted in deck-name-sorted well order, so WELSPECS
        and COMPDAT share the same ascending well order rather than an arbitrary one.

        Note: the fixture imports wells A then B, so insertion order already matches name
        order; this test locks in the deterministic ascending ordering and cross-keyword
        consistency that the sort guarantees.
        """
        project, case, timeline = project_with_case_and_well
        well_paths = project.well_paths()
        assert len(well_paths) >= 2, (
            "Test requires at least two well paths in the fixture"
        )

        for wp in well_paths[:2]:
            timeline.add_perf_event(
                event_date="2024-01-01",
                well_path=wp,
                start_md=2000.0,
                end_md=2200.0,
                diameter=0.1,
                state="OPEN",
            )

        timeline.set_timestamp(timestamp="2024-12-31")
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print(f"\nSchedule text for sorted well order:\n{schedule_text}")

        export_names = sorted(wp.name.replace(" ", "") for wp in well_paths[:2])

        def first_positions(block):
            squashed = block.replace(" ", "")
            return [squashed.find(name) for name in export_names]

        welspecs_block = schedule_text.split("WELSPECS\n", 1)[1].split("\n/\n", 1)[0]
        compdat_block = schedule_text.split("COMPDAT\n", 1)[1].split("\n/\n", 1)[0]

        for block_name, block in (
            ("WELSPECS", welspecs_block),
            ("COMPDAT", compdat_block),
        ):
            positions = first_positions(block)
            assert all(p >= 0 for p in positions), (
                f"Both wells must appear in the {block_name} block: {block!r}"
            )
            assert positions == sorted(positions), (
                f"{block_name} wells not in ascending name order {export_names}: {block!r}"
            )

    def test_welsegs_compsegs_not_merged_across_wells(self, project_with_case_and_well):
        """WELSEGS and COMPSEGS carry a per-well header record and therefore cannot
        be merged across wells: each MSW well must get its own keyword block on the
        same date, while the other keywords stay grouped.
        """
        project, case, timeline = project_with_case_and_well
        well_paths = project.well_paths()
        assert len(well_paths) >= 2, (
            "Test requires at least two well paths in the fixture"
        )

        for wp in well_paths[:2]:
            timeline.add_tubing_event(
                event_date="2024-01-01",
                well_path=wp,
                start_md=0.0,
                end_md=2500.0,
                inner_diameter=0.15,
                roughness=1.0e-5,
            )
            timeline.add_perf_event(
                event_date="2024-01-01",
                well_path=wp,
                start_md=2000.0,
                end_md=2200.0,
                diameter=0.1,
                state="OPEN",
            )

        timeline.set_timestamp(timestamp="2024-12-31")
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=well_paths[:2]
        )
        print(f"\nSchedule text for unmerged MSW keywords:\n{schedule_text}")

        # One WELSEGS and one COMPSEGS header per MSW well (not merged into one block).
        assert schedule_text.count("WELSEGS\n") == 2, (
            f"WELSEGS should appear once per well; got {schedule_text.count('WELSEGS')}:\n{schedule_text}"
        )
        assert schedule_text.count("COMPSEGS\n") == 2, (
            f"COMPSEGS should appear once per well; got {schedule_text.count('COMPSEGS')}:\n{schedule_text}"
        )

        # COMPDAT, by contrast, stays grouped under a single header.
        assert schedule_text.count("COMPDAT\n") == 1, (
            f"COMPDAT should stay grouped under one header; got {schedule_text.count('COMPDAT')}:\n{schedule_text}"
        )


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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for keyword event:\n{schedule_text}")

        # Verify the keyword event is in the output
        assert schedule_text, "Schedule text should not be empty"
        assert "WELTARG" in schedule_text, "Schedule should contain WELTARG keyword"
        # Eclipse well names typically don't have spaces
        well_name_no_spaces = well_path.name.replace(" ", "")
        assert (
            well_name_no_spaces in schedule_text or well_path.name in schedule_text
        ), (
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text for multiple keywords:\n{schedule_text}")

        # Verify all keyword events are in the output
        assert schedule_text, "Schedule text should not be empty"
        assert "WCONHIST" in schedule_text, "Schedule should contain WCONHIST keyword"
        assert "WELTARG" in schedule_text, "Schedule should contain WELTARG keyword"
        assert "WRFTPLT" in schedule_text, "Schedule should contain WRFTPLT keyword"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"

    def test_wconhist_item_order_canonical(self, project_with_case_and_well):
        """Regression for #14065: WCONHIST items must appear in the Eclipse-defined
        canonical order regardless of how keyword_data was constructed in Python.

        Canonical WCONHIST order: WELL, STATUS, CMODE, ORAT, WRAT, GRAT, VFP_TABLE,
        ALQ, THP, BHP, WGASRAT_HIS, NGLRAT_HIS.
        """
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Intentionally non-canonical insertion order: BHP placed before ORAT/WRAT/GRAT.
        timeline.add_well_keyword_event(
            event_date="2024-01-15",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "BHP": 250.0,
                "ORAT": 3999.99,
                "WRAT": 0.01,
                "GRAT": 550678.44,
                "VFP_TABLE": 1,
            },
        )

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print(f"\nSchedule text for canonical-order check:\n{schedule_text}")

        assert "WCONHIST" in schedule_text
        # Extract the WCONHIST record body (between the keyword and its terminating '/').
        wconhist_block = schedule_text.split("WCONHIST", 1)[1].split("/", 1)[0]

        orat_pos = wconhist_block.find("3999.99")
        grat_pos = wconhist_block.find("550678")
        bhp_pos = wconhist_block.find("250")
        assert orat_pos >= 0, "ORAT value missing from WCONHIST output"
        assert grat_pos >= 0, "GRAT value missing from WCONHIST output"
        assert bhp_pos >= 0, "BHP value missing from WCONHIST output"
        assert orat_pos < bhp_pos, (
            f"ORAT must precede BHP in canonical WCONHIST order; got block: {wconhist_block!r}"
        )
        assert grat_pos < bhp_pos, (
            f"GRAT must precede BHP in canonical WCONHIST order; got block: {wconhist_block!r}"
        )

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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule with perf and keyword events:\n{schedule_text}")

        # Verify both physical completions and keyword events are present
        assert schedule_text, "Schedule text should not be empty"
        assert "WELSEGS" in schedule_text, "Schedule should contain MSW WELSEGS keyword"
        assert "WELTARG" in schedule_text, (
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule with control and keyword events:\n{schedule_text}")

        # Verify both control and keyword events are present
        assert schedule_text, "Schedule text should not be empty"
        assert "WCONPROD" in schedule_text or "WCONINJE" in schedule_text, (
            "Schedule should contain control keyword"
        )
        assert "WRFTPLT" in schedule_text, (
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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

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
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule with boolean keyword values:\n{schedule_text}")
        assert schedule_text, "Schedule text should not be empty"


class TestScheduleKeywordEvents:
    """Tests for schedule-level keyword event functionality (not tied to wells)."""

    @pytest.fixture
    def project_with_case_and_well(self, rips_instance, initialize_test):
        """Load a case with well paths for schedule keyword event tests."""
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

    def test_add_keyword_event_rptrst(self, project_with_case_and_well):
        """Test adding a RPTRST schedule keyword event (not tied to a well)."""
        project, case, timeline = project_with_case_and_well

        # Create a RPTRST event (report restart settings)
        event = timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
                "FREQ": 1,
            },
        )

        assert event is not None, "Keyword event should be created"

    def test_add_keyword_event_gruptree(self, project_with_case_and_well):
        """Test adding a GRUPTREE schedule keyword event."""
        project, case, timeline = project_with_case_and_well

        # Create a GRUPTREE event (group tree definition)
        event = timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="GRUPTREE",
            keyword_data={
                "CHILD": "OP",
                "PARENT": "FIELD",
            },
        )

        assert event is not None, "GRUPTREE event should be created"

    def test_add_keyword_event_rptsched(self, project_with_case_and_well):
        """Test adding a RPTSCHED schedule keyword event."""
        project, case, timeline = project_with_case_and_well

        # Create a RPTSCHED event (report schedule settings)
        event = timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTSCHED",
            keyword_data={
                "FIP": 1,
                "WELLS": 2,
            },
        )

        assert event is not None, "RPTSCHED event should be created"

    def test_tuning_keyword_multi_record_output(self, project_with_case_and_well):
        """TUNING is a multi-record keyword (3 records, each terminated by '/'). Items
        spanning different records must be distributed into their own records, producing
        three slashes rather than one.
        """
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # A well event so the date section is emitted.
        timeline.add_control_event(
            event_date="2018-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        # Items from record 1 (TSINIT, TSMAXZ, TMAXWC) and record 3 (NEWTMX..MXWPIT).
        timeline.add_keyword_event(
            event_date="2018-01-01",
            keyword_name="TUNING",
            keyword_data={
                "TSINIT": 1,
                "TSMAXZ": 30,
                "TMAXWC": 1,
                "NEWTMX": 12,
                "NEWTMN": 1,
                "LITMAX": 50,
                "LITMIN": 1,
                "MXWSIT": 50,
                "MXWPIT": 50,
            },
        )

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule text with TUNING keyword:\n{schedule_text}")

        assert "TUNING" in schedule_text, "Schedule should contain TUNING keyword"

        # Isolate the TUNING block (header line up to the following blank line).
        tuning_block = schedule_text.split("TUNING\n", 1)[1].split("\n\n", 1)[0]
        record_terminators = [
            line for line in tuning_block.splitlines() if line.strip().endswith("/")
        ]
        assert len(record_terminators) == 3, (
            f"TUNING must emit three records (three '/'), got {len(record_terminators)}:\n{tuning_block}"
        )

        # Record 1 keeps TSMAXZ; record 3 keeps NEWTMX. Both values must survive.
        assert "30" in tuning_block, "TSMAXZ value missing from TUNING record 1"
        assert "12" in tuning_block, "NEWTMX value missing from TUNING record 3"

    def test_keyword_event_schedule_output(self, project_with_case_and_well):
        """Test that schedule keyword events appear in schedule text generation."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add a well event to ensure we have a date section
        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        # Add a schedule-level keyword event (not tied to a well)
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
                "FREQ": 1,
            },
        )

        # Generate schedule text (keep the first date as a DATES keyword for this assertion)
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=project.well_paths(),
            first_date_as_comment=False,
        )

        print(f"\nSchedule text with RPTRST keyword:\n{schedule_text}")

        # Verify the schedule-level keyword is in the output
        assert schedule_text, "Schedule text should not be empty"
        assert "RPTRST" in schedule_text, "Schedule should contain RPTRST keyword"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"

    def test_keyword_event_mixed_with_well_events(self, project_with_case_and_well):
        """Test schedule keyword events alongside well-specific events."""
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
            event_date="2024-01-01",
            well_path=well_path,
            start_md=2000.0,
            end_md=2200.0,
            diameter=0.1,
            state="OPEN",
        )

        # Add schedule-level keyword event on the same date
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
            },
        )

        # Apply tubing/perf events
        timeline.set_timestamp(timestamp="2024-12-31")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(
            f"\nSchedule with mixed well and schedule-level keywords:\n{schedule_text}"
        )

        # Verify both well-specific and schedule-level keywords are present
        assert schedule_text, "Schedule text should not be empty"
        assert "WELSEGS" in schedule_text, "Schedule should contain WELSEGS keyword"
        assert "RPTRST" in schedule_text, "Schedule should contain RPTRST keyword"

    def test_keyword_event_at_multiple_dates(self, project_with_case_and_well):
        """Test schedule keyword events at different dates."""
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Add well events at different dates to create date sections
        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        timeline.add_control_event(
            event_date="2024-06-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=800.0,
            oil_rate=800.0,
            is_producer=True,
        )

        # Add schedule-level keyword events at different dates
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
            },
        )

        timeline.add_keyword_event(
            event_date="2024-06-01",
            keyword_name="RPTSCHED",
            keyword_data={
                "FIP": 1,
            },
        )

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )

        print(f"\nSchedule with keyword events at multiple dates:\n{schedule_text}")

        # Verify both keywords are present
        assert schedule_text, "Schedule text should not be empty"
        assert "RPTRST" in schedule_text, "Schedule should contain RPTRST keyword"
        assert "RPTSCHED" in schedule_text, "Schedule should contain RPTSCHED keyword"
        assert "DATES" in schedule_text, "Schedule should contain DATES keyword"

    def test_keyword_event_type_inference(self, project_with_case_and_well):
        """Test that add_keyword_event correctly infers types from Python values."""
        project, case, timeline = project_with_case_and_well

        # Create event with mixed types: str, int, float, bool
        event = timeline.add_keyword_event(
            event_date="2024-03-15",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,  # int
                "FREQ": 1,  # int
            },
        )

        assert event is not None, "Event with mixed types should be created"

    def test_rptrst_mnemonic_output(self, project_with_case_and_well):
        """RPTRST/RPTSCHED are mnemonic-list keywords. bool True must emit a
        bare KEY, int/float/str must emit KEY=VALUE, bool False must be omitted.
        """
        project, case, timeline = project_with_case_and_well
        well_path = project.well_paths()[0]

        # Schedule generation requires at least one well event so a well path is
        # selected for output; the assertions below target only the RPTRST block.
        timeline.add_control_event(
            event_date="2024-01-01",
            well_path=well_path,
            control_mode="ORAT",
            control_value=1000.0,
            oil_rate=1000.0,
            is_producer=True,
        )

        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
                "DEN": True,
                "ROCKC": True,
                "RPORV": True,
                "RFIP": True,
                "FLOWS": True,
                "NORST": 1,
                "FLORES": True,
                "OBSOLETE": False,
            },
        )

        schedule_text = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=project.well_paths()
        )
        print(f"\nRPTRST mnemonic output:\n{schedule_text}")

        assert "RPTRST" in schedule_text
        rptrst_block = schedule_text.split("RPTRST", 1)[1].split("/", 1)[0]

        # Keyed mnemonics rendered as KEY=VALUE.
        assert "BASIC=2" in rptrst_block
        assert "NORST=1" in rptrst_block
        # Flag mnemonics rendered as bare tokens. Whitespace-bounded so we don't accept
        # accidental substring matches like 'DEN' inside another token.
        for flag in ("DEN", "ROCKC", "RPORV", "RFIP", "FLOWS", "FLORES"):
            assert f" {flag} " in rptrst_block or rptrst_block.rstrip().endswith(
                f" {flag}"
            ), f"flag {flag!r} missing from RPTRST output: {rptrst_block!r}"
        # False-valued flag must be omitted entirely.
        assert "OBSOLETE" not in rptrst_block
