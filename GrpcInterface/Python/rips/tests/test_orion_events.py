"""
Tests for the ORIONEVENTS parser and applier (rips.orion_events).

The parser tests (Layer A) are pure Python and need no running ResInsight.
The applier tests (Layer B) drive the mapping logic against a fake timeline and
fake project, so they also run without an instance.

TestOrionEventsIntegration drives the real WellEventTimeline API and therefore
needs a running ResInsight (the shared instance from conftest.py); it is skipped
automatically when no instance is available.
"""

import datetime
import os
import sys

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import rips  # noqa: E402
import dataroot  # noqa: E402
from rips.orion_events import (  # noqa: E402
    ApplyReport,
    OrionParseError,
    apply_orion_document,
    parse_orion_events,
)

SAMPLE = """\
# A comment line
ORIONEVENTS 1.0
UNIT METRIC

SET A1_STARTUP = 2018-01-01
SET A2_STARTUP = 2018-03-01 + 9

'55_33-A-1'
  @A1_STARTUP      PERFORATION  MDSTART=1644.49  MDEND=1664.28  RADIUS=0.12065  SKIN=5  COMPLETION_NUMBER=1
  @A1_STARTUP + 5  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1
  @A1_STARTUP + 5  WELTARG      CMODE=BHP  VALUE=50

'55_33-A-2'
  @A2_STARTUP      PERFORATION  MDSTART=1692.79  MDEND=1706  RADIUS=0.12065  SKIN=5  COMPLETION_NUMBER=1
"""


# ---------------------------------------------------------------------------
# Fakes for the applier tests
# ---------------------------------------------------------------------------


class FakeWellPath:
    def __init__(self, name):
        self.name = name


class FakeProject:
    def __init__(self, names):
        self._wells = {name: FakeWellPath(name) for name in names}

    def well_path_by_name(self, name):
        return self._wells.get(name)


class FakeTimeline:
    """Records calls made by the applier instead of contacting ResInsight."""

    def __init__(self):
        self.perf_calls = []
        self.keyword_calls = []

    def add_perf_event(self, **kwargs):
        self.perf_calls.append(kwargs)

    def add_well_keyword_event(self, **kwargs):
        self.keyword_calls.append(kwargs)


# ---------------------------------------------------------------------------
# Layer A: parsing
# ---------------------------------------------------------------------------


