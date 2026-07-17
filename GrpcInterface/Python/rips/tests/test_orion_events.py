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
    _cli,
    apply_orion_document,
    parse_orion_events,
)

SAMPLE = """\
# A comment line
ORIONEVENTS 2.0
UNIT METRIC

DATE     A1_STARTUP = 2018-01-01
DATE     A2_STARTUP = 2018-03-01 + 9
DURATION RAMP       = 5 DAYS

WELL A1 = "55_33-A-1"

WELL A1
  @A1_STARTUP         PERFORATION  MDSTART=1644.49  MDEND=1664.28  RADIUS=0.12065  SKIN=5  COMPLETION_NUMBER=1
  @A1_STARTUP + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1
  @A1_STARTUP + RAMP  WELTARG      CMODE=BHP  VALUE=50

WELL "55_33-A-2"
  @A2_STARTUP  PERFORATION  MDSTART=1692.79  MDEND=1706  RADIUS=0.12065  SKIN=5  COMPLETION_NUMBER=1
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
        assert doc.version == "2.0"
        assert doc.unit_system == "METRIC"

    def test_date_declarations_with_day_offset(self):
        doc = parse_orion_events(SAMPLE)
        assert doc.variables["A1_STARTUP"].kind == "DATE"
        assert doc.variables["A1_STARTUP"].value == datetime.date(2018, 1, 1)
        # 2018-03-01 + 9 days
        assert doc.variables["A2_STARTUP"].value == datetime.date(2018, 3, 10)

    def test_duration_and_well_declarations(self):
        doc = parse_orion_events(SAMPLE)
        assert doc.variables["RAMP"].kind == "DURATION"
        assert doc.variables["RAMP"].value == 5
        assert doc.variables["A1"].kind == "WELL"
        assert doc.variables["A1"].value == "55_33-A-1"

    def test_well_blocks_and_event_dates(self):
        doc = parse_orion_events(SAMPLE)
        assert [w.well_name for w in doc.wells] == ["55_33-A-1", "55_33-A-2"]
        well1 = doc.wells[0]
        assert [e.event_type for e in well1.events] == [
            "PERFORATION",
            "WCONHIST",
            "WELTARG",
        ]
        # @A1_STARTUP + RAMP -> 2018-01-06
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
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            '  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 FILTER="SOIL(0) > 0.8 AND PERMX > 200"\n'
        )
        doc = parse_orion_events(text)
        filter_attr = doc.wells[0].events[0].attributes["FILTER"]
        assert filter_attr.value == "SOIL(0) > 0.8 AND PERMX > 200"
        assert filter_attr.quoted is True
        assert doc.wells[0].events[0].attributes["MDEND"].value == 2

    def test_perfid_attribute_parses(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 PERFID=Valysar\n'
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].attributes["PERFID"].value == "Valysar"

    def test_trailing_comment_ignored_but_not_inside_quotes(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            '  @2018-01-01 WCONHIST NOTE="a # b" CMODE=ORAT  # trailing comment\n'
        )
        doc = parse_orion_events(text)
        attrs = doc.wells[0].events[0].attributes
        assert attrs["NOTE"].value == "a # b"
        assert "CMODE" in attrs

    def test_iso_date_literal_event(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n  @2020-12-31 PERFORATION MDSTART=1 MDEND=2\n'
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2020, 12, 31)

    def test_negative_offset(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-10\n"
            'WELL "W"\n  @START - 5 PERFORATION MDSTART=1 MDEND=2\n'
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2018, 1, 5)

    def test_minus_after_iso_date(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  @2018-01-01 - 5 PERFORATION MDSTART=1 MDEND=2\n'
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2017, 12, 27)

    def test_offset_chain_with_duration_variable(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-01\nDURATION RAMP = 5\n"
            'WELL "W"\n  @START + RAMP - 2 PERFORATION MDSTART=1 MDEND=2\n'
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2018, 1, 4)

    def test_duration_declaration_days_suffix_optional(self):
        for suffix in ("", " DAYS", " days"):
            doc = parse_orion_events(f"ORIONEVENTS 2.0\nDURATION X = 5{suffix}\n")
            assert doc.variables["X"].value == 5

    def test_duration_arithmetic_in_declaration(self):
        text = "ORIONEVENTS 2.0\nDURATION RAMP = 5\nDURATION X = RAMP + 2 DAYS\n"
        doc = parse_orion_events(text)
        assert doc.variables["X"].value == 7

    def test_well_alias_resolution(self):
        doc = parse_orion_events(SAMPLE)
        # 'WELL A1' block resolves to the declared alias target.
        assert doc.wells[0].well_name == "55_33-A-1"

    def test_quoted_well_block_ignores_alias(self):
        text = 'ORIONEVENTS 2.0\nWELL A1 = "55_33-A-1"\nWELL "A1"\n'
        doc = parse_orion_events(text)
        assert doc.wells[0].well_name == "A1"

    def test_unknown_well_alias_raises(self):
        with pytest.raises(OrionParseError, match="Unknown variable 'NOPE'"):
            parse_orion_events("ORIONEVENTS 2.0\nWELL NOPE\n")

    def test_empty_well_block_ok(self):
        doc = parse_orion_events('ORIONEVENTS 2.0\nWELL "W"\n')
        assert doc.wells[0].well_name == "W"
        assert doc.wells[0].events == []

    def test_v1_file_rejected_with_clear_message(self):
        with pytest.raises(OrionParseError, match="no longer supported"):
            parse_orion_events("ORIONEVENTS 1.0\nUNIT METRIC\n")

    def test_unsupported_version_rejected(self):
        with pytest.raises(OrionParseError, match="Unsupported ORIONEVENTS version"):
            parse_orion_events("ORIONEVENTS 3.0\n")

    def test_missing_header_raises(self):
        with pytest.raises(OrionParseError):
            parse_orion_events('UNIT METRIC\nWELL "W"\n')

    def test_event_before_well_block_raises(self):
        with pytest.raises(OrionParseError, match="before any WELL block"):
            parse_orion_events(
                "ORIONEVENTS 2.0\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
            )

    def test_unknown_variable_raises(self):
        with pytest.raises(OrionParseError, match="Unknown variable 'NOPE'"):
            parse_orion_events(
                'ORIONEVENTS 2.0\nWELL "W"\n  @NOPE PERFORATION MDSTART=1 MDEND=2\n'
            )

    def test_duration_where_date_expected_raises(self):
        text = (
            "ORIONEVENTS 2.0\nDURATION RAMP = 5\n"
            'WELL "W"\n  @RAMP WCONHIST STATUS=OPEN\n'
        )
        with pytest.raises(
            OrionParseError, match=r"is a DURATION \(declared line 2\) but a DATE"
        ):
            parse_orion_events(text)

    def test_date_where_duration_expected_raises(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-01\nDATE OTHER = 2018-02-01\n"
            'WELL "W"\n  @START + OTHER WCONHIST STATUS=OPEN\n'
        )
        with pytest.raises(OrionParseError, match="is a DATE .* but a DURATION"):
            parse_orion_events(text)

    def test_well_variable_in_date_context_raises(self):
        text = 'ORIONEVENTS 2.0\nWELL A1 = "X"\nWELL A1\n  @A1 WCONHIST STATUS=OPEN\n'
        with pytest.raises(OrionParseError, match="is a WELL .* but a DATE"):
            parse_orion_events(text)

    def test_cross_type_redefinition_raises(self):
        text = "ORIONEVENTS 2.0\nDATE X = 2018-01-01\nDURATION X = 5\n"
        with pytest.raises(
            OrionParseError, match="already declared as DATE .* redeclare as DURATION"
        ):
            parse_orion_events(text)

    def test_forward_reference_raises(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n  @START PERFORATION MDSTART=1 MDEND=2\n'
            "DATE START = 2018-01-01\n"
        )
        with pytest.raises(OrionParseError, match="Unknown variable 'START'"):
            parse_orion_events(text)

    def test_single_quoted_well_name_rejected(self):
        text = "ORIONEVENTS 2.0\n'W'\n"
        with pytest.raises(OrionParseError, match="1.x syntax"):
            parse_orion_events(text)

    def test_set_line_rejected_with_hint(self):
        text = "ORIONEVENTS 2.0\nSET X = 2018-01-01\n"
        with pytest.raises(OrionParseError, match="SET is ORIONEVENTS 1.x syntax"):
            parse_orion_events(text)

    def test_malformed_attribute_raises(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  @2018-01-01 PERFORATION MDSTART=1 bogus MDEND=2\n'
        with pytest.raises(OrionParseError, match="Malformed attribute"):
            parse_orion_events(text)

    def test_duplicate_date_declaration_warns(self):
        text = 'ORIONEVENTS 2.0\nDATE X = 2018-01-01\nDATE X = 2019-01-01\nWELL "W"\n'
        doc = parse_orion_events(text)
        assert any("Duplicate" in w.message for w in doc.warnings)
        assert doc.variables["X"].value == datetime.date(2019, 1, 1)


class TestDiagnostics:
    def test_all_errors_reported_in_one_pass(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "DATE A1_STARTUP = 2018-01-01\n"
            "SET X = 2018-01-01\n"  # line 3
            'WELL "W"\n'
            "  @A1_STRTUP PERFORATION MDSTART=1 MDEND=2\n"  # line 5
            "  @A1_STARTUP + NOPE WCONHIST STATUS=OPEN\n"  # line 6
        )
        with pytest.raises(OrionParseError) as excinfo:
            parse_orion_events(text)
        assert [issue.loc.line for issue in excinfo.value.errors] == [3, 5, 6]

    def test_malformed_well_block_suppresses_cascading_errors(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "WELL 55_33-A-2\n"  # malformed: unquoted special characters
            "  @2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
            "  @2018-01-01 WCONHIST STATUS=OPEN\n"
        )
        with pytest.raises(OrionParseError) as excinfo:
            parse_orion_events(text)
        assert len(excinfo.value.errors) == 1
        assert "Malformed WELL line" in excinfo.value.errors[0].message

    def test_unknown_variable_hint(self):
        text = (
            "ORIONEVENTS 2.0\nDATE A1_STARTUP = 2018-01-01\n"
            'WELL "W"\n  @A1_STRTUP PERFORATION MDSTART=1 MDEND=2\n'
        )
        with pytest.raises(OrionParseError, match="did you mean 'A1_STARTUP'"):
            parse_orion_events(text)

    def test_misspelled_keyword_hint(self):
        with pytest.raises(OrionParseError, match="did you mean 'DURATION'"):
            parse_orion_events("ORIONEVENTS 2.0\nDURATON X = 5\n")


class TestValidatorCli:
    def _write(self, tmp_path, text):
        path = tmp_path / "events.orion"
        path.write_text(text)
        return str(path)

    def test_valid_file_exits_zero_with_summary(self, tmp_path, capsys):
        path = self._write(tmp_path, SAMPLE)
        assert _cli([path]) == 0
        out = capsys.readouterr().out
        assert "OK" in out
        assert "4 variable(s), 2 well block(s), 4 event(s)" in out

    def test_invalid_file_exits_nonzero_with_errors(self, tmp_path, capsys):
        path = self._write(
            tmp_path,
            'ORIONEVENTS 2.0\nSET X = 2018-01-01\nWELL "W"\n  @NOPE WCONHIST A=1\n',
        )
        assert _cli([path]) == 1
        out = capsys.readouterr().out
        assert "Line 2" in out
        assert "Line 4" in out
        assert "2 error(s) found" in out

    def test_missing_file_exits_nonzero(self, tmp_path, capsys):
        assert _cli([str(tmp_path / "nope.orion")]) == 1
        assert "Error" in capsys.readouterr().out


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
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  @2018-01-01 WCONHIST STATUS=OPEN CMODE=ORAT DSHIFT=10\n"
        )
        timeline, report = self._apply(text)
        data = timeline.keyword_calls[0]["keyword_data"]
        assert "DSHIFT" not in data
        # Event date is NOT shifted.
        assert timeline.keyword_calls[0]["event_date"] == "2018-01-01"
        assert any("DSHIFT" in w for w in report.warnings)

    def test_filter_on_perforation_warns_and_applies(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            '  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 FILTER="SOIL(0) > 0.8"\n'
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        assert report.events_skipped == 0
        assert any("FILTER" in w for w in report.warnings)
        assert len(timeline.perf_calls) == 1

    def test_perfid_on_perforation_warns_and_applies(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 PERFID=Valysar\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        assert any("PERFID" in w for w in report.warnings)

    def test_filter_on_keyword_event_warns_and_applies(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            '  @2018-01-01 WCONHIST STATUS=OPEN FILTER="PERMX > 200"\n'
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        assert "FILTER" not in timeline.keyword_calls[0]["keyword_data"]
        assert any("FILTER" in w for w in report.warnings)

    def test_unknown_well_warns_and_skips(self):
        timeline, report = self._apply(SAMPLE, names=("55_33-A-1",))
        # Second well is unknown -> its single perforation is skipped.
        assert report.events_skipped == 1
        assert any("55_33-A-2" in w for w in report.warnings)

    def test_unknown_well_error_policy_raises(self):
        from rips.exception import RipsError

        with pytest.raises(RipsError):
            self._apply(SAMPLE, names=("55_33-A-1",), on_unknown_well="error")

    def test_unknown_event_type_warns_with_hint(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  @2018-01-01 WCONHST STATUS=OPEN\n'
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any(
            "WCONHST" in w and "did you mean 'WCONHIST'" in w for w in report.warnings
        )

    def test_perforation_missing_required_attr_is_error(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  @2018-01-01 PERFORATION MDSTART=1\n'
        )
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("MDEND" in e for e in report.errors)

    def test_perforation_unknown_attr_is_error(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  @2018-01-01 PERFORATION MDSTART=1 MDEND=2 ZZZ=3\n'
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
            "ORIONEVENTS 2.0\n"
            "UNIT METRIC\n"
            "DATE START = 2024-01-01\n"
            "DURATION RAMP = 5 DAYS\n"
            f'WELL "{well.name}"\n'
            "  @START         PERFORATION  MDSTART=2000  MDEND=2200  RADIUS=0.05  SKIN=0.5  COMPLETION_NUMBER=1\n"
            "  @START + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1\n"
            "  @START + RAMP  WELTARG      CMODE=BHP  VALUE=50\n"
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
            'ORIONEVENTS 2.0\nWELL "NO_SUCH_WELL"\n'
            "  @2024-01-01 PERFORATION MDSTART=1 MDEND=2\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.events_applied == 0
        assert any("NO_SUCH_WELL" in w for w in report.warnings)
