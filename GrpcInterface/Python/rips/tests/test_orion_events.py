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
    coalesce_orion_document,
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
  A1_STARTUP         PERFORATION  MDSTART=1644.49  MDEND=1664.28  DIAMETER=0.2413  SKIN=5  COMPLETION_NUMBER=1
  A1_STARTUP + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1
  A1_STARTUP + RAMP  WELTARG      CMODE=BHP  VALUE=50

WELL "55_33-A-2"
  A2_STARTUP  PERFORATION  MDSTART=1692.79  MDEND=1706  DIAMETER=0.2413  SKIN=5  COMPLETION_NUMBER=1
"""


# ---------------------------------------------------------------------------
# Fakes for the applier tests
# ---------------------------------------------------------------------------


class FakeCompletionSettings:
    def __init__(self):
        self.group_name_for_export = "FIELD"
        self.allow_well_cross_flow = True
        self.reference_depth_for_export = None
        self.well_type_for_export = "OIL"
        self.custom_segment_calls = []

    def add_custom_segment_interval(self, **kwargs):
        self.custom_segment_calls.append(kwargs)


class FakeMswSettings:
    def __init__(self):
        self.pressure_drop = "HF-"
        self.update_calls = 0

    def update(self):
        self.update_calls += 1


class FakeWellPath:
    def __init__(self, name):
        self.name = name
        self._completion_settings = FakeCompletionSettings()
        self._msw_settings = FakeMswSettings()

    def completion_settings(self):
        return self._completion_settings

    def msw_settings(self):
        return self._msw_settings


class FakeProject:
    def __init__(self, names, cases=()):
        self._wells = {name: FakeWellPath(name) for name in names}
        self._cases = list(cases)

    def well_path_by_name(self, name):
        return self._wells.get(name)

    def cases(self):
        return self._cases


class FakePropertyFilter:
    """Bounds default to sentinel 'result range' values, as setToDefaultValues does."""

    def __init__(self, result_variable, result_type):
        self.result_variable = result_variable
        self.result_type = result_type
        self.lower_bound = -1.0e20
        self.upper_bound = 1.0e20
        self.update_calls = 0

    def update(self):
        self.update_calls += 1


class FakeCombinedFilter:
    def __init__(self, name, combine_mode):
        self.name = name
        self.combine_mode = combine_mode
        self.property_filters = []

    def add_property_filter(self, result_variable, result_type):
        property_filter = FakePropertyFilter(result_variable, result_type)
        self.property_filters.append(property_filter)
        return property_filter


class FakeDataFilterCollection:
    def __init__(self):
        self.combined_filters = []

    def add_combined_filter(self, name="", combine_mode="AND"):
        combined = FakeCombinedFilter(name, combine_mode)
        self.combined_filters.append(combined)
        return combined


class FakeCase:
    """A case with canned per-type result names and a data filter collection."""

    _PROPERTIES = {
        "STATIC_NATIVE": ["PORO", "PERMX"],
        "DYNAMIC_NATIVE": ["SOIL", "PRESSURE", "PORO"],
        "GENERATED": [],
    }

    def __init__(self):
        self._data_filter_collection = FakeDataFilterCollection()

    def available_properties(self, property_type):
        return self._PROPERTIES[property_type]

    def data_filter_collection(self):
        return self._data_filter_collection


class FakeTimelineEvent:
    def __init__(self):
        self.comment = ""
        self.update_calls = 0

    def update(self):
        self.update_calls += 1


class FakePerfEvent(FakeTimelineEvent):
    """The object returned by add_perf_event; records attached filters."""

    def __init__(self):
        super().__init__()
        self.filters = []

    def add_filter(self, filter):
        self.filters.append(filter)


class FakeTimeline:
    """Records calls made by the applier instead of contacting ResInsight."""

    def __init__(self):
        self.perf_calls = []
        self.perf_events = []
        self.keyword_calls = []
        self.tubing_calls = []
        self.valve_calls = []
        self.state_calls = []
        self.wellspec_calls = []
        self.schedule_keyword_calls = []
        self.raw_text_calls = []
        self.created_events = []

    def add_perf_event(self, **kwargs):
        self.perf_calls.append(kwargs)
        perf_event = FakePerfEvent()
        self.perf_events.append(perf_event)
        return perf_event

    def _new_event(self):
        event = FakeTimelineEvent()
        self.created_events.append(event)
        return event

    def add_well_keyword_event(self, **kwargs):
        self.keyword_calls.append(kwargs)
        return self._new_event()

    def add_tubing_event(self, **kwargs):
        self.tubing_calls.append(kwargs)
        return self._new_event()

    def add_valve_event(self, **kwargs):
        self.valve_calls.append(kwargs)
        return self._new_event()

    def add_state_event(self, **kwargs):
        self.state_calls.append(kwargs)
        return self._new_event()

    def add_wellspec_event(self, **kwargs):
        self.wellspec_calls.append(kwargs)
        return self._new_event()

    def add_keyword_event(self, **kwargs):
        self.schedule_keyword_calls.append(kwargs)
        return self._new_event()

    def add_raw_text_event(self, **kwargs):
        self.raw_text_calls.append(kwargs)
        return self._new_event()


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
        # A1_STARTUP + RAMP -> 2018-01-06
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
            '  2018-01-01 PERFORATION MDSTART=1 MDEND=2 FILTER="SOIL > 0.8 AND PERMX > 200"\n'
        )
        doc = parse_orion_events(text)
        filter_attr = doc.wells[0].events[0].attributes["FILTER"]
        assert filter_attr.value == "SOIL > 0.8 AND PERMX > 200"
        assert filter_attr.quoted is True
        assert doc.wells[0].events[0].attributes["MDEND"].value == 2

    def test_comment_attribute_is_preserved(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            '  2018-01-01 WCONHIST STATUS=OPEN COMMENT="Startup target"\n'
        )
        event = parse_orion_events(text).wells[0].events[0]
        assert event.attributes["COMMENT"].value == "Startup target"
        assert event.attributes["COMMENT"].quoted is True

    def test_trailing_comment_ignored_but_not_inside_quotes(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            '  2018-01-01 WCONHIST NOTE="a # b" CMODE=ORAT  # trailing comment\n'
        )
        doc = parse_orion_events(text)
        attrs = doc.wells[0].events[0].attributes
        assert attrs["NOTE"].value == "a # b"
        assert "CMODE" in attrs

    def test_iso_date_literal_event(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  2020-12-31 PERFORATION MDSTART=1 MDEND=2\n'
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2020, 12, 31)

    def test_negative_offset(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-10\n"
            'WELL "W"\n  START - 5 PERFORATION MDSTART=1 MDEND=2\n'
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2018, 1, 5)

    def test_minus_after_iso_date(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  2018-01-01 - 5 PERFORATION MDSTART=1 MDEND=2\n'
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.date(2017, 12, 27)

    def test_offset_chain_with_duration_variable(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-01\nDURATION RAMP = 5\n"
            'WELL "W"\n  START + RAMP - 2 PERFORATION MDSTART=1 MDEND=2\n'
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
        with pytest.raises(OrionParseError, match="before any WELL or SCHEDULE block"):
            parse_orion_events(
                "ORIONEVENTS 2.0\n  2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
            )

    def test_unknown_variable_raises(self):
        with pytest.raises(OrionParseError, match="Unknown variable 'NOPE'"):
            parse_orion_events(
                'ORIONEVENTS 2.0\nWELL "W"\n  NOPE PERFORATION MDSTART=1 MDEND=2\n'
            )

    def test_duration_where_date_expected_raises(self):
        text = (
            "ORIONEVENTS 2.0\nDURATION RAMP = 5\n"
            'WELL "W"\n  RAMP WCONHIST STATUS=OPEN\n'
        )
        with pytest.raises(
            OrionParseError, match=r"is a DURATION \(declared line 2\) but a DATE"
        ):
            parse_orion_events(text)

    def test_date_where_duration_expected_raises(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2018-01-01\nDATE OTHER = 2018-02-01\n"
            'WELL "W"\n  START + OTHER WCONHIST STATUS=OPEN\n'
        )
        with pytest.raises(OrionParseError, match="is a DATE .* but a DURATION"):
            parse_orion_events(text)

    def test_well_variable_in_date_context_raises(self):
        text = 'ORIONEVENTS 2.0\nWELL A1 = "X"\nWELL A1\n  A1 WCONHIST STATUS=OPEN\n'
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
            'ORIONEVENTS 2.0\nWELL "W"\n  START PERFORATION MDSTART=1 MDEND=2\n'
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
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  2018-01-01 PERFORATION MDSTART=1 bogus MDEND=2\n'
        with pytest.raises(OrionParseError, match="Malformed attribute"):
            parse_orion_events(text)

    def test_duplicate_date_declaration_warns(self):
        text = 'ORIONEVENTS 2.0\nDATE X = 2018-01-01\nDATE X = 2019-01-01\nWELL "W"\n'
        doc = parse_orion_events(text)
        assert any("Duplicate" in w.message for w in doc.warnings)
        assert doc.variables["X"].value == datetime.date(2019, 1, 1)

    def test_schedule_block_parses(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN\n"
            "SCHEDULE\n"
            "  2024-01-01 RPTRST BASIC=2 FREQ=1\n"
            "  2024-01-01 GRUPTREE CHILD_GROUP=OP PARENT_GROUP=FIELD\n"
            'WELL "W"\n'
            "  2024-02-01 WELTARG CMODE=ORAT VALUE=5000\n"
        )
        doc = parse_orion_events(text)
        assert [e.event_type for e in doc.schedule_events] == ["RPTRST", "GRUPTREE"]
        assert doc.schedule_events[0].attributes["BASIC"].value == 2
        # WELL after SCHEDULE switches the sink back to the well block.
        assert [len(w.events) for w in doc.wells] == [1, 1]

    def test_group_blocks_parse_and_switch_event_sink(self):
        text = (
            'ORIONEVENTS 2.0\nGROUP "OP"\n'
            "  2020-07-01 GEFAC FACTOR=1.0 TRANSFER=YES\n"
            "  2020-07-01 GCONPROD CMODE=LRAT LRAT=20000\n"
            'GROUP "WI"\n'
            "  2020-07-01 GCONINJE TYPE=WATER CMODE=RATE RATE=16000\n"
            "SCHEDULE\n"
            "  2020-07-01 RPTRST BASIC=2\n"
        )
        doc = parse_orion_events(text)
        assert [group.group_name for group in doc.groups] == ["OP", "WI"]
        assert [event.event_type for event in doc.groups[0].events] == [
            "GEFAC",
            "GCONPROD",
        ]
        assert [event.event_type for event in doc.groups[1].events] == ["GCONINJE"]
        assert [event.event_type for event in doc.schedule_events] == ["RPTRST"]

    def test_empty_group_block_ok(self):
        doc = parse_orion_events('ORIONEVENTS 2.0\nGROUP "OP"\n')
        assert doc.groups[0].group_name == "OP"
        assert doc.groups[0].events == []

    def test_malformed_group_line_rejected(self):
        with pytest.raises(OrionParseError, match="Malformed GROUP line"):
            parse_orion_events("ORIONEVENTS 2.0\nGROUP OP\n")

    def test_duplicate_wellspec_for_well_and_date_is_rejected(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            "  2024-01-01 WELSPECS GROUP=A\n"
            'WELL "W"\n'
            "  2024-01-01 WELSPECS PHASE=GAS\n"
        )
        with pytest.raises(OrionParseError, match="WELSPECS already defined.*line 3"):
            parse_orion_events(text)

    def test_wellspec_same_date_for_different_wells_is_allowed(self):
        document = parse_orion_events(
            'ORIONEVENTS 2.0\nWELL "A"\n'
            "  2024-01-01 WELSPECS GROUP=GA\n"
            'WELL "B"\n'
            "  2024-01-01 WELSPECS GROUP=GB\n"
        )
        assert [len(well.events) for well in document.wells] == [1, 1]

    def test_boolean_attributes_are_typed_unless_quoted(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "SCHEDULE\n"
            '  2024-01-01 RPTRST DEN=True ROCKC=FALSE LABEL="True"\n'
        )
        attributes = parse_orion_events(text).schedule_events[0].attributes

        assert attributes["DEN"].value is True
        assert attributes["ROCKC"].value is False
        assert attributes["LABEL"].value == "True"
        assert isinstance(attributes["LABEL"].value, str)

    def test_single_restart_event_parses(self):
        document = parse_orion_events(
            "ORIONEVENTS 2.0\nSCHEDULE\n  2024-02-01 RESTART\n"
        )
        restart = document.schedule_events[0]
        assert restart.event_type == "RESTART"
        assert restart.attributes == {}

    @pytest.mark.parametrize(
        "text,expected_error",
        [
            (
                'ORIONEVENTS 2.0\nWELL "W"\n  2024-01-01 RESTART\n',
                "only valid in a SCHEDULE block",
            ),
            (
                'ORIONEVENTS 2.0\nGROUP "G"\n  2024-01-01 RESTART\n',
                "only valid in a SCHEDULE block",
            ),
            (
                "ORIONEVENTS 2.0\nSCHEDULE\n  2024-01-01 RESTART VALUE=1\n",
                "takes no attributes",
            ),
            (
                "ORIONEVENTS 2.0\nSCHEDULE\n"
                "  2024-01-01 RESTART\n"
                "  2024-02-01 RESTART\n",
                "Only one RESTART event",
            ),
        ],
    )
    def test_invalid_restart_event_rejected(self, text, expected_error):
        with pytest.raises(OrionParseError, match=expected_error):
            parse_orion_events(text)

    def test_schedule_line_with_arguments_rejected(self):
        with pytest.raises(OrionParseError, match="SCHEDULE takes no arguments"):
            parse_orion_events("ORIONEVENTS 2.0\nSCHEDULE NOW\n")

    def test_raw_text_block_preserves_body_and_attributes(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=BEFORE_KEYWORD "
            "ANCHOR=COMPDAT PRIORITY=-2\n"
            "# not an ORION comment\n"
            "  this is raw too\n"
            "END\n"
            "END_RAW_TEXT\n"
        )
        event = parse_orion_events(text).schedule_events[0]

        assert event.event_type == "RAW_TEXT"
        assert event.raw_text == "# not an ORION comment\n  this is raw too\nEND\n"
        assert event.raw_placement == "BEFORE_KEYWORD"
        assert event.raw_anchor == "COMPDAT"
        assert event.raw_priority == -2

    @pytest.mark.parametrize(
        "header,error",
        [
            ("RAW_TEXT", "requires PLACEMENT"),
            ("RAW_TEXT PLACEMENT=NOPE", "PLACEMENT must be"),
            (
                "RAW_TEXT PLACEMENT=BEFORE_KEYWORD",
                "ANCHOR is required",
            ),
            (
                "RAW_TEXT PLACEMENT=AFTER_DATE ANCHOR=COMPDAT",
                "ANCHOR is only valid",
            ),
            (
                'RAW_TEXT PLACEMENT=END_OF_DATE PRIORITY="1"',
                "PRIORITY must be an integer",
            ),
            (
                "RAW_TEXT PLACEMENT=END_OF_DATE EXTRA=1",
                "Unknown RAW_TEXT attribute",
            ),
        ],
    )
    def test_invalid_raw_text_header_rejected(self, header, error):
        text = f"ORIONEVENTS 2.0\nSCHEDULE\n  2024-01-01 {header}\nx\nEND_RAW_TEXT\n"
        with pytest.raises(OrionParseError, match=error):
            parse_orion_events(text)

    def test_raw_text_outside_schedule_rejected(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE\n"
            "x\nEND_RAW_TEXT\n"
        )
        with pytest.raises(OrionParseError, match="only valid in a SCHEDULE"):
            parse_orion_events(text)

    def test_unterminated_raw_text_rejected(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE\ntext\n"
        )
        with pytest.raises(OrionParseError, match="Unterminated RAW_TEXT"):
            parse_orion_events(text)

    def test_report_lines_parse(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "DATE START = 2024-01-01\n"
            "SCHEDULE\n"
            "INSERT_DATE 2024-06-01\n"
            "INSERT_DATE START + 31\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.date(2024, 6, 1),
            datetime.date(2024, 2, 1),
        ]

    def test_report_keeps_duplicates_and_file_order(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "INSERT_DATE 2024-06-01\nINSERT_DATE 2024-06-01\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [datetime.date(2024, 6, 1)] * 2

    def test_insert_date_is_only_valid_in_schedule_block(self):
        with pytest.raises(OrionParseError, match="only valid in a SCHEDULE"):
            parse_orion_events("ORIONEVENTS 2.0\nINSERT_DATE 2024-06-01\n")

    def test_legacy_report_keyword_is_rejected(self):
        with pytest.raises(OrionParseError, match="renamed to INSERT_DATE"):
            parse_orion_events("ORIONEVENTS 2.0\nSCHEDULE\nREPORT 2024-06-01\n")

    def test_report_with_undeclared_variable_raises(self):
        with pytest.raises(OrionParseError, match="NOPE"):
            parse_orion_events("ORIONEVENTS 2.0\nSCHEDULE\nINSERT_DATE NOPE + 1\n")

    def test_malformed_report_line_raises(self):
        with pytest.raises(OrionParseError, match="Malformed INSERT_DATE line"):
            parse_orion_events("ORIONEVENTS 2.0\nSCHEDULE\nINSERT_DATE\n")

    def test_report_with_datetime_literal(self):
        text = "ORIONEVENTS 2.0\nSCHEDULE\nINSERT_DATE 2024-06-01T14:45:30.500\n"
        doc = parse_orion_events(text)
        assert doc.report_dates == [datetime.datetime(2024, 6, 1, 14, 45, 30, 500000)]

    def test_daily_report_recurrence_with_inclusive_until(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "DATE START = 2024-01-01\n"
            "DATE END = 2024-01-05\n"
            "SCHEDULE\n"
            "INSERT_DATE START + 1 EVERY 2 DAYS UNTIL END\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.date(2024, 1, 2),
            datetime.date(2024, 1, 4),
        ]

    def test_monthly_report_recurrence_is_anchored_to_initial_day(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "INSERT_DATE 2024-01-31 EVERY MONTH UNTIL 2024-04-30\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.date(2024, 1, 31),
            datetime.date(2024, 2, 29),
            datetime.date(2024, 3, 31),
            datetime.date(2024, 4, 30),
        ]

    def test_yearly_report_recurrence_clamps_leap_day(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "INSERT_DATE 2024-02-29 EVERY YEAR UNTIL 2028-02-29\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.date(2024, 2, 29),
            datetime.date(2025, 2, 28),
            datetime.date(2026, 2, 28),
            datetime.date(2027, 2, 28),
            datetime.date(2028, 2, 29),
        ]

    def test_report_recurrence_without_until_uses_last_event(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "SCHEDULE\n"
            "INSERT_DATE 2024-01-01 EVERY MONTH\n"
            'WELL "W"\n'
            "  2024-03-15 WCONHIST STATUS=OPEN\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.date(2024, 1, 1),
            datetime.date(2024, 2, 1),
            datetime.date(2024, 3, 1),
        ]

    def test_recurring_report_preserves_datetime(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "SCHEDULE\n"
            "INSERT_DATE 2024-06-01T14:45:30.500 EVERY DAY "
            "UNTIL 2024-06-02T14:45:30.500\n"
        )
        doc = parse_orion_events(text)
        assert doc.report_dates == [
            datetime.datetime(2024, 6, 1, 14, 45, 30, 500000),
            datetime.datetime(2024, 6, 2, 14, 45, 30, 500000),
        ]

    @pytest.mark.parametrize(
        ("report_line", "message"),
        [
            (
                "INSERT_DATE 2024-01-01 EVERY 0 DAYS UNTIL 2024-01-02",
                "greater than zero",
            ),
            ("INSERT_DATE 2024-01-02 EVERY DAY UNTIL 2024-01-01", "must not precede"),
            ("INSERT_DATE 2024-01-01 EVERY DAY", "requires at least one event"),
        ],
    )
    def test_invalid_report_recurrence_rejected(self, report_line, message):
        with pytest.raises(OrionParseError, match=message):
            parse_orion_events(f"ORIONEVENTS 2.0\nSCHEDULE\n{report_line}\n")

    def test_event_dates_parse_without_prefix(self):
        text = (
            "ORIONEVENTS 2.0\nDATE START = 2024-05-15\n"
            'WELL "W"\n'
            "  START + 1 PERFORATION MDSTART=1 MDEND=2\n"
            "  2024-05-17 WCONHIST STATUS=OPEN\n"
        )
        doc = parse_orion_events(text)
        assert [event.event_date for event in doc.wells[0].events] == [
            datetime.date(2024, 5, 16),
            datetime.date(2024, 5, 17),
        ]

    def test_at_prefix_is_rejected(self):
        text = 'ORIONEVENTS 2.0\nWELL "W"\n  @2024-05-17 WCONHIST STATUS=OPEN\n'
        with pytest.raises(OrionParseError, match="Unrecognized line"):
            parse_orion_events(text)

    def test_datetime_literal_event(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            "  2024-05-15T14:45:30.500 PERFORATION MDSTART=1 MDEND=2\n"
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.datetime(
            2024, 5, 15, 14, 45, 30, 500000
        )

    def test_datetime_with_day_offset(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            "  2024-05-15T14:45:30 + 2 PERFORATION MDSTART=1 MDEND=2\n"
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.datetime(
            2024, 5, 17, 14, 45, 30
        )

    def test_date_variable_with_time_of_day(self):
        text = (
            "ORIONEVENTS 2.0\nDATE T0 = 2024-05-15T14:45:30.500\n"
            'WELL "W"\n  T0 + 1 PERFORATION MDSTART=1 MDEND=2\n'
        )
        doc = parse_orion_events(text)
        assert doc.wells[0].events[0].event_date == datetime.datetime(
            2024, 5, 16, 14, 45, 30, 500000
        )


class TestFilterParsing:
    def _perf_with_filter(self, decls, filter_value):
        text = (
            "ORIONEVENTS 2.0\n"
            + decls
            + 'WELL "W"\n'
            + f"  2018-01-01 PERFORATION MDSTART=1 MDEND=2 FILTER={filter_value}\n"
        )
        return parse_orion_events(text)

    def test_filter_declaration_single_term(self):
        doc = parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "poro > 0.4"\n')
        value = doc.variables["F"]
        assert value.kind == "FILTER"
        expr = value.value
        assert expr.combine_mode == "AND"
        assert len(expr.terms) == 1
        term = expr.terms[0]
        assert term.result_name == "PORO"  # uppercased
        assert term.result_type is None
        assert term.op == ">"
        assert term.value == 0.4

    def test_filter_declaration_two_terms_and(self):
        doc = parse_orion_events(
            'ORIONEVENTS 2.0\nFILTER F = "poro>0.4 AND permx > 100.0"\n'
        )
        expr = doc.variables["F"].value
        assert expr.combine_mode == "AND"
        assert [t.result_name for t in expr.terms] == ["PORO", "PERMX"]

    def test_filter_declaration_or_mode(self):
        doc = parse_orion_events(
            'ORIONEVENTS 2.0\nFILTER F = "PORO < 0.1 OR PERMX <= 10"\n'
        )
        expr = doc.variables["F"].value
        assert expr.combine_mode == "OR"
        assert [t.op for t in expr.terms] == ["<", "<="]

    def test_filter_all_operators(self):
        doc = parse_orion_events(
            'ORIONEVENTS 2.0\nFILTER F = "A > 1 AND B >= 2 AND C < 3 AND D <= 4"\n'
        )
        expr = doc.variables["F"].value
        assert [t.op for t in expr.terms] == [">", ">=", "<", "<="]
        assert [t.value for t in expr.terms] == [1.0, 2.0, 3.0, 4.0]

    def test_filter_scientific_notation_value(self):
        doc = parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "PERMX < 1.5e4"\n')
        assert doc.variables["F"].value.terms[0].value == 15000.0

    def test_filter_qualified_result_type(self):
        doc = parse_orion_events(
            'ORIONEVENTS 2.0\nFILTER F = "DYNAMIC_NATIVE.MY_PROPERTY > 1"\n'
        )
        term = doc.variables["F"].value.terms[0]
        assert term.result_type == "DYNAMIC_NATIVE"
        assert term.result_name == "MY_PROPERTY"

    def test_filter_qualifier_alias_case_insensitive(self):
        for qualifier, expected in (
            ("dynamic", "DYNAMIC_NATIVE"),
            ("static", "STATIC_NATIVE"),
            ("Generated", "GENERATED"),
            ("static_native", "STATIC_NATIVE"),
        ):
            doc = parse_orion_events(
                f'ORIONEVENTS 2.0\nFILTER F = "{qualifier}.my_property > 1"\n'
            )
            term = doc.variables["F"].value.terms[0]
            assert term.result_type == expected
            assert term.result_name == "MY_PROPERTY"

    def test_perforation_filter_reference_resolves(self):
        doc = self._perf_with_filter('FILTER poroperm = "poro>0.4"\n', "poroperm")
        event = doc.wells[0].events[0]
        assert event.filter is not None
        assert event.filter.name == "poroperm"
        assert event.filter.expr.terms[0].result_name == "PORO"
        # The attribute dict stays lossless.
        assert event.attributes["FILTER"].value == "poroperm"

    def test_perforation_inline_filter_expression(self):
        doc = self._perf_with_filter("", '"permx > 100 OR poro < 0.1"')
        event = doc.wells[0].events[0]
        assert event.filter is not None
        assert event.filter.name is None
        assert event.filter.expr.combine_mode == "OR"
        assert [t.result_name for t in event.filter.expr.terms] == ["PERMX", "PORO"]

    def test_filter_on_non_perforation_event_is_plain_attribute(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "W"\n'
            '  2018-01-01 WCONHIST STATUS=OPEN FILTER="PERMX > 200"\n'
        )
        doc = parse_orion_events(text)
        event = doc.wells[0].events[0]
        assert event.filter is None
        assert event.attributes["FILTER"].value == "PERMX > 200"

    def test_mixed_and_or_raises(self):
        with pytest.raises(OrionParseError, match="mixes AND and OR"):
            parse_orion_events(
                'ORIONEVENTS 2.0\nFILTER F = "A > 1 AND B > 2 OR C > 3"\n'
            )

    def test_lowercase_connector_raises(self):
        with pytest.raises(OrionParseError, match="uppercase AND / OR"):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "poro>0.4 and permx>1"\n')

    def test_equality_operator_raises(self):
        with pytest.raises(OrionParseError, match="only >, >=, < and <="):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "poro = 0.4"\n')

    def test_function_style_result_name_raises(self):
        with pytest.raises(OrionParseError, match="Malformed filter term"):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "SOIL(0) > 0.8"\n')

    def test_non_numeric_value_raises(self):
        with pytest.raises(OrionParseError, match="Malformed filter term"):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "poro > high"\n')

    def test_empty_filter_expression_raises(self):
        with pytest.raises(OrionParseError, match="Empty filter expression"):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = ""\n')

    def test_unknown_qualifier_raises_with_hint(self):
        with pytest.raises(
            OrionParseError, match="Unknown result type 'DYNAMIK'.*did you mean"
        ):
            parse_orion_events('ORIONEVENTS 2.0\nFILTER F = "DYNAMIK.PORO > 1"\n')

    def test_malformed_filter_declaration_raises(self):
        with pytest.raises(OrionParseError, match="Malformed FILTER declaration"):
            parse_orion_events("ORIONEVENTS 2.0\nFILTER F = poro>0.4\n")

    def test_undeclared_filter_reference_raises_with_hint(self):
        with pytest.raises(OrionParseError, match="did you mean 'poroperm'"):
            self._perf_with_filter('FILTER poroperm = "poro>1"\n', "poropermm")

    def test_wrong_kind_filter_reference_raises(self):
        with pytest.raises(OrionParseError, match="is a DATE .* but a FILTER"):
            self._perf_with_filter("DATE X = 2018-01-01\n", "X")

    def test_filter_variable_in_date_context_raises(self):
        text = (
            'ORIONEVENTS 2.0\nFILTER F = "poro>1"\nWELL "W"\n'
            "  F PERFORATION MDSTART=1 MDEND=2\n"
        )
        with pytest.raises(OrionParseError, match="is a FILTER .* but a DATE"):
            parse_orion_events(text)

    def test_non_identifier_filter_value_raises(self):
        with pytest.raises(OrionParseError, match="must name a declared FILTER"):
            self._perf_with_filter("", "123")

    def test_duplicate_filter_declaration_warns(self):
        text = 'ORIONEVENTS 2.0\nFILTER F = "poro>1"\nFILTER F = "permx>2"\n'
        doc = parse_orion_events(text)
        assert any("Duplicate FILTER" in w.message for w in doc.warnings)
        assert doc.variables["F"].value.terms[0].result_name == "PERMX"


class TestDiagnostics:
    def test_all_errors_reported_in_one_pass(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "DATE A1_STARTUP = 2018-01-01\n"
            "SET X = 2018-01-01\n"  # line 3
            'WELL "W"\n'
            "  A1_STRTUP PERFORATION MDSTART=1 MDEND=2\n"  # line 5
            "  A1_STARTUP + NOPE WCONHIST STATUS=OPEN\n"  # line 6
        )
        with pytest.raises(OrionParseError) as excinfo:
            parse_orion_events(text)
        assert [issue.loc.line for issue in excinfo.value.errors] == [3, 5, 6]

    def test_malformed_well_block_suppresses_cascading_errors(self):
        text = (
            "ORIONEVENTS 2.0\n"
            "WELL 55_33-A-2\n"  # malformed: unquoted special characters
            "  2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
            "  2018-01-01 WCONHIST STATUS=OPEN\n"
        )
        with pytest.raises(OrionParseError) as excinfo:
            parse_orion_events(text)
        assert len(excinfo.value.errors) == 1
        assert "Malformed WELL line" in excinfo.value.errors[0].message

    def test_unknown_variable_hint(self):
        text = (
            "ORIONEVENTS 2.0\nDATE A1_STARTUP = 2018-01-01\n"
            'WELL "W"\n  A1_STRTUP PERFORATION MDSTART=1 MDEND=2\n'
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
        assert "4 variable(s), 2 well block(s), 4 well event(s)" in out

    def test_invalid_file_exits_nonzero_with_errors(self, tmp_path, capsys):
        path = self._write(
            tmp_path,
            'ORIONEVENTS 2.0\nSET X = 2018-01-01\nWELL "W"\n  NOPE WCONHIST A=1\n',
        )
        assert _cli([path]) == 1
        out = capsys.readouterr().out
        assert "Line 2" in out
        assert "Line 4" in out
        assert "2 error(s) found" in out

    def test_missing_file_exits_nonzero(self, tmp_path, capsys):
        assert _cli([str(tmp_path / "nope.orion")]) == 1
        assert "Error" in capsys.readouterr().out


class FakeWellPathCollection:
    def __init__(self, timeline):
        self._timeline = timeline

    def event_timeline(self):
        return self._timeline


class FakeCliProject(FakeProject):
    """A project whose well path collections are discoverable via descendants."""

    def __init__(self, names, cases=(), collections=()):
        super().__init__(names, cases)
        self._collections = list(collections)

    def descendants(self, cls):
        return self._collections


class FakeInstance:
    def __init__(self, project):
        self.project = project


class TestApplyCli:
    def _write(self, tmp_path, text):
        path = tmp_path / "events.orion"
        path.write_text(text)
        return str(path)

    def test_apply_exits_zero_and_reports_applied_events(
        self, tmp_path, capsys, monkeypatch
    ):
        timeline = FakeTimeline()
        project = FakeCliProject(
            ("55_33-A-1", "55_33-A-2"),
            collections=[FakeWellPathCollection(timeline)],
        )
        monkeypatch.setattr(
            rips.Instance, "find", staticmethod(lambda: FakeInstance(project))
        )
        path = self._write(tmp_path, SAMPLE)
        assert _cli([path, "--apply"]) == 0
        out = capsys.readouterr().out
        assert "Events applied: 4" in out
        assert len(timeline.perf_calls) == 2
        assert len(timeline.keyword_calls) == 2

    def test_apply_without_running_instance_exits_nonzero(
        self, tmp_path, capsys, monkeypatch
    ):
        monkeypatch.setattr(rips.Instance, "find", staticmethod(lambda: None))
        path = self._write(tmp_path, SAMPLE)
        assert _cli([path, "--apply"]) == 1
        assert "no running ResInsight instance" in capsys.readouterr().out

    def test_apply_without_well_path_collection_exits_nonzero(
        self, tmp_path, capsys, monkeypatch
    ):
        project = FakeCliProject(("55_33-A-1",), collections=[])
        monkeypatch.setattr(
            rips.Instance, "find", staticmethod(lambda: FakeInstance(project))
        )
        path = self._write(tmp_path, SAMPLE)
        assert _cli([path, "--apply"]) == 1
        assert "no well path collection" in capsys.readouterr().out

    def test_validate_only_does_not_contact_instance(
        self, tmp_path, capsys, monkeypatch
    ):
        def fail():
            raise AssertionError("Instance.find must not be called")

        monkeypatch.setattr(rips.Instance, "find", staticmethod(fail))
        path = self._write(tmp_path, SAMPLE)
        assert _cli([path]) == 0
        assert "OK" in capsys.readouterr().out


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

    def test_same_owner_type_and_date_events_are_merged(self):
        text = (
            "ORIONEVENTS 2.0\n"
            'WELL "55_33-A-1"\n'
            "  2018-01-01 WCONHIST STATUS=OPEN ORAT=100\n"
            'WELL "55_33-A-1"\n'
            "  2018-01-01 wconhist STATUS=SHUT CMODE=ORAT\n"
            "  2018-01-02 WCONHIST STATUS=OPEN\n"
            'WELL "55_33-A-2"\n'
            "  2018-01-01 WCONHIST STATUS=OPEN\n"
        )
        document = parse_orion_events(text)
        merged = coalesce_orion_document(document)

        # Normalization does not mutate the parsed source representation.
        assert len(document.wells) == 3
        assert len(merged.wells) == 2
        assert len(merged.wells[0].events) == 2

        merged_event = merged.wells[0].events[0]
        assert merged_event.event_type == "WCONHIST"
        assert merged_event.attributes["STATUS"].value == "SHUT"
        assert merged_event.attributes["ORAT"].value == 100
        assert merged_event.attributes["CMODE"].value == "ORAT"

        timeline, report = self._apply(text)
        assert report.events_applied == 3
        assert len(timeline.keyword_calls) == 3
        assert timeline.keyword_calls[0]["keyword_data"]["STATUS"] == "SHUT"
        assert timeline.keyword_calls[0]["keyword_data"]["ORAT"] == 100
        assert timeline.keyword_calls[0]["keyword_data"]["CMODE"] == "ORAT"

    def test_well_keyword_history_is_inherited_chronologically(self):
        text = (
            "ORIONEVENTS 2.0\n"
            'WELL "55_33-A-1"\n'
            "  2024-01-20 WCONHIST WRAT=0.03\n"
            '  2024-01-15 WCONHIST STATUS=OPEN CMODE=RESV GRAT=4756545.5 COMMENT="Startup"\n'
            "  2024-01-15 WCONHIST ORAT=3999.99 WRAT=0.01 GRAT=550678.44 VFP=1\n"
            "  2024-01-15 WELTARG CMODE=ORAT VALUE=5000\n"
            'WELL "55_33-A-2"\n'
            "  2024-01-20 WCONHIST WRAT=0.04\n"
        )
        document = parse_orion_events(text)
        merged = coalesce_orion_document(document)

        first_well_events = merged.wells[0].events
        january_15 = next(
            event
            for event in first_well_events
            if event.event_type.upper() == "WCONHIST"
            and event.event_date.isoformat() == "2024-01-15"
        )
        january_20 = next(
            event
            for event in first_well_events
            if event.event_type.upper() == "WCONHIST"
            and event.event_date.isoformat() == "2024-01-20"
        )

        assert january_15.attributes["GRAT"].value == 550678.44
        assert january_15.attributes["COMMENT"].value == "Startup"
        assert january_20.attributes["STATUS"].value == "OPEN"
        assert january_20.attributes["CMODE"].value == "RESV"
        assert january_20.attributes["ORAT"].value == 3999.99
        assert january_20.attributes["WRAT"].value == 0.03
        assert january_20.attributes["GRAT"].value == 550678.44
        assert january_20.attributes["VFP"].value == 1
        assert "COMMENT" not in january_20.attributes

        # Historical state is isolated by owner and keyword type.
        assert set(merged.wells[1].events[0].attributes) == {"WRAT"}
        weltarg = next(
            event
            for event in first_well_events
            if event.event_type.upper() == "WELTARG"
        )
        assert set(weltarg.attributes) == {"CMODE", "VALUE"}

        assert len(merged.warnings) == 1
        warning = merged.warnings[0]
        assert warning.loc.line == 5
        assert "conflicting WCONHIST attribute 'GRAT'" in warning.message
        assert "previous value on line 4" in warning.message
        assert "using '550678.44'" in warning.message

        timeline, report = self._apply(text)
        assert report.events_applied == 4
        assert len(report.warnings) == 1
        assert "conflicting WCONHIST attribute 'GRAT'" in report.warnings[0]
        january_20_call = timeline.keyword_calls[0]
        assert january_20_call["event_date"] == "2024-01-20"
        assert january_20_call["keyword_data"]["STATUS"] == "OPEN"
        assert january_20_call["keyword_data"]["WRAT"] == 0.03

    def test_same_date_perforations_are_not_merged(self):
        text = (
            "ORIONEVENTS 2.0\n"
            'WELL "55_33-A-1"\n'
            "  2018-01-01 PERFORATION MDSTART=1000 MDEND=1100 COMPLETION_NUMBER=1\n"
            "  2018-01-01 PERFORATION MDSTART=1200 MDEND=1300 COMPLETION_NUMBER=2\n"
        )

        document = parse_orion_events(text)
        merged = coalesce_orion_document(document)

        assert len(merged.wells[0].events) == 2

        timeline, report = self._apply(text)
        assert report.events_applied == 2
        assert report.events_skipped == 0
        assert [(call["start_md"], call["end_md"]) for call in timeline.perf_calls] == [
            (1000.0, 1100.0),
            (1200.0, 1300.0),
        ]
        assert [call["completion_number"] for call in timeline.perf_calls] == [1, 2]

    def test_group_and_schedule_events_merge_only_within_owner(self):
        text = (
            "ORIONEVENTS 2.0\n"
            'GROUP "OP"\n'
            "  2018-01-01 GCONPROD CONTROL_MODE=ORAT\n"
            'GROUP "OP"\n'
            "  2018-01-01 GCONPROD OIL_TARGET=100\n"
            'GROUP "OTHER"\n'
            "  2018-01-01 GCONPROD OIL_TARGET=200\n"
            "SCHEDULE\n"
            "  2018-01-01 RPTRST BASIC=1\n"
            "SCHEDULE\n"
            "  2018-01-01 RPTRST FREQ=2\n"
        )
        merged = coalesce_orion_document(parse_orion_events(text))

        assert len(merged.groups) == 2
        assert len(merged.groups[0].events) == 1
        assert set(merged.groups[0].events[0].attributes) == {
            "CONTROL_MODE",
            "OIL_TARGET",
        }
        assert len(merged.groups[1].events) == 1
        assert len(merged.schedule_events) == 1
        assert set(merged.schedule_events[0].attributes) == {"BASIC", "FREQ"}

    def test_perforation_maps_diameter_directly(self):
        timeline, report = self._apply(SAMPLE)
        assert report.events_applied == 4  # 2 perfs + WCONHIST + WELTARG
        first = timeline.perf_calls[0]
        assert first["start_md"] == 1644.49
        assert first["end_md"] == 1664.28
        assert first["diameter"] == pytest.approx(0.2413)
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

    def test_comment_is_applied_to_timeline_event_not_keyword_data(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            '  2018-01-01 WCONHIST STATUS=OPEN COMMENT="Startup target"\n'
        )
        timeline, report = self._apply(text)

        assert report.errors == []
        assert "COMMENT" not in timeline.keyword_calls[0]["keyword_data"]
        assert timeline.created_events[0].comment == "Startup target"
        assert timeline.created_events[0].update_calls == 1

    def test_perforation_comment_is_applied(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2018-01-01 PERFORATION MDSTART=1 MDEND=2 COMMENT=Interval\n"
        )
        timeline, report = self._apply(text)

        assert report.errors == []
        assert timeline.perf_events[0].comment == "Interval"
        assert timeline.perf_events[0].update_calls == 1

    def test_weltarg_value_translation(self):
        timeline, _ = self._apply(SAMPLE)
        weltarg = next(
            c for c in timeline.keyword_calls if c["keyword_name"] == "WELTARG"
        )
        data = weltarg["keyword_data"]
        assert data["NEW_VALUE"] == 50  # VALUE -> NEW_VALUE
        assert data["CMODE"] == "BHP"

    def test_keyword_attributes_forward_without_special_casing(self):
        # DSHIFT is not part of the format; it forwards unchanged like any
        # other attribute instead of being stripped.
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2018-01-01 WCONHIST STATUS=OPEN CMODE=ORAT DSHIFT=10\n"
        )
        timeline, report = self._apply(text)
        data = timeline.keyword_calls[0]["keyword_data"]
        assert data["DSHIFT"] == 10
        assert timeline.keyword_calls[0]["event_date"] == "2018-01-01"
        assert not report.warnings

    def test_report_dates_on_apply_report(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2018-01-01 WCONHIST STATUS=OPEN\n"
            "SCHEDULE\n"
            "INSERT_DATE 2018-07-01\n"
            "INSERT_DATE 2018-03-01\n"
            "INSERT_DATE 2018-07-01\n"
        )
        timeline, report = self._apply(text)
        # Sorted, deduplicated ISO strings ready for
        # generate_schedule_text(additional_dates=...). No timeline events.
        assert report.report_dates == ["2018-03-01", "2018-07-01"]
        assert report.events_applied == 1

    def test_radius_on_perforation_is_unknown_attribute_error(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2018-01-01 PERFORATION MDSTART=1 MDEND=2 RADIUS=0.1\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert any("RADIUS" in error for error in report.errors)
        assert not timeline.perf_calls

    def test_perfid_on_perforation_is_unknown_attribute_error(self):
        # PERFID is not part of the format; it is rejected like any other
        # unknown completion attribute.
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2018-01-01 PERFORATION MDSTART=1 MDEND=2 PERFID=Valysar\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert any("PERFID" in e for e in report.errors)
        assert not timeline.perf_calls

    def test_filter_on_keyword_event_warns_and_applies(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            '  2018-01-01 WCONHIST STATUS=OPEN FILTER="PERMX > 200"\n'
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
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2018-01-01 WCONHST STATUS=OPEN\n'
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any(
            "WCONHST" in w and "did you mean 'WCONHIST'" in w for w in report.warnings
        )

    def test_perforation_missing_required_attr_is_error(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2018-01-01 PERFORATION MDSTART=1\n'
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("MDEND" in e for e in report.errors)
        assert 'Line 3 [WELL "55_33-A-1", date 2018-01-01]' in report.errors[0]

    def test_perforation_unknown_attr_is_error(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2018-01-01 PERFORATION MDSTART=1 MDEND=2 ZZZ=3\n'
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("ZZZ" in e for e in report.errors)

    def test_wellspec_partial_updates_are_cumulative_by_date(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2019-01-01 WELSPECS CROSSFLOW=False PHASE=gas\n"
            "  2018-01-01 WELSPECS GROUP=my_group REFDEPTH=1002 PHASE=water\n"
        )
        timeline, report = self._apply(text)

        assert report.errors == []
        assert report.events_applied == 2
        # Calls retain source order, but snapshots are resolved chronologically.
        assert timeline.wellspec_calls[0] == {
            "event_date": "2019-01-01",
            "well_path": timeline.wellspec_calls[0]["well_path"],
            "group_name": "my_group",
            "allow_cross_flow": False,
            "reference_depth": 1002.0,
            "well_type": "GAS",
        }
        assert timeline.wellspec_calls[1]["group_name"] == "my_group"
        assert timeline.wellspec_calls[1]["allow_cross_flow"] is True
        assert timeline.wellspec_calls[1]["well_type"] == "WATER"

    @pytest.mark.parametrize(
        "attributes,expected_error",
        [
            ("CROSSFLOW=YES", "CROSSFLOW must be True or False"),
            ("REFDEPTH=deep", "REFDEPTH must be numeric"),
            ("PHASE=steam", "PHASE must be OIL, GAS, WATER, or LIQUID"),
            ("GROUP=1", "GROUP must be a non-empty string"),
            ("UNKNOWN=1", "unknown WELSPECS attribute"),
            ("COMMENT=empty", "needs at least one setting attribute"),
        ],
    )
    def test_invalid_wellspec_is_reported_and_skipped(self, attributes, expected_error):
        text = (
            f'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2018-01-01 WELSPECS {attributes}\n'
        )
        timeline, report = self._apply(text)

        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert expected_error in report.errors[0]
        assert timeline.wellspec_calls == []

    def test_segment_mapping_creates_custom_interval_and_sets_pressure_drop(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-01-01 SEGMENT MDSTART=0 MDEND=2500 INNER_DIAMETER=0.15 "
            "ROUGHNESS=1.0e-5 PRESSURE_COMPONENTS=HFA\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        call = timeline.tubing_calls[0]
        assert call["start_md"] == 0.0
        assert call["end_md"] == 2500.0
        assert call["inner_diameter"] == 0.15
        assert call["roughness"] == pytest.approx(1.0e-5)

        well = call["well_path"]
        assert well.completion_settings().custom_segment_calls == [
            {"start_md": 0.0, "end_md": 2500.0}
        ]
        assert well.msw_settings().pressure_drop == "HFA"
        assert well.msw_settings().update_calls == 1

    def test_segment_rejects_invalid_pressure_components(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-01-01 SEGMENT MDSTART=0 MDEND=2500 "
            "PRESSURE_COMPONENTS=INVALID\n"
        )
        timeline, report = self._apply(text)

        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert "PRESSURE_COMPONENTS must be H--, HF-, or HFA" in report.errors[0]
        assert timeline.tubing_calls == []

    def test_tubing_is_reported_as_renamed_event(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-01-01 TUBING MDSTART=0 MDEND=2500\n"
        )
        timeline, report = self._apply(text)

        assert report.events_skipped == 1
        assert timeline.keyword_calls == []
        assert any("did you mean 'SEGMENT'" in warning for warning in report.warnings)

    def test_wellspec_is_reported_as_renamed_event(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2024-01-01 WELLSPEC GROUP=FIELD\n'
        timeline, report = self._apply(text)

        assert report.events_skipped == 1
        assert timeline.wellspec_calls == []
        assert any("did you mean 'WELSPECS'" in warning for warning in report.warnings)

    def test_valve_mapping(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-03-01 VALVE MD=2100 TYPE=ICV STATE=OPEN CV=0.7 AREA=0.0001\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        call = timeline.valve_calls[0]
        assert call["measured_depth"] == 2100.0
        assert call["valve_type"] == "ICV"
        assert call["state"] == "OPEN"
        assert call["flow_coefficient"] == 0.7
        assert call["area"] == pytest.approx(0.0001)

    def test_valve_missing_type_is_error(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2024-03-01 VALVE MD=2100\n'
        _, report = self._apply(text)
        assert report.events_skipped == 1
        assert any("TYPE" in e for e in report.errors)

    def test_state_mapping(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2024-02-15 STATE STATE=SHUT\n'
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        assert timeline.state_calls[0]["well_state"] == "SHUT"

    def test_generic_well_keyword_pass_through(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-06-01 WRFTPLT OUTPUT_RFT=YES OUTPUT_PLT=NO OUTPUT_SEGMENT=NO\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 1
        assert report.warnings == []
        call = timeline.keyword_calls[0]
        assert call["keyword_name"] == "WRFTPLT"
        assert call["keyword_data"]["WELL"] == "55_33-A-1"
        assert call["keyword_data"]["OUTPUT_RFT"] == "YES"

    def test_typo_of_builtin_is_not_passed_through(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-01-01 PERFORATIN MDSTART=1 MDEND=2\n"
        )
        timeline, report = self._apply(text)
        assert report.events_skipped == 1
        assert timeline.keyword_calls == []
        assert any("did you mean 'PERFORATION'" in w for w in report.warnings)

    def test_schedule_events_applied_without_well(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RPTRST BASIC=2 FREQ=1\n"
            "  2024-01-01 TUNING TSINIT=1 TSMAXZ=30\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 2
        rptrst = timeline.schedule_keyword_calls[0]
        assert rptrst["keyword_name"] == "RPTRST"
        assert rptrst["keyword_data"] == {"BASIC": 2, "FREQ": 1}
        assert "WELL" not in rptrst["keyword_data"]

    def test_restart_event_creates_non_emitting_timeline_marker(self):
        text = "ORIONEVENTS 2.0\nSCHEDULE\n  2024-02-01 RESTART\n"
        timeline, report = self._apply(text)

        assert report.events_applied == 1
        assert report.errors == []
        assert timeline.schedule_keyword_calls == [
            {
                "event_date": "2024-02-01",
                "keyword_name": "RESTART",
                "keyword_data": {},
            }
        ]

    def test_raw_text_event_is_applied_without_coalescing(self):
        text = (
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE PRIORITY=2\n"
            "first\nEND_RAW_TEXT\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE PRIORITY=1\n"
            "second\nEND_RAW_TEXT\n"
        )
        timeline, report = self._apply(text)

        assert report.events_applied == 2
        assert [call["text"] for call in timeline.raw_text_calls] == [
            "first\n",
            "second\n",
        ]
        assert timeline.raw_text_calls[0] == {
            "event_date": "2024-01-01",
            "text": "first\n",
            "placement": "AFTER_DATE",
            "anchor_keyword": "",
            "priority": 2,
        }

    def test_group_events_inject_group_name(self):
        text = (
            'ORIONEVENTS 2.0\nGROUP "OP"\n'
            "  2020-07-01 GEFAC EFFICIENCY_FACTOR=1.0 USE_GEFAC_IN_NETWORK=YES\n"
            "  2020-07-01 GCONPROD CONTROL_MODE=LRAT LIQUID_TARGET=20000 WATER_TARGET=20000 OIL_TARGET=20000\n"
            'GROUP "WI"\n'
            "  2020-07-01 GCONINJE PHASE=WATER CONTROL_MODE=RATE SURFACE_TARGET=16000\n"
        )
        timeline, report = self._apply(text)
        assert report.events_applied == 3
        assert [call["keyword_name"] for call in timeline.schedule_keyword_calls] == [
            "GEFAC",
            "GCONPROD",
            "GCONINJE",
        ]
        assert timeline.schedule_keyword_calls[0]["keyword_data"] == {
            "GROUP": "OP",
            "EFFICIENCY_FACTOR": 1.0,
            "USE_GEFAC_IN_NETWORK": "YES",
        }
        assert timeline.schedule_keyword_calls[1]["keyword_data"]["GROUP"] == "OP"
        assert timeline.schedule_keyword_calls[2]["keyword_data"]["GROUP"] == "WI"

    def test_member_event_expands_to_unique_grouptree_events(self):
        text = (
            'ORIONEVENTS 2.0\nGROUP "PRODUCERS"\n'
            '  2024-01-01 MEMBER MEMBERS="WELL_A, WELL_B,WELL_A" '
            'COMMENT="Group membership"\n'
        )
        timeline, report = self._apply(text)

        assert report.events_applied == 2
        assert report.errors == []
        assert [call["keyword_data"] for call in timeline.schedule_keyword_calls] == [
            {"CHILD_GROUP": "WELL_A", "PARENT_GROUP": "PRODUCERS"},
            {"CHILD_GROUP": "WELL_B", "PARENT_GROUP": "PRODUCERS"},
        ]
        assert all(
            event.comment == "Group membership" for event in timeline.created_events
        )

    @pytest.mark.parametrize(
        "block,event,expected_error",
        [
            ("SCHEDULE", 'MEMBER MEMBERS="A"', "needs a GROUP block"),
            ('GROUP "G"', "MEMBER", "missing required attribute"),
            (
                'GROUP "G"',
                'MEMBER MEMBERS="A,,B"',
                "non-empty names",
            ),
            (
                'GROUP "G"',
                'MEMBER MEMBERS="A" EXTRA=1',
                "unknown MEMBER attribute",
            ),
        ],
    )
    def test_invalid_member_event_is_skipped(self, block, event, expected_error):
        text = f"ORIONEVENTS 2.0\n{block}\n  2024-01-01 {event}\n"
        timeline, report = self._apply(text)

        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert timeline.schedule_keyword_calls == []
        assert expected_error in report.errors[0]
        expected_scope = 'GROUP "G"' if block.startswith("GROUP") else "SCHEDULE"
        assert f"[{expected_scope}, date 2024-01-01]" in report.errors[0]

    def test_completion_event_in_schedule_block_is_error(self):
        text = "ORIONEVENTS 2.0\nSCHEDULE\n  2024-01-01 PERFORATION MDSTART=1 MDEND=2\n"
        timeline, report = self._apply(text)
        assert report.events_skipped == 1
        assert timeline.schedule_keyword_calls == []
        assert any("needs a WELL block" in e for e in report.errors)

    def test_datetime_event_date_keeps_milliseconds(self):
        text = (
            'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n'
            "  2024-05-15T14:45:30.500 PERFORATION MDSTART=1 MDEND=2\n"
        )
        timeline, _ = self._apply(text)
        assert timeline.perf_calls[0]["event_date"] == "2024-05-15T14:45:30.500"

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


class TestFilterApplying:
    def _apply(self, text, case=None, cases=None, **opts):
        doc = parse_orion_events(text)
        timeline = FakeTimeline()
        project = FakeProject(
            ["55_33-A-1"], cases=cases if cases is not None else [FakeCase()]
        )
        report = apply_orion_document(doc, timeline, project, case=case, **opts)
        return timeline, project, report

    def _perf_text(self, decls, *filter_values):
        events = "".join(
            f"  2018-01-{index:02d} PERFORATION MDSTART=1 MDEND=2 FILTER={value}\n"
            for index, value in enumerate(filter_values, start=1)
        )
        return "ORIONEVENTS 2.0\n" + decls + 'WELL "55_33-A-1"\n' + events

    def test_declared_filter_creates_combined_filter_and_attaches(self):
        text = self._perf_text(
            'FILTER poroperm = "poro>0.4 AND permx > 100.0"\n', "poroperm"
        )
        timeline, project, report = self._apply(text)
        assert report.events_applied == 1
        assert report.errors == []
        assert report.warnings == []  # FILTER no longer warns on PERFORATION

        combined_filters = project.cases()[0].data_filter_collection().combined_filters
        assert len(combined_filters) == 1
        combined = combined_filters[0]
        assert combined.name == "poroperm"
        assert combined.combine_mode == "AND"

        poro, permx = combined.property_filters
        assert (poro.result_variable, poro.result_type) == ("PORO", "STATIC_NATIVE")
        assert (permx.result_variable, permx.result_type) == ("PERMX", "STATIC_NATIVE")
        # '>' sets only the lower bound; the default upper bound is untouched.
        assert poro.lower_bound == 0.4
        assert poro.upper_bound == 1.0e20
        assert permx.lower_bound == 100.0
        assert poro.update_calls == 1

        assert timeline.perf_events[0].filters == [combined]

    def test_upper_bound_operators_and_or_mode(self):
        text = self._perf_text("", '"PORO < 0.1 OR PERMX <= 10"')
        _, project, report = self._apply(text)
        assert report.errors == []
        combined = project.cases()[0].data_filter_collection().combined_filters[0]
        assert combined.combine_mode == "OR"
        assert combined.name == ""  # anonymous inline filter: auto-derived name
        poro, permx = combined.property_filters
        assert poro.upper_bound == 0.1
        assert poro.lower_bound == -1.0e20
        assert permx.upper_bound == 10.0

    def test_declared_filter_shared_between_perforations(self):
        text = self._perf_text('FILTER poroperm = "poro>0.4"\n', "poroperm", "poroperm")
        timeline, project, _ = self._apply(text)
        combined_filters = project.cases()[0].data_filter_collection().combined_filters
        assert len(combined_filters) == 1
        assert timeline.perf_events[0].filters == timeline.perf_events[1].filters

    def test_identical_inline_filters_shared_distinct_not(self):
        text = self._perf_text("", '"poro>0.4"', '"poro>0.4"', '"poro>0.5"')
        timeline, project, _ = self._apply(text)
        combined_filters = project.cases()[0].data_filter_collection().combined_filters
        assert len(combined_filters) == 2
        assert timeline.perf_events[0].filters == timeline.perf_events[1].filters
        assert timeline.perf_events[2].filters != timeline.perf_events[0].filters

    def test_unqualified_name_prefers_static_over_dynamic(self):
        # PORO exists in both STATIC_NATIVE and DYNAMIC_NATIVE in FakeCase.
        text = self._perf_text("", '"PORO > 0.2"')
        _, project, _ = self._apply(text)
        combined = project.cases()[0].data_filter_collection().combined_filters[0]
        assert combined.property_filters[0].result_type == "STATIC_NATIVE"

    def test_qualified_name_searches_only_that_type(self):
        text = self._perf_text("", '"dynamic.PORO > 0.2"')
        _, project, _ = self._apply(text)
        combined = project.cases()[0].data_filter_collection().combined_filters[0]
        assert combined.property_filters[0].result_type == "DYNAMIC_NATIVE"

    def test_explicit_case_overrides_project_first_case(self):
        explicit_case = FakeCase()
        text = self._perf_text("", '"PORO > 0.2"')
        _, project, _ = self._apply(text, case=explicit_case)
        assert explicit_case.data_filter_collection().combined_filters
        first_case = project.cases()[0]
        assert not first_case.data_filter_collection().combined_filters

    def test_filter_without_case_raises(self):
        from rips.exception import RipsError

        text = self._perf_text("", '"PORO > 0.2"')
        with pytest.raises(RipsError, match="no case is available"):
            self._apply(text, cases=[])

    def test_no_filter_without_case_is_fine(self):
        text = 'ORIONEVENTS 2.0\nWELL "55_33-A-1"\n  2018-01-01 PERFORATION MDSTART=1 MDEND=2\n'
        _, _, report = self._apply(text, cases=[])
        assert report.events_applied == 1

    def test_missing_result_raises_before_applying(self):
        from rips.exception import RipsError

        text = self._perf_text("", '"NOTHERE > 1"')
        doc = parse_orion_events(text)
        timeline = FakeTimeline()
        project = FakeProject(["55_33-A-1"], cases=[FakeCase()])
        with pytest.raises(
            RipsError,
            match="'NOTHERE' not found .searched STATIC_NATIVE, DYNAMIC_NATIVE, GENERATED",
        ):
            apply_orion_document(doc, timeline, project)
        assert timeline.perf_calls == []  # nothing applied

    def test_missing_qualified_result_raises(self):
        from rips.exception import RipsError

        text = self._perf_text("", '"generated.PORO > 1"')
        with pytest.raises(RipsError, match="'PORO' not found among GENERATED results"):
            self._apply(text)

    def test_declared_but_unused_filter_creates_nothing(self):
        text = (
            'ORIONEVENTS 2.0\nFILTER unused = "poro>0.4"\nWELL "55_33-A-1"\n'
            "  2018-01-01 PERFORATION MDSTART=1 MDEND=2\n"
        )
        _, project, report = self._apply(text)
        assert report.events_applied == 1
        assert not project.cases()[0].data_filter_collection().combined_filters


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

    def test_wconhist_invalid_item_names_report_context_and_valid_names(
        self, project_with_case_and_wells
    ):
        project, _case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN INVALID_FIELD=1.0\n"
        )

        report = apply_orion_document(document, timeline, project)

        assert report.events_applied == 0
        assert report.events_skipped == 1
        assert len(report.errors) == 1
        error_msg = report.errors[0]
        assert f'Line 3 [WELL "{well.name}", date 2024-01-01]' in error_msg
        assert (
            "Keyword 'WCONHIST' contains invalid item names: INVALID_FIELD" in error_msg
        )
        assert "Valid item names are:" in error_msg

    def test_unknown_keyword_reports_context_and_continues(
        self, project_with_case_and_wells
    ):
        project, _case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 NOT_A_KEYWORD VALUE=1\n"
            "  2024-01-02 WCONHIST STATUS=OPEN\n"
        )

        report = apply_orion_document(document, timeline, project)

        assert report.events_applied == 1
        assert report.events_skipped == 1
        assert len(report.errors) == 1
        error_msg = report.errors[0]
        assert f'Line 3 [WELL "{well.name}", date 2024-01-01]' in error_msg
        assert "Keyword 'NOT_A_KEYWORD' is not recognized by opm-common" in error_msg

    def test_rptrst_boolean_values_emit_bare_mnemonics(
        self, project_with_case_and_wells
    ):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN\n"
            "SCHEDULE\n"
            "  2024-01-01 RPTRST BASIC=2 DEN=True ROCKC=True RPORV=True "
            "RFIP=True FLOWS=True FLORES=True NORST=False\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.events_applied == 2

        schedule = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[]
        )
        rptrst_block = schedule.split("RPTRST", 1)[1].split("/", 1)[0]
        tokens = rptrst_block.split()

        assert "BASIC=2" in tokens
        for flag in ("DEN", "ROCKC", "RPORV", "RFIP", "FLOWS", "FLORES"):
            assert flag in tokens
            assert f"{flag}=True" not in rptrst_block
        assert "NORST" not in tokens

    def test_raw_text_placement_and_priority(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN\n"
            "SCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE PRIORITY=5\n"
            "-- after-date-late\nEND_RAW_TEXT\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE PRIORITY=-1\n"
            "-- after-date-early\nEND_RAW_TEXT\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=BEFORE_KEYWORD ANCHOR=WCONHIST\n"
            "-- before-wconhist\nEND_RAW_TEXT\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_KEYWORD ANCHOR=WCONHIST\n"
            "-- after-wconhist\nEND_RAW_TEXT\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=END_OF_DATE\n"
            "-- end-of-date\nEND_RAW_TEXT\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        schedule = timeline.generate_schedule_text(
            eclipse_case=case, first_date_as_comment=False
        )

        positions = [
            schedule.index(marker)
            for marker in (
                "DATES",
                "-- after-date-early",
                "-- after-date-late",
                "-- before-wconhist",
                "WCONHIST",
                "-- after-wconhist",
                "-- end-of-date",
            )
        ]
        assert positions == sorted(positions)

    def test_raw_text_only_schedule_is_generated(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        document = parse_orion_events(
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=AFTER_DATE\n"
            "-- raw-only\nEND_RAW_TEXT\n"
        )

        apply_orion_document(document, timeline, project)
        schedule = timeline.generate_schedule_text(
            eclipse_case=case, first_date_as_comment=False
        )
        assert "DATES" in schedule
        assert "-- raw-only\n" in schedule

        schedule_with_date_comment = timeline.generate_schedule_text(eclipse_case=case)
        assert "-- Date: 1 JAN 2024\n-- raw-only\n" in schedule_with_date_comment

    def test_raw_text_missing_anchor_fails_generation(
        self, project_with_case_and_wells
    ):
        project, case, timeline = project_with_case_and_wells
        document = parse_orion_events(
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "  2024-01-01 RAW_TEXT PLACEMENT=BEFORE_KEYWORD ANCHOR=COMPDAT\n"
            "text\nEND_RAW_TEXT\n"
        )

        apply_orion_document(document, timeline, project)
        with pytest.raises(rips.RipsError, match="COMPDAT.*not emitted"):
            timeline.generate_schedule_text(eclipse_case=case)

    def test_event_comment_precedes_generated_keyword(
        self, project_with_case_and_wells
    ):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            '  2024-01-01 WCONHIST STATUS=OPEN COMMENT="Startup target"\n'
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []

        schedule = timeline.generate_schedule_text(eclipse_case=case)
        assert "-- Startup target\nWCONHIST\n" in schedule
        assert "COMMENT" not in schedule

    def test_group_sections_generate_group_keywords(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2020-07-01 WCONHIST STATUS=OPEN CMODE=ORAT\n"
            'GROUP "OP"\n'
            "  2020-07-01 GEFAC EFFICIENCY_FACTOR=1.0 USE_GEFAC_IN_NETWORK=YES\n"
            "  2020-07-01 GCONPROD CONTROL_MODE=LRAT LIQUID_TARGET=20000 "
            "WATER_TARGET=20000 OIL_TARGET=20000\n"
            'GROUP "WI"\n'
            "  2020-07-01 GCONINJE PHASE=WATER CONTROL_MODE=RATE "
            "SURFACE_TARGET=16000\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.events_applied == 4

        schedule = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[]
        )
        assert "GEFAC" in schedule
        assert "GCONPROD" in schedule
        assert "GCONINJE" in schedule
        assert "'OP'" in schedule
        assert "'WI'" in schedule

    def test_member_event_generates_grouptree(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN\n"
            'GROUP "PRODUCERS"\n'
            '  2024-01-01 MEMBER MEMBERS="WELL_A,WELL_B"\n'
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.events_applied == 3

        schedule = timeline.generate_schedule_text(eclipse_case=case)
        grouptree_block = schedule.split("GRUPTREE", 1)[1].split("\n/\n", 1)[0]
        normalized_block = " ".join(grouptree_block.split())
        assert "'WELL_A' 'PRODUCERS'" in normalized_block
        assert "'WELL_B' 'PRODUCERS'" in normalized_block

    def test_wellspec_updates_settings_and_generates_cumulative_welspecs(
        self, project_with_case_and_wells
    ):
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2018-01-01 WELSPECS GROUP=my_group REFDEPTH=1002 PHASE=water\n"
            "  2019-01-01 WELSPECS CROSSFLOW=False REFDEPTH=1000 PHASE=oil\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.events_applied == 2

        schedule = timeline.generate_schedule_text(
            eclipse_case=case, first_date_as_comment=False, align_columns=True
        )
        assert schedule.count("WELSPECS\n") == 2
        blocks = schedule.split("WELSPECS\n")[1:]
        first_record = " ".join(blocks[0].split("\n/\n", 1)[0].split())
        second_record = " ".join(blocks[1].split("\n/\n", 1)[0].split())

        assert "'my_group'" in first_record
        assert "1002" in first_record
        assert "'WATER'" in first_record
        assert "'YES'" in first_record
        assert "1*" not in first_record.split("'my_group'", 1)[1].split("1002", 1)[0]

        assert "'my_group'" in second_record
        assert "1000" in second_record
        assert "'OIL'" in second_record
        assert "'NO'" in second_record

        timeline.set_timestamp(timestamp="2018-06-01")
        settings = well.completion_settings()
        assert settings.group_name_for_export == "my_group"
        assert settings.allow_well_cross_flow is True
        assert settings.reference_depth_for_export == 1002
        assert settings.well_type_for_export == "WATER"

        timeline.set_timestamp(timestamp="2019-06-01")
        settings = well.completion_settings()
        assert settings.group_name_for_export == "my_group"
        assert settings.allow_well_cross_flow is False
        assert settings.reference_depth_for_export == 1000
        assert settings.well_type_for_export == "OIL"

    def test_restart_truncates_generated_schedule(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        well = project.well_paths()[0]
        document = parse_orion_events(
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            "  2024-01-01 WCONHIST STATUS=OPEN CMODE=ORAT ORAT=100\n"
            "  2024-02-01 WCONHIST STATUS=OPEN CMODE=ORAT ORAT=200\n"
            "  2024-03-01 WCONHIST STATUS=OPEN CMODE=ORAT ORAT=300\n"
            "SCHEDULE\n"
            "INSERT_DATE 2024-01-15\n"
            "INSERT_DATE 2024-04-01\n"
            "  2024-02-01 RESTART\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []

        schedule = timeline.generate_schedule_text(
            eclipse_case=case,
            first_date_as_comment=False,
            additional_dates=report.report_dates,
        )
        assert "1 'JAN' 2024" not in schedule
        assert "15 'JAN' 2024" not in schedule
        assert "1 'FEB' 2024" in schedule
        assert "1 'MAR' 2024" in schedule
        assert "1 'APR' 2024" in schedule
        assert " 100" not in schedule
        assert " 200" in schedule
        assert " 300" in schedule
        assert "RESTART\n" not in schedule

    def test_report_only_document_generates_schedule(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        document = parse_orion_events(
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "INSERT_DATE 2024-02-01\nINSERT_DATE 2024-06-01\n"
        )

        report = apply_orion_document(document, timeline, project)
        schedule = timeline.generate_schedule_text(
            eclipse_case=case,
            first_date_as_comment=False,
            additional_dates=report.report_dates,
        )

        assert report.events_applied == 0
        assert report.report_dates == ["2024-02-01", "2024-06-01"]
        assert "1 'FEB' 2024" in schedule
        assert "1 'JUN' 2024" in schedule

    def test_end_is_first_keyword_after_date(self, project_with_case_and_wells):
        project, case, timeline = project_with_case_and_wells
        document = parse_orion_events(
            "ORIONEVENTS 2.0\nSCHEDULE\n"
            "2024-01-01 RPTRST BASIC=2 FREQ=1\n"
            "2024-01-01 END\n"
        )

        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        schedule = timeline.generate_schedule_text(
            eclipse_case=case,
            first_date_as_comment=False,
        )

        date_position = schedule.index("1 'JAN' 2024")
        end_position = schedule.index("\nEND\n", date_position)
        rptrst_position = schedule.index("\nRPTRST\n", date_position)
        assert date_position < end_position < rptrst_position

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
            "  START         PERFORATION  MDSTART=2000  MDEND=2200  DIAMETER=0.1  SKIN=0.5  COMPLETION_NUMBER=1\n"
            "  START + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1\n"
            "  START + RAMP  WELTARG      CMODE=BHP  VALUE=50\n"
            "SCHEDULE\n"
            "INSERT_DATE 2024-07-01\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.events_applied == 3
        assert report.report_dates == ["2024-07-01"]

        # Materialize completions from the perforation event.
        timeline.set_timestamp(timestamp="2024-01-15")
        perforations = well.completions().perforations().perforations()
        assert len(perforations) > 0, "Perforation should be created from event"
        perf = perforations[0]
        # DIAMETER is passed directly to the perforation event.
        assert perf.diameter == pytest.approx(0.1)
        assert abs(perf.start_measured_depth - 2000.0) < 1.0
        assert abs(perf.end_measured_depth - 2200.0) < 1.0

        # The generated schedule should carry the mapped keywords, and the
        # INSERT_DATE date should appear as a bare DATES entry (issue #14514).
        schedule = timeline.generate_schedule_text(
            eclipse_case=case,
            export_msw_for_wells=[],
            additional_dates=report.report_dates,
        )
        assert "COMPDAT" in schedule
        assert "WCONHIST" in schedule
        assert "WELTARG" in schedule
        assert "1 'JUL' 2024" in schedule, (
            "INSERT_DATE date should be emitted as a DATES entry"
        )

    def test_apply_full_event_coverage_and_schedule(self, project_with_case_and_wells):
        """All event kinds from well_event_schedule.py expressed as ORIONEVENTS."""
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        text = (
            "ORIONEVENTS 2.0\n"
            "UNIT METRIC\n"
            "DATE STARTUP = 2024-01-01\n"
            "DURATION RAMP = 31 DAYS\n"
            f'WELL "{well.name}"\n'
            "  STARTUP         SEGMENT      MDSTART=0  MDEND=2500  INNER_DIAMETER=0.15  ROUGHNESS=1.0e-5 PRESSURE_COMPONENTS=HFA\n"
            "  STARTUP + RAMP  PERFORATION  MDSTART=2000  MDEND=2200  DIAMETER=0.1  SKIN=0.5  COMPLETION_NUMBER=1\n"
            "  2024-05-15T14:45:30.500  PERFORATION  MDSTART=2300  MDEND=2350  DIAMETER=0.1  SKIN=0.4  COMPLETION_NUMBER=2\n"
            "  2024-03-01      VALVE        MD=2100  TYPE=ICV  STATE=OPEN  CV=0.7  AREA=0.0001\n"
            "  2024-02-15      STATE        STATE=OPEN\n"
            "  2024-01-15      WCONHIST     STATUS=OPEN  CMODE=RESV  ORAT=3999.99  VFP=1\n"
            "  2024-05-01      WELTARG      CMODE=ORAT  VALUE=5000.0\n"
            "  2024-06-01      WRFTPLT      OUTPUT_RFT=YES  OUTPUT_PLT=NO  OUTPUT_SEGMENT=NO\n"
            "SCHEDULE\n"
            "  STARTUP  RPTRST    BASIC=2  FREQ=1\n"
            "  STARTUP  GRUPTREE  CHILD_GROUP=OP  PARENT_GROUP=FIELD\n"
            "  STARTUP  TUNING    TSINIT=1  TSMAXZ=30  NEWTMX=12\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.errors == []
        assert report.warnings == []
        assert report.events_applied == 11

        custom_segments = well.descendants(rips.CustomSegmentInterval)
        assert any(
            segment.start_md == 0.0 and segment.end_md == 2500.0
            for segment in custom_segments
        )
        assert well.msw_settings().pressure_drop == "HFA"

        timeline.set_timestamp(timestamp="2024-12-24")
        schedule = timeline.generate_schedule_text(
            eclipse_case=case, export_msw_for_wells=[well]
        )
        for keyword in (
            "COMPDAT",
            "COMPLUMP",
            "WCONHIST",
            "WELTARG",
            "WRFTPLT",
            "RPTRST",
            "GRUPTREE",
            "TUNING",
        ):
            assert keyword in schedule, f"{keyword} missing from schedule"
        # The datetime perforation must surface as a DATES entry with TIME.
        assert "14:45:30.500" in schedule

    def test_apply_unknown_well_warns_and_applies_nothing(
        self, project_with_case_and_wells
    ):
        project, _case, timeline = project_with_case_and_wells
        text = (
            'ORIONEVENTS 2.0\nWELL "NO_SUCH_WELL"\n'
            "  2024-01-01 PERFORATION MDSTART=1 MDEND=2\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project)
        assert report.events_applied == 0
        assert any("NO_SUCH_WELL" in w for w in report.warnings)

    def test_apply_filter_creates_combined_filter_on_perforation(
        self, project_with_case_and_wells
    ):
        """FILTER declaration -> case-level combined filter attached to the perforation."""
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        text = (
            "ORIONEVENTS 2.0\n"
            'FILTER hiperm = "PERMX > 50.0"\n'
            f'WELL "{well.name}"\n'
            "  2024-01-01 PERFORATION MDSTART=2000 MDEND=2200 DIAMETER=0.1 FILTER=hiperm\n"
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project, case=case)
        assert report.errors == []
        assert report.warnings == []
        assert report.events_applied == 1

        # The declared filter exists at case level under its declaration name.
        combined = next(
            f for f in case.data_filter_collection().filters() if f.name == "hiperm"
        )
        assert combined.combine_mode == "AND"
        property_filter = combined.filters()[0]
        assert property_filter.lower_bound == 50.0
        # Only the lower bound was set; the upper bound keeps the result max
        # (PERMX in TEST10K tops out at exactly 100.0).
        assert property_filter.upper_bound == 100.0

        # The filter survives materialization onto the perforation interval.
        timeline.set_timestamp(timestamp="2024-01-15")
        perforations = well.completions().perforations().perforations()
        assert len(perforations) == 1
        cell_filter = perforations[0].cell_filter()
        assert cell_filter is not None
        assert cell_filter.name == "hiperm"

    def test_apply_inline_filter_gets_auto_derived_name(
        self, project_with_case_and_wells
    ):
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        text = (
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            '  2024-01-01 PERFORATION MDSTART=2000 MDEND=2200 FILTER="static.PERMX >= 50"\n'
        )
        document = parse_orion_events(text)
        report = apply_orion_document(document, timeline, project, case=case)
        assert report.errors == []
        assert report.events_applied == 1

        filters = case.data_filter_collection().filters()
        assert len(filters) == 1
        # Anonymous inline filter: ResInsight auto-derives a descriptive name.
        assert "PERMX" in filters[0].name

    def test_apply_filter_missing_result_raises_and_applies_nothing(
        self, project_with_case_and_wells
    ):
        from rips.exception import RipsError

        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        text = (
            "ORIONEVENTS 2.0\n"
            f'WELL "{well.name}"\n'
            '  2024-01-01 PERFORATION MDSTART=2000 MDEND=2200 FILTER="NO_SUCH_RESULT > 1"\n'
        )
        document = parse_orion_events(text)
        with pytest.raises(RipsError, match="NO_SUCH_RESULT"):
            apply_orion_document(document, timeline, project, case=case)
        assert case.data_filter_collection().filters() == []

    def test_perf_event_filter_round_trip(self, project_with_case_and_wells):
        """Direct timeline API: add_filter / cell_filter on the perf event itself."""
        project, case, timeline = project_with_case_and_wells
        well = next(wp for wp in project.well_paths() if "A" in wp.name)

        combined = case.data_filter_collection().add_combined_filter(
            name="Round Trip", combine_mode="AND"
        )
        perf_event = timeline.add_perf_event(
            event_date="2024-01-01",
            well_path=well,
            start_md=2000.0,
            end_md=2200.0,
            state="OPEN",
        )
        assert perf_event.cell_filter() is None
        perf_event.add_filter(filter=combined)
        cell_filter = perf_event.cell_filter()
        assert cell_filter is not None
        assert cell_filter.name == "Round Trip"