class TestParsing:
    def test_header_and_unit(self):
        doc = parse_orion_events(SAMPLE)
        assert doc.version == "1.0"
        assert doc.unit_system == "METRIC"

    def test_set_variables_with_day_offset(self):
        doc = parse_orion_events(SAMPLE)
        assert doc.variables["A1_STARTUP"] == datetime.date(2018, 1, 1)
        # 2018-03-01 + 9 days
        assert doc.variables["A2_STARTUP"] == datetime.date(2018, 3, 10)

    def test_well_blocks_and_event_dates(self):
        doc = parse_orion_events(SAMPLE)
        assert [w.well_name for w in doc.wells] == ["55_33-A-1", "55_33-A-2"]
        well1 = doc.wells[0]
        assert [e.event_type for e in well1.events] == [
            "PERFORATION",
            "WCONHIST",
            "WELTARG",
        ]
        # @A1_STARTUP + 5 -> 2018-01-06
        assert well1.events[1].event_date == datetime.date(2018, 1, 6)

    def test_value_type_inference(self):
        doc = parse_orion_events(SAMPLE)
        perf = doc.wells[0].events[0]
        assert perf.attributes["MDSTART"].value == 1644.49  # float
        assert perf.attributes["COMPLETION_NUMBER"].value == 1  # int
        assert perf.attributes["MDEND"].value == 1664.28
        wconhist = doc.wells[0].events[1]
        assert wconhist.attributes["STATUS"].value == "OPEN"  # string

    def test_quoted_filter_value_is_single_attribute(self):
        # FILTER itself is unsupported, but the quote-aware tokenizer must treat
        # the whole quoted expression as one attribute. Use an allowed key here.
        text = (
            "ORIONEVENTS 1.0\n'W'\n"
            '  @2018-01-01 WCONHIST NOTE="SOIL(0) > 0.8 AND PERMX > 200" CMODE=ORAT\n'
        )
        doc = parse_orion_events(text)
        note = doc.wells[0].events[0].attributes["NOTE"]
        assert note.value == "SOIL(0) > 0.8 AND PERMX > 200"
        assert note.quoted is True
        assert doc.wells[0].events[0].attributes["CMODE"].value == "ORAT"

    def test_trailing_comment_ignored_but_not_inside_quotes(self):
        text = (
            "ORIONEVENTS 1.0\n'W'\n"
            '  @2018-01-01 WCONHIST NOTE="a # b" CMODE=ORAT  # trailing comment\n'
        )
        doc = parse_orion_events(text)
        attrs = doc.wells[0].events[0].attributes
        assert attrs["NOTE"].value == "a # b"
        assert "CMODE" in attrs

    def test_iso_date_literal_event(self):
        text = "ORIONEVENTS 1.0\n'W'\n  @2020-12-31 PERFORATION MDSTART=1 MDEND=2\n"
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2020, 12, 31)

    def test_missing_header_raises(self):
        with pytest.raises(OrionParseError):
            parse_orion_events("UNIT METRIC\n'W'\n")

    def test_event_before_well_block_raises(self):
        with pytest.raises(OrionParseError):
            parse_orion_events(
                "ORIONEVENTS 1.0\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
            )

    def test_unknown_variable_raises(self):
        with pytest.raises(OrionParseError):
            parse_orion_events(
                "ORIONEVENTS 1.0\n'W'\n  @NOPE PERFORATION MDSTART=1 MDEND=2\n"
            )

    def test_filter_attribute_is_unsupported(self):
        text = (
            "ORIONEVENTS 1.0\n'W'\n"
            '  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 FILTER="SOIL(0) > 0.8"\n'
        )
        with pytest.raises(OrionParseError, match="FILTER"):
            parse_orion_events(text)

    def test_perfid_attribute_is_unsupported(self):
        text = "ORIONEVENTS 1.0\n'W'\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 PERFID=Valysar\n"
        with pytest.raises(OrionParseError, match="PERFID"):
            parse_orion_events(text)

    def test_malformed_attribute_raises(self):
        text = (
            "ORIONEVENTS 1.0\n'W'\n  @2018-01-01 PERFORATION MDSTART=1 bogus MDEND=2\n"
        )
        with pytest.raises(OrionParseError):
            parse_orion_events(text)

    def test_duplicate_set_warns(self):
        text = "ORIONEVENTS 1.0\nSET X = 2018-01-01\nSET X = 2019-01-01\n'W'\n"
        doc = parse_orion_events(text)
        assert any("Duplicate" in w.message for w in doc.warnings)
        assert doc.variables["X"] == datetime.date(2019, 1, 1)


# ---------------------------------------------------------------------------
# Layer B: applying
# ---------------------------------------------------------------------------


class TestApplying:
    def _apply(self, text, names=("55_33-A-1", "55_33-A-2"), **opts):
        doc = parse_orion_events(text)
        timeline = FakeTimeline()
        project = FakeProject(names)
        report = apply_orion_document(doc, timeline, project, **opts)
        return timeline, report

    def test_perforation_mapping_radius_to_diameter(self):
        timeline, report = self._apply(SAMPLE)
        assert report.events_applied == 4  # 2 perfs + WCONHIST + WELTARG
        first = timeline.perf_calls[0]
        assert first["start_md"] == 1644.49
        assert first["end_md"] == 1664.28
        assert first["diameter"] == pytest.approx(2 * 0.12065)
        assert first["skin_factor"] == 5.0
        assert first["completion_number"] == 1
        assert first["state"] == "OPEN"
        assert first["event_date"] == "2018-01-01"

    def test_wconhist_field_translation_and_well_injection(self):
        timeline, _ = self._apply(SAMPLE)
        wconhist = next(
            c for c in timeline.keyword_calls if c["keyword_name"] == "WCONHIST"
        )
        data = wconhist["keyword_data"]
        assert data["WELL"] == "55_33-A-1"
        assert data["STATUS"] == "OPEN"
        assert data["CMODE"] == "ORAT"
        assert data["VFP_TABLE"] == 1  # VFP -> VFP_TABLE
        assert "VFP" not in data

    def test_weltarg_value_translation(self):
        timeline, _ = self._apply(SAMPLE)
        weltarg = next(
            c for c in timeline.keyword_calls if c["keyword_name"] == "WELTARG"
        )
        data = weltarg["keyword_data"]
        assert data["NEW_VALUE"] == 50  # VALUE -> NEW_VALUE
        assert data["CMODE"] == "BHP"

    def test_dshift_is_ignored_with_warning(self):
        text = (
            "ORIONEVENTS 1.0\n'55_33-A-1'\n"
            "  @2018-01-01 WCONHIST STATUS=OPEN CMODE=ORAT DSHIFT=10\n"
        )
        timeline, report = self._apply(text)
        data = timeline.keyword_calls[0]["keyword_data"]
        assert "DSHIFT" not in data
        # Event date is NOT shifted.
        assert timeline.keyword_calls[0]["event_date"] == "2018-01-01"
        assert any("DSHIFT" in w for w in report.warnings)

    def test_unknown_well_warns_and_skips(self):
        timeline, report = self._apply(SAMPLE, names=("55_33-A-1",))
        # Second well is unknown -> its single perforation is skipped.
        assert report.events_skipped == 1
        assert any("55_33-A-2" in w for w in report.warnings)

    def test_unknown_well_error_policy_raises(self):
        from rips.exception import RipsError

        with pytest.raises(RipsError):
            self._apply(SAMPLE, names=("55_33-A-1",), on_unknown_well="error")

    def test_unknown_event_type_warns(self):
        text = "ORIONEVENTS 1.0\n'55_33-A-1'\n  @2018-01-01 FOObar X=1\n"
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("FOObar" in w for w in report.warnings)

    def test_perforation_missing_required_attr_is_error(self):
        text = "ORIONEVENTS 1.0\n'55_33-A-1'\n  @2018-01-01 PERFORATION MDSTART=1\n"
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("MDEND" in e for e in report.errors)

    def test_perforation_unknown_attr_is_error(self):
        text = "ORIONEVENTS 1.0\n'55_33-A-1'\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 ZZZ=3\n"
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("ZZZ" in e for e in report.errors)

    def test_invalid_policy_value_raises(self):
        doc = parse_orion_events(SAMPLE)
        with pytest.raises(ValueError):
            apply_orion_document(
                doc, FakeTimeline(), FakeProject(["x"]), on_unknown_well="bogus"
            )

    def test_apply_report_default_is_empty(self):
        report = ApplyReport()
        assert report.events_applied == 0
        assert report.warnings == []


# ---------------------------------------------------------------------------
# Integration: drives the real WellEventTimeline API (needs a ResInsight
# instance provided by conftest.py).
# ---------------------------------------------------------------------------


class TestOrionEventsIntegration:
    @pytest.fixture
    def project_with_case_and_wells(self, rips_instance, initialize_test):
        """Load the TEST10K case and import two well paths from .dev files."""
        case_root = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
        project = rips_instance.project
        case = project.load_case(path=case_root + "/TEST10K_FLT_LGR_NNC.EGRID")
        project.import_well_paths(
            well_path_files=[
                case_root + "/wellpath_a.dev",
                case_root + "/wellpath_b.dev",
            ]
        )
        well_path_coll = project.descendants(rips.WellPathCollection)[0]
        return project, case, well_path_coll.event_timeline()

    def test_apply_creates_perforations_and_schedule(self, project_with_case_and_wells):
        """End-to-end: parse -> apply -> set_timestamp -> generate schedule."""
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        # Reference the real well name; MD range is valid for well path A.
        text = (
            "ORIONEVENTS 1.0\n"
            "UNIT METRIC\n"
            "SET START = 2024-01-01\n"
            f"'{well.name}'\n"
            "  @START      PERFORATION  MDSTART=2000  MDEND=2200  RADIUS=0.05  SKIN=0.5  COMPLETION_NUMBER=1\n"
            "  @START + 5  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1\n"
            "  @START + 5  WELTARG      CMODE=BHP  VALUE=50\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.events_applied == 3

        # Materialize completions from the perforation event.
        timeline.set_timestamp(timestamp="2024-01-15")
        perforations = well.completions().perforations().perforations()
        assert len(perforations) > 0, "Perforation should be created from event"
        perf = perforations[0]
        # RADIUS=0.05 must have been mapped to diameter = 0.1.
        assert abs(perf.start_measured_depth - 2000.0) < 1.0
        assert abs(perf.end_measured_depth - 2200.0) < 1.0

        # The generated schedule should carry the mapped keywords.
        schedule = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[]
        )
        assert "COMPDAT" in schedule
        assert "WCONHIST" in schedule
        assert "WELTARG" in schedule

    def test_apply_unknown_well_warns_and_applies_nothing(
        self, project_with_case_and_wells
    ):
        project, _case, timeline = project_with_case_and_wells
        text = (
            "ORIONEVENTS 1.0\n'NO_SUCH_WELL'\n"
            "  @2024-01-01 PERFORATION MDSTART=1 MDEND=2\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.events_applied == 0
        assert any("NO_SUCH_WELL" in w for w in report.warnings)
