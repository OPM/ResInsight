"""
ORIONEVENTS well-event-timeline parser and applier.

This module reads an ORIONEVENTS text file describing well completion and
control events over time, and applies them to a ResInsight project through the
existing :class:`WellEventTimeline` API (``add_perf_event``,
``add_well_keyword_event``, ...).

It is split into two independent layers:

* **Layer A - parser** (:func:`parse_orion_events` / :func:`parse_orion_events_file`):
  a pure-Python tokenizer that turns the text into an intermediate
  representation (:class:`OrionDocument`). It has no dependency on a running
  ResInsight instance and can be unit-tested standalone. It doubles as a
  standalone validator: ``python3 -m rips.orion_events <file.orion>``.
* **Layer B - applier** (:func:`apply_orion_document` / :func:`apply_orion_events_file`):
  takes the intermediate representation plus a live ``rips`` project/timeline and
  calls the ``WellEventTimeline`` API, performing all semantic mapping and
  validation.

File format grammar, version 2.0 (EBNF-ish)::

    document        = header , { statement } ;
    header          = "ORIONEVENTS" , "2.0" ;           (* first meaningful line *)
    statement       = unit_directive | declaration | insert_date_line | well_block_open
                    | group_block_open | schedule_block_open | event_line
                    | raw_text_event ;
    unit_directive  = "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;
    insert_date_line = "INSERT_DATE" , date_expr , [ recurrence ] ;
    recurrence      = "EVERY" , [ positive_integer ] , period ,
                      [ "UNTIL" , date_expr ] ;
    period          = "DAY" | "DAYS" | "MONTH" | "MONTHS" | "YEAR" | "YEARS" ;

    declaration     = date_decl | duration_decl | well_decl | filter_decl ;
    date_decl       = "DATE" , ident , "=" , date_expr ;         (* DATE X = 2018-03-01 + 9 *)
    duration_decl   = "DURATION" , ident , "=" , duration_expr ; (* DURATION RAMP = 5 DAYS *)
    well_decl       = "WELL" , ident , "=" , quoted_string ;     (* WELL A1 = "55_33-A-1" *)
    filter_decl     = "FILTER" , ident , "=" , '"' , filter_expr , '"' ;
                                        (* FILTER POROPERM = "PORO > 0.4 AND PERMX > 100.0" *)
    filter_expr     = filter_term , { ( "AND" | "OR" ) , filter_term } ;
                                        (* one combine mode; mixing AND and OR is an error *)
    filter_term     = [ result_type , "." ] , ident , comp_op , number ;
    comp_op         = ">" | ">=" | "<" | "<=" ;

    well_block_open     = "WELL" , ( quoted_string | ident ) ;   (* no "=" present *)
    group_block_open    = "GROUP" , quoted_string ;     (* group keyword events *)
    schedule_block_open = "SCHEDULE" ;                  (* well-less keyword events *)
    event_line      = date_expr , event_type , { attribute } ;
    raw_text_event  = date_expr , "RAW_TEXT" , raw_text_attributes , newline,
                      { raw_line , newline } , "END_RAW_TEXT" ;
    raw_text_attributes = "PLACEMENT=" ,
                          ( "AFTER_DATE" | "BEFORE_KEYWORD" |
                            "AFTER_KEYWORD" | "END_OF_DATE" ) ,
                          [ "ANCHOR=" , ident ] , [ "PRIORITY=" , integer ] ;

    date_expr       = ( iso_date | iso_datetime | date_ident ) , { sign , term } ;
    duration_expr   = ( integer | dur_ident ) , { sign , term } , [ "DAYS" | "days" ] ;
    term            = integer | dur_ident ;             (* whole days *)
    sign            = "+" | "-" ;
    iso_date        = 4digit , "-" , 2digit , "-" , 2digit ;
    iso_datetime    = iso_date , "T" , 2digit , ":" , 2digit , ":" , 2digit ,
                      [ "." , digits ] ;                (* 2024-05-15T14:45:30.500 *)
    ident           = letter_or_underscore , { word_char } ;
    attribute       = ident , "=" , ( quoted_string | bareword ) ;
    comment         = "#" , rest-of-line ;              (* line or trailing *)

Notes on the grammar:

* The format is line-oriented; every non-blank line is dispatched on its first
  token: ``ORIONEVENTS`` (once), ``UNIT``, ``DATE``, ``DURATION``, ``WELL``,
  ``GROUP``, ``SCHEDULE``, ``INSERT_DATE`` or an event date. Anything else is an
  error. Keywords are
  uppercase and case-sensitive (the ``DAYS`` suffix is also accepted as ``days``).
* Comments start with ``#`` (outside of double quotes) and run to end of line.
* Variables are **typed**: ``DATE``, ``DURATION`` (whole days), ``WELL``
  (well-name alias) and ``FILTER`` (cell filter expression) declarations share
  one namespace and must precede use.
  Using a variable of the wrong type is an error that cites both the use and
  the declaration site. Redeclaring a name with the same type warns and the
  last value wins; redeclaring with a different type is an error.
* Date arithmetic is a chain of signed whole-day terms, each an integer or a
  ``DURATION`` variable: ``START + RAMP - 2``. Whitespace around ``+``/``-``
  is optional but conventional. An event date may carry a time-of-day
  (``2024-05-15T14:45:30.500``), which the schedule generator preserves as
  the optional TIME field of the DATES keyword.
* ``WELL <ident>`` opens an event block for a declared ``WELL`` alias;
  ``WELL "<name>"`` opens a block for the literal well name and never consults
  variables. A ``WELL`` line containing ``=`` is always a declaration. A bare
  ``GROUP "<name>"`` opens a block of group-level Eclipse keyword events; the
  group name is injected as the ``GROUP`` item when each event is applied.
  A bare ``SCHEDULE`` line opens a block of schedule-level keyword events not
  tied to any well (RPTRST, GRUPTREE, TUNING, ...). Empty blocks are legal.
  ``MEMBER MEMBERS="A,B"`` inside a GROUP block is shorthand for one GRUPTREE
  record per unique member, with the enclosing group as parent. A schedule may
  contain one attribute-free ``RESTART`` event; it truncates generated schedule
  output before its timestamp and is not itself emitted as a keyword.
* ``INSERT_DATE <date_expr>`` inside a ``SCHEDULE`` block names a date that
  should appear as a bare ``DATES`` keyword in the generated schedule even when no
  events fall on it — in Eclipse/Flow a ``DATES`` entry ensures a summary
  report at that date. ``EVERY [n] DAYS|MONTHS|YEARS`` makes it recurring and
  an inclusive ``UNTIL <date_expr>`` sets the end date. When ``UNTIL`` is
  omitted, the latest event date is used. Monthly and yearly recurrences
  stay anchored to the initial calendar day, clamping to the end of shorter
  months. The dates are collected on :attr:`OrionDocument.report_dates` and
  surfaced by the applier as sorted ISO strings on
  :attr:`ApplyReport.report_dates`, ready to pass to
  ``WellEventTimeline.generate_schedule_text(additional_dates=...)``. A
  ``INSERT_DATE`` line is not tied to any well.
* ``RAW_TEXT`` is valid only inside a ``SCHEDULE`` block. Its body is copied
  without parsing or formatting through the mandatory standalone
  ``END_RAW_TEXT`` line. ``PLACEMENT`` is ``AFTER_DATE``, ``BEFORE_KEYWORD``,
  ``AFTER_KEYWORD`` or ``END_OF_DATE``. Before/after-keyword placement requires
  ``ANCHOR=<Eclipse keyword>``; the other placements forbid it. ``PRIORITY`` is
  an optional integer (default 0); lower values are emitted first and source
  order breaks ties.
* Double quotes are used everywhere: well names, filter expressions and
  attribute values, e.g. ``FILTER="SOIL > 0.8 AND PERMX > 200"``.
* Every attribute is ``KEY=VALUE``; bare positional tokens are rejected.
* Event types inside a WELL block are either the built-in completion events
  ``PERFORATION``, ``SEGMENT``, ``VALVE``, ``STATE`` and ``WELSPECS``, or any
  Eclipse well keyword (``WCONHIST``, ``WELTARG``, ``WRFTPLT``, ``WCONPROD``,
  ...), which
  is passed through generically with the well name injected as WELL. Event
  ``WELSPECS`` accepts partial updates to ``GROUP``, ``CROSSFLOW``, ``REFDEPTH``
  and ``PHASE`` (OIL/GAS/WATER/LIQUID). Omitted values inherit the previous
  WELSPECS state, initially using the well's completion export settings. There
  may be multiple WELSPECS events for a well, but not at the same timestamp.
  Each emits a WELSPECS record with the cumulative state. Event values are
  materialized back onto completion settings by ``timeline.set_timestamp()``.
  Event types inside a GROUP block are Eclipse group keywords with the group
  name injected as GROUP. Event types inside a SCHEDULE block are Eclipse schedule
  keywords passed through as-is. An event type that closely resembles a built-in
  is treated as a typo per the ``on_unknown_event`` policy instead of being
  passed through.
* On a PERFORATION event, ``FILTER=<name>`` references a declared ``FILTER``
  variable and ``FILTER="<expr>"`` is an inline anonymous filter expression.
  The applier materializes each used filter as a case-level combined data
  filter and attaches it to the perforation event. A filter term is
  ``[TYPE.]NAME <op> NUMBER`` with ``>``, ``>=``, ``<`` or ``<=``; bounds are
  inclusive, so ``>`` behaves as ``>=``. An unqualified result name is
  searched in STATIC_NATIVE, then DYNAMIC_NATIVE, then GENERATED results; a
  ``TYPE.`` qualifier (``STATIC``/``DYNAMIC``/``GENERATED`` or the full
  ``*_NATIVE`` form, case-insensitive) restricts the search to that type.
* Any other attribute key parses; ``FILTER`` on events other than
  PERFORATION is ignored with a warning when applied.
* The parser recovers per line and reports **all** errors in one pass: the
  raised :class:`OrionParseError` carries one :class:`ParseIssue` per problem.
  Unknown names come with "did you mean" suggestions where possible.
* ORIONEVENTS 1.x files (``SET`` variables, single-quoted well names) are not
  supported; the header version is rejected with a pointer to this grammar.
"""

from __future__ import annotations

import calendar
import copy
import datetime
import difflib
import os
import re
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Set, Tuple, Union

from .exception import RipsError

AttrScalar = Union[str, int, float, bool]


# ---------------------------------------------------------------------------
# Exceptions and intermediate representation (Layer A output)
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class SourceLoc:
    """Location of a construct in the source file (1-based line number)."""

    line: int
    text: str


@dataclass(frozen=True)
class ParseIssue:
    """A single fatal problem found while parsing."""

    message: str
    loc: Optional[SourceLoc]


class OrionParseError(Exception):
    """Raised for structural or type errors while parsing.

    ``errors`` holds one :class:`ParseIssue` per problem; a single parse pass
    collects every bad line before raising.
    """

    def __init__(
        self,
        message: Optional[str] = None,
        loc: Optional[SourceLoc] = None,
        errors: Optional[List[ParseIssue]] = None,
    ) -> None:
        if errors is not None:
            self.errors = list(errors)
        else:
            self.errors = [ParseIssue(message=message or "", loc=loc)]
        self.loc = loc
        lines = [
            f"Line {issue.loc.line}: {issue.message}" if issue.loc else issue.message
            for issue in self.errors
        ]
        super().__init__("\n".join(lines))


@dataclass(frozen=True)
class ParseWarning:
    """A non-fatal issue encountered while parsing."""

    message: str
    loc: SourceLoc


@dataclass(frozen=True)
class FilterTerm:
    """One comparison in a filter expression: ``[TYPE.]NAME <op> value``.

    ``result_type`` is ``STATIC_NATIVE``, ``DYNAMIC_NATIVE`` or ``GENERATED``
    when the term is qualified, or None to search all three (in that order).
    """

    result_name: str
    result_type: Optional[str]
    op: str
    value: float


@dataclass(frozen=True)
class FilterExpr:
    """A parsed filter expression: comparison terms with one combine mode."""

    terms: Tuple[FilterTerm, ...]
    combine_mode: str
    raw: str

    def __str__(self) -> str:
        return self.raw


@dataclass(frozen=True)
class EventFilter:
    """A cell filter attached to a PERFORATION event.

    ``name`` is the FILTER declaration name, or None for an inline expression.
    """

    name: Optional[str]
    expr: FilterExpr


@dataclass(frozen=True)
class OrionValue:
    """A typed variable: kind is ``DATE``, ``DURATION``, ``WELL`` or ``FILTER``.

    A ``DATE`` value is a :class:`datetime.date`, or a :class:`datetime.datetime`
    when declared with a time-of-day. A ``FILTER`` value is a
    :class:`FilterExpr`.
    """

    kind: str
    value: Union[datetime.date, datetime.datetime, int, str, FilterExpr]
    loc: SourceLoc


@dataclass(frozen=True)
class AttrValue:
    """A single ``KEY=VALUE`` attribute with its type-inferred value."""

    raw: str
    value: AttrScalar
    quoted: bool


@dataclass
class WellSpecState:
    """Fully resolved cumulative state for one WELSPECS event."""

    group: str
    crossflow: bool
    refdepth: Optional[float]
    phase: str


@dataclass
class OrionEvent:
    """One event line in an enclosing WELL, GROUP or SCHEDULE block."""

    event_type: str
    event_date: Union[datetime.date, datetime.datetime]
    attributes: Dict[str, AttrValue]
    loc: SourceLoc
    filter: Optional[EventFilter] = None
    well_spec: Optional[WellSpecState] = None
    raw_text: Optional[str] = None
    raw_placement: Optional[str] = None
    raw_anchor: Optional[str] = None
    raw_priority: int = 0
    scope: str = "SCHEDULE"
    scope_name: Optional[str] = None


@dataclass
class WellBlock:
    """A ``WELL`` block header followed by its events."""

    well_name: str
    events: List[OrionEvent] = field(default_factory=list)
    loc: SourceLoc = SourceLoc(0, "")


@dataclass
class GroupBlock:
    """A ``GROUP`` block header followed by its keyword events."""

    group_name: str
    events: List[OrionEvent] = field(default_factory=list)
    loc: SourceLoc = SourceLoc(0, "")


@dataclass(frozen=True)
class _ReportSpec:
    """One INSERT_DATE declaration, expanded after all event dates are known."""

    start: Union[datetime.date, datetime.datetime]
    interval: Optional[int]
    period: Optional[str]
    end: Optional[Union[datetime.date, datetime.datetime]]
    loc: SourceLoc


@dataclass
class OrionDocument:
    """Parsed, lossless representation of an ORIONEVENTS file."""

    version: str
    unit_system: str = "METRIC"
    variables: Dict[str, OrionValue] = field(default_factory=dict)
    wells: List[WellBlock] = field(default_factory=list)
    groups: List[GroupBlock] = field(default_factory=list)
    schedule_events: List[OrionEvent] = field(default_factory=list)
    report_dates: List[Union[datetime.date, datetime.datetime]] = field(
        default_factory=list
    )
    warnings: List[ParseWarning] = field(default_factory=list)


def _iso_event_date(event_date: Union[datetime.date, datetime.datetime]) -> str:
    """Format an event date for the timeline API, keeping ms time-of-day."""
    if isinstance(event_date, datetime.datetime):
        if event_date.microsecond:
            return event_date.isoformat(timespec="milliseconds")
        return event_date.isoformat()
    return event_date.isoformat()


def _event_context(event: OrionEvent) -> str:
    """Return the source scope and timestamp identifying an event."""
    scope = event.scope
    if event.scope_name is not None:
        scope += f' "{event.scope_name}"'
    return f"[{scope}, date {_iso_event_date(event.event_date)}]"


def _event_message(event: OrionEvent, message: str) -> str:
    """Add source line, scope and timestamp to an event-level message."""
    return f"Line {event.loc.line} {_event_context(event)}: {message}"


def _set_event_scopes(
    wells: List[WellBlock],
    groups: List[GroupBlock],
    schedule_events: List[OrionEvent],
) -> None:
    """Attach enclosing block information to parsed events."""
    for well in wells:
        for event in well.events:
            event.scope = "WELL"
            event.scope_name = well.well_name
    for group in groups:
        for event in group.events:
            event.scope = "GROUP"
            event.scope_name = group.group_name
    for event in schedule_events:
        event.scope = "SCHEDULE"
        event.scope_name = None


# ---------------------------------------------------------------------------
# Layer A: pure parser
# ---------------------------------------------------------------------------

_KEYWORDS = (
    "ORIONEVENTS",
    "UNIT",
    "DATE",
    "DURATION",
    "WELL",
    "FILTER",
    "GROUP",
    "SCHEDULE",
    "INSERT_DATE",
)

_IDENT = r"[A-Za-z_]\w*"
_ISO_DATE = r"\d{4}-\d{2}-\d{2}(?:T\d{2}:\d{2}:\d{2}(?:\.\d+)?)?"
_DATE_BASE = rf"(?P<base>{_ISO_DATE}|{_IDENT})"
_TERMS = rf"(?P<terms>(?:\s*[-+]\s*(?:\d+|{_IDENT}))*)"

_HEADER_RE = re.compile(r"^ORIONEVENTS\s+(?P<version>\d+\.\d+)$")
_UNIT_RE = re.compile(r"^UNIT\s+(?P<unit>METRIC|FIELD|LAB)$")
_DATE_DECL_RE = re.compile(rf"^DATE\s+(?P<name>{_IDENT})\s*=\s*{_DATE_BASE}{_TERMS}$")
_DURATION_DECL_RE = re.compile(
    rf"^DURATION\s+(?P<name>{_IDENT})\s*=\s*(?P<base>\d+|{_IDENT}){_TERMS}"
    r"(?:\s+(?:DAYS|days))?$"
)
_WELL_DECL_RE = re.compile(rf'^WELL\s+(?P<name>{_IDENT})\s*=\s*"(?P<well>[^"]*)"$')
_INSERT_DATE_RE = re.compile(
    rf"^INSERT_DATE\s+{_DATE_BASE}{_TERMS}"
    rf"(?:\s+EVERY\s+(?:(?P<count>\d+)\s+)?"
    rf"(?P<period>DAY|DAYS|MONTH|MONTHS|YEAR|YEARS)"
    rf"(?:\s+UNTIL\s+(?P<end_base>{_ISO_DATE}|{_IDENT})"
    rf"(?P<end_terms>(?:\s*[-+]\s*(?:\d+|{_IDENT}))*)"
    rf")?)?$"
)
_FILTER_DECL_RE = re.compile(rf'^FILTER\s+(?P<name>{_IDENT})\s*=\s*"(?P<expr>[^"]*)"$')
_FILTER_SPLIT_RE = re.compile(r"\s+(AND|OR)\s+")
_NUMBER = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"
_FILTER_TERM_RE = re.compile(
    rf"^(?:(?P<qual>{_IDENT})\.)?(?P<name>{_IDENT})\s*"
    rf"(?P<op>>=|<=|>|<)\s*(?P<value>{_NUMBER})$"
)
# Result-type qualifiers accepted in filter terms (case-insensitive).
_RESULT_TYPE_ALIASES = {
    "STATIC": "STATIC_NATIVE",
    "STATIC_NATIVE": "STATIC_NATIVE",
    "DYNAMIC": "DYNAMIC_NATIVE",
    "DYNAMIC_NATIVE": "DYNAMIC_NATIVE",
    "GENERATED": "GENERATED",
}
_WELL_BLOCK_RE = re.compile(rf'^WELL\s+(?:"(?P<qname>[^"]*)"|(?P<ref>{_IDENT}))$')
_GROUP_BLOCK_RE = re.compile(r'^GROUP\s+"(?P<name>[^"]*)"$')
_EVENT_RE = re.compile(rf"^{_DATE_BASE}{_TERMS}\s+(?P<rest>.+)$")
_TERM_RE = re.compile(rf"([-+])\s*(\d+|{_IDENT})")
_ATTR_RE = re.compile(r'(?P<key>[A-Za-z_]\w*)\s*=\s*(?:"(?P<qval>[^"]*)"|(?P<val>\S+))')


def parse_orion_events_file(path: Union[str, "os.PathLike[str]"]) -> OrionDocument:
    """Parse an ORIONEVENTS file from disk into an :class:`OrionDocument`."""
    with open(path, "r", encoding="utf-8") as handle:
        return parse_orion_events(handle.read())


def parse_orion_events(text: str) -> OrionDocument:
    """Parse ORIONEVENTS 2.0 text into an :class:`OrionDocument`.

    Raises:
        OrionParseError: carrying every error found in the file. A missing or
            unsupported ``ORIONEVENTS`` header aborts immediately.
    """
    version: Optional[str] = None
    unit_holder = ["METRIC"]  # mutable so _parse_line can update it
    variables: Dict[str, OrionValue] = {}
    wells: List[WellBlock] = []
    groups: List[GroupBlock] = []
    schedule_events: List[OrionEvent] = []
    report_specs: List[_ReportSpec] = []
    warnings: List[ParseWarning] = []
    errors: List[ParseIssue] = []
    # Event lines append to the current sink: a WellBlock's event list or the
    # document-level schedule_events list.
    current_events: Optional[List[OrionEvent]] = None

    source_lines = text.splitlines()
    line_index = 0
    while line_index < len(source_lines):
        raw_line = source_lines[line_index]
        loc = SourceLoc(line=line_index + 1, text=raw_line)
        line = _strip_comment(raw_line).strip()
        line_index += 1
        if not line:
            continue

        # The first meaningful line must be the header; header problems are
        # fatal since nothing else can be interpreted without a version.
        if version is None:
            match = _HEADER_RE.match(line)
            if match is None:
                raise OrionParseError(
                    "File must start with 'ORIONEVENTS <version>'", loc
                )
            version = match.group("version")
            _check_version(version, loc)
            continue

        if _is_raw_text_header(line):
            end_index = line_index
            while (
                end_index < len(source_lines)
                and source_lines[end_index].strip() != "END_RAW_TEXT"
            ):
                end_index += 1
            if end_index == len(source_lines):
                errors.append(ParseIssue("Unterminated RAW_TEXT block", loc))
                break

            body_lines = source_lines[line_index:end_index]
            line_index = end_index + 1
            try:
                if current_events is not schedule_events:
                    raise OrionParseError(
                        "RAW_TEXT is only valid in a SCHEDULE block", loc
                    )
                current_events.append(
                    _parse_raw_text_event(line, body_lines, variables, loc)
                )
            except OrionParseError as exc:
                errors.extend(exc.errors)
            continue

        try:
            current_events = _parse_line(
                line,
                loc,
                variables,
                wells,
                groups,
                schedule_events,
                report_specs,
                warnings,
                current_events,
                unit_holder,
            )
        except OrionParseError as exc:
            errors.extend(exc.errors)
            if line.split(None, 1)[0] in ("WELL", "GROUP", "SCHEDULE") or (
                _EVENT_RE.match(line) is not None and current_events is None
            ):
                # Suppress cascading errors from lines belonging to a broken
                # (or missing) block: swallow them into a discarded list.
                current_events = []

    if version is None:
        raise OrionParseError("Empty file: missing 'ORIONEVENTS' header")

    _set_event_scopes(wells, groups, schedule_events)
    errors.extend(_restart_validation_issues(wells, groups, schedule_events))
    errors.extend(_wellspec_validation_issues(wells))
    report_dates, report_errors = _expand_report_specs(
        report_specs, wells, groups, schedule_events
    )
    errors.extend(report_errors)
    if errors:
        raise OrionParseError(errors=errors)

    return OrionDocument(
        version=version,
        unit_system=unit_holder[0],
        variables=variables,
        wells=wells,
        groups=groups,
        schedule_events=schedule_events,
        report_dates=report_dates,
        warnings=warnings,
    )


def _as_datetime(value: Union[datetime.date, datetime.datetime]) -> datetime.datetime:
    if isinstance(value, datetime.datetime):
        return value
    return datetime.datetime.combine(value, datetime.time.min)


def _report_occurrence(
    start: Union[datetime.date, datetime.datetime],
    interval: int,
    period: str,
    occurrence: int,
) -> Union[datetime.date, datetime.datetime]:
    offset = interval * occurrence
    if period == "DAY":
        return start + datetime.timedelta(days=offset)

    if period == "MONTH":
        month_index = start.year * 12 + start.month - 1 + offset
        year, zero_based_month = divmod(month_index, 12)
        month = zero_based_month + 1
    else:
        year = start.year + offset
        month = start.month

    day = min(start.day, calendar.monthrange(year, month)[1])
    return start.replace(year=year, month=month, day=day)


def _expand_report_specs(
    report_specs: List[_ReportSpec],
    wells: List[WellBlock],
    groups: List[GroupBlock],
    schedule_events: List[OrionEvent],
) -> Tuple[
    List[Union[datetime.date, datetime.datetime]],
    List[ParseIssue],
]:
    event_dates = [event.event_date for well in wells for event in well.events]
    event_dates.extend(event.event_date for group in groups for event in group.events)
    event_dates.extend(event.event_date for event in schedule_events)
    last_event_date = max(event_dates, key=_as_datetime) if event_dates else None

    dates: List[Union[datetime.date, datetime.datetime]] = []
    issues: List[ParseIssue] = []
    for spec in report_specs:
        if spec.period is None:
            dates.append(spec.start)
            continue

        if spec.interval is None or spec.interval <= 0:
            issues.append(
                ParseIssue("INSERT_DATE interval must be greater than zero", spec.loc)
            )
            continue

        end = spec.end if spec.end is not None else last_event_date
        if end is None:
            issues.append(
                ParseIssue(
                    "Recurring INSERT_DATE without UNTIL requires at least one event",
                    spec.loc,
                )
            )
            continue
        if _as_datetime(end) < _as_datetime(spec.start):
            issues.append(
                ParseIssue(
                    "INSERT_DATE end date must not precede its start date", spec.loc
                )
            )
            continue

        occurrence = 0
        while True:
            try:
                value = _report_occurrence(
                    spec.start, spec.interval, spec.period, occurrence
                )
            except (OverflowError, ValueError):
                break
            if _as_datetime(value) > _as_datetime(end):
                break
            dates.append(value)
            occurrence += 1

    return dates, issues


def _restart_validation_issues(
    wells: List[WellBlock],
    groups: List[GroupBlock],
    schedule_events: List[OrionEvent],
) -> List[ParseIssue]:
    """Validate placement, cardinality and shape of RESTART events."""
    issues: List[ParseIssue] = []
    for well_block in wells:
        for event in well_block.events:
            if event.event_type.upper() == "RESTART":
                issues.append(
                    ParseIssue(
                        f"{_event_context(event)}: RESTART is only valid in a "
                        "SCHEDULE block",
                        event.loc,
                    )
                )
    for group_block in groups:
        for event in group_block.events:
            if event.event_type.upper() == "RESTART":
                issues.append(
                    ParseIssue(
                        f"{_event_context(event)}: RESTART is only valid in a "
                        "SCHEDULE block",
                        event.loc,
                    )
                )

    restart_events = [
        event for event in schedule_events if event.event_type.upper() == "RESTART"
    ]
    for event in restart_events:
        if event.attributes:
            issues.append(
                ParseIssue(
                    f"{_event_context(event)}: RESTART takes no attributes", event.loc
                )
            )
    for event in restart_events[1:]:
        issues.append(
            ParseIssue(
                f"{_event_context(event)}: Only one RESTART event is allowed per "
                "schedule",
                event.loc,
            )
        )
    return issues


def _wellspec_validation_issues(wells: List[WellBlock]) -> List[ParseIssue]:
    """Reject multiple WELSPECS events for one well at the same timestamp."""
    seen: Dict[Tuple[str, Union[datetime.date, datetime.datetime]], OrionEvent] = {}
    issues: List[ParseIssue] = []
    for well in wells:
        for event in well.events:
            if event.event_type.upper() != "WELSPECS":
                continue
            key = (well.well_name, event.event_date)
            previous = seen.get(key)
            if previous is not None:
                issues.append(
                    ParseIssue(
                        f"{_event_context(event)}: WELSPECS already defined "
                        f"(first definition on line {previous.loc.line})",
                        event.loc,
                    )
                )
            else:
                seen[key] = event
    return issues


def _check_version(version: str, loc: SourceLoc) -> None:
    major = version.split(".")[0]
    if major == "2":
        return
    if major == "1":
        raise OrionParseError(
            "ORIONEVENTS 1.x files are no longer supported; this parser requires "
            "version 2.0 (typed DATE/DURATION/WELL declarations, WELL event "
            "blocks, double-quoted names). See the rips.orion_events docstring "
            "for the 2.0 grammar",
            loc,
        )
    raise OrionParseError(
        f"Unsupported ORIONEVENTS version '{version}'; expected 2.0", loc
    )


def _parse_line(
    line: str,
    loc: SourceLoc,
    variables: Dict[str, OrionValue],
    wells: List[WellBlock],
    groups: List[GroupBlock],
    schedule_events: List[OrionEvent],
    report_specs: List[_ReportSpec],
    warnings: List[ParseWarning],
    current_events: Optional[List[OrionEvent]],
    unit_holder: List[str],
) -> Optional[List[OrionEvent]]:
    """Dispatch one non-header line; returns the current event sink."""
    first = line.split(None, 1)[0]

    if first == "UNIT":
        match = _UNIT_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed UNIT line: {line!r} (expected UNIT METRIC|FIELD|LAB)", loc
            )
        unit_holder[0] = match.group("unit")
        return current_events

    if first == "GROUP":
        match = _GROUP_BLOCK_RE.match(line)
        if match is None:
            raise OrionParseError(
                f'Malformed GROUP line: {line!r} (expected GROUP "<group-name>")',
                loc,
            )
        new_group = GroupBlock(group_name=match.group("name"), loc=loc)
        groups.append(new_group)
        return new_group.events

    if first == "SCHEDULE":
        if line != "SCHEDULE":
            raise OrionParseError(
                f"Malformed SCHEDULE line: {line!r} (SCHEDULE takes no arguments)", loc
            )
        return schedule_events

    if first == "INSERT_DATE":
        if current_events is not schedule_events:
            raise OrionParseError("INSERT_DATE is only valid in a SCHEDULE block", loc)
        match = _INSERT_DATE_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed INSERT_DATE line: {line!r} "
                "(expected INSERT_DATE <date-expr> [EVERY [count] "
                "DAYS|MONTHS|YEARS [UNTIL <date-expr>]])",
                loc,
            )
        start = _eval_date_expr(
            match.group("base"), match.group("terms"), variables, loc
        )
        period = match.group("period")
        end_base = match.group("end_base")
        end = (
            _eval_date_expr(end_base, match.group("end_terms"), variables, loc)
            if end_base is not None
            else None
        )
        report_specs.append(
            _ReportSpec(
                start=start,
                interval=int(match.group("count") or 1) if period else None,
                period=period.rstrip("S") if period else None,
                end=end,
                loc=loc,
            )
        )
        return current_events

    if first == "DATE":
        match = _DATE_DECL_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed DATE declaration: {line!r} "
                "(expected DATE NAME = <iso-date|DATE-var> [+|- <days|DURATION-var> ...])",
                loc,
            )
        value = _eval_date_expr(
            match.group("base"), match.group("terms"), variables, loc
        )
        _declare(
            match.group("name"),
            OrionValue("DATE", value, loc),
            variables,
            warnings,
            loc,
        )
        return current_events

    if first == "DURATION":
        match = _DURATION_DECL_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed DURATION declaration: {line!r} "
                "(expected DURATION NAME = <days|DURATION-var> [+|- ...] [DAYS])",
                loc,
            )
        days = _eval_duration_expr(
            match.group("base"), match.group("terms"), variables, loc
        )
        _declare(
            match.group("name"),
            OrionValue("DURATION", days, loc),
            variables,
            warnings,
            loc,
        )
        return current_events

    if first == "FILTER":
        match = _FILTER_DECL_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed FILTER declaration: {line!r} "
                '(expected FILTER NAME = "<result> <op> <number> [AND|OR ...]")',
                loc,
            )
        expr = _parse_filter_expr(match.group("expr"), loc)
        _declare(
            match.group("name"),
            OrionValue("FILTER", expr, loc),
            variables,
            warnings,
            loc,
        )
        return current_events

    if first == "WELL":
        decl_match = _WELL_DECL_RE.match(line)
        if decl_match is not None:
            _declare(
                decl_match.group("name"),
                OrionValue("WELL", decl_match.group("well"), loc),
                variables,
                warnings,
                loc,
            )
            return current_events
        block_match = _WELL_BLOCK_RE.match(line)
        if block_match is None:
            raise OrionParseError(
                f"Malformed WELL line: {line!r} (well names containing special "
                'characters must be double-quoted, e.g. WELL "55_33-A-1")',
                loc,
            )
        name = block_match.group("qname")
        if name is None:
            name = str(
                _lookup_var(block_match.group("ref"), "WELL", variables, loc).value
            )
        new_well = WellBlock(well_name=name, loc=loc)
        wells.append(new_well)
        return new_well.events

    if first == "REPORT":
        raise OrionParseError(
            "REPORT has been renamed to INSERT_DATE and is only valid in a "
            "SCHEDULE block",
            loc,
        )

    if current_events is not None and _EVENT_RE.match(line):
        current_events.append(_parse_event_line(line, variables, loc))
        return current_events
    if first[0].isdigit() and _EVENT_RE.match(line):
        raise OrionParseError("Event line found before any WELL or SCHEDULE block", loc)

    raise OrionParseError(_unrecognized_line_message(line, first), loc)


def _unrecognized_line_message(line: str, first: str) -> str:
    if first == "ORIONEVENTS":
        return "Duplicate ORIONEVENTS header"
    if first == "SET":
        return (
            "SET is ORIONEVENTS 1.x syntax; declare a typed variable instead, "
            "e.g. DATE NAME = 2018-01-01"
        )
    if line.startswith("'"):
        return (
            "Single-quoted well names are ORIONEVENTS 1.x syntax; open a well "
            'block with WELL "name"'
        )
    message = f"Unrecognized line: {line!r}"
    close = difflib.get_close_matches(first, _KEYWORDS, n=1, cutoff=0.6)
    if close:
        message += f"; did you mean '{close[0]}'?"
    return message


def _declare(
    name: str,
    value: OrionValue,
    variables: Dict[str, OrionValue],
    warnings: List[ParseWarning],
    loc: SourceLoc,
) -> None:
    existing = variables.get(name)
    if existing is not None:
        if existing.kind != value.kind:
            raise OrionParseError(
                f"'{name}' is already declared as {existing.kind} "
                f"(line {existing.loc.line}); cannot redeclare as {value.kind}",
                loc,
            )
        warnings.append(ParseWarning(f"Duplicate {value.kind} '{name}'", loc))
    variables[name] = value


def _lookup_var(
    name: str,
    expected_kind: str,
    variables: Dict[str, OrionValue],
    loc: SourceLoc,
) -> OrionValue:
    """Resolve a typed variable reference; the single typed-error site."""
    value = variables.get(name)
    if value is None:
        same_kind = [n for n, v in variables.items() if v.kind == expected_kind]
        close = difflib.get_close_matches(
            name, same_kind or list(variables), n=1, cutoff=0.6
        )
        hint = f"; did you mean '{close[0]}'?" if close else ""
        raise OrionParseError(f"Unknown variable '{name}'{hint}", loc)
    if value.kind != expected_kind:
        raise OrionParseError(
            f"Variable '{name}' is a {value.kind} (declared line {value.loc.line}) "
            f"but a {expected_kind} is required here",
            loc,
        )
    return value


def _eval_terms(terms: str, variables: Dict[str, OrionValue], loc: SourceLoc) -> int:
    """Evaluate a signed chain of whole-day terms to a net day count."""
    total = 0
    for match in _TERM_RE.finditer(terms):
        sign, term = match.groups()
        if term.isdigit():
            days = int(term)
        else:
            value = _lookup_var(term, "DURATION", variables, loc).value
            assert isinstance(value, int)
            days = value
        total += days if sign == "+" else -days
    return total


def _eval_date_expr(
    base: str, terms: str, variables: Dict[str, OrionValue], loc: SourceLoc
) -> Union[datetime.date, datetime.datetime]:
    """Evaluate an ISO date(time) or DATE variable plus optional signed day terms."""
    result: Union[datetime.date, datetime.datetime]
    if base[0].isdigit():
        try:
            if "T" in base:
                result = datetime.datetime.fromisoformat(base)
            else:
                result = datetime.date.fromisoformat(base)
        except ValueError as exc:
            raise OrionParseError(f"Invalid date '{base}': {exc}", loc)
    else:
        value = _lookup_var(base, "DATE", variables, loc).value
        assert isinstance(value, datetime.date)
        result = value
    return result + datetime.timedelta(days=_eval_terms(terms, variables, loc))


def _eval_duration_expr(
    base: str, terms: str, variables: Dict[str, OrionValue], loc: SourceLoc
) -> int:
    """Evaluate an integer or DURATION variable plus optional signed day terms."""
    if base.isdigit():
        result = int(base)
    else:
        value = _lookup_var(base, "DURATION", variables, loc).value
        assert isinstance(value, int)
        result = value
    return result + _eval_terms(terms, variables, loc)


def _parse_filter_expr(text: str, loc: SourceLoc) -> FilterExpr:
    """Parse a filter expression into typed comparison terms."""
    stripped = text.strip()
    if not stripped:
        raise OrionParseError("Empty filter expression", loc)

    parts = _FILTER_SPLIT_RE.split(stripped)
    chunks = parts[0::2]
    connectors = parts[1::2]
    if len(set(connectors)) > 1:
        raise OrionParseError(
            "Filter expression mixes AND and OR; a combined filter has a "
            "single combine mode",
            loc,
        )
    combine_mode = connectors[0] if connectors else "AND"

    terms = tuple(_parse_filter_term(chunk, loc) for chunk in chunks)
    return FilterExpr(terms=terms, combine_mode=combine_mode, raw=stripped)


def _parse_filter_term(chunk: str, loc: SourceLoc) -> FilterTerm:
    chunk = chunk.strip()
    match = _FILTER_TERM_RE.match(chunk)
    if match is None:
        raise OrionParseError(_malformed_filter_term_message(chunk), loc)

    qual = match.group("qual")
    result_type: Optional[str] = None
    if qual is not None:
        result_type = _RESULT_TYPE_ALIASES.get(qual.upper())
        if result_type is None:
            close = difflib.get_close_matches(
                qual.upper(), sorted(set(_RESULT_TYPE_ALIASES)), n=1, cutoff=0.6
            )
            hint = f"; did you mean '{close[0]}'?" if close else ""
            raise OrionParseError(
                f"Unknown result type '{qual}' in filter term {chunk!r}{hint}", loc
            )
    return FilterTerm(
        result_name=match.group("name").upper(),
        result_type=result_type,
        op=match.group("op"),
        value=float(match.group("value")),
    )


def _malformed_filter_term_message(chunk: str) -> str:
    if re.search(r"\b(and|or)\b", chunk):
        return (
            f"Malformed filter term {chunk!r}: combine keywords must be "
            "uppercase AND / OR"
        )
    if re.search(r"(?<![<>])=", chunk):
        return (
            f"Malformed filter term {chunk!r}: only >, >=, < and <= are "
            "supported in filter expressions (bounds are inclusive, so > "
            "behaves as >=)"
        )
    return (
        f"Malformed filter term {chunk!r} "
        "(expected NAME <op> NUMBER with >, >=, < or <=, optionally TYPE.NAME)"
    )


def _strip_comment(line: str) -> str:
    """Remove a ``#`` comment, honoring double-quoted spans."""
    in_quote = False
    result: List[str] = []
    for char in line:
        if char == '"':
            in_quote = not in_quote
        if char == "#" and not in_quote:
            break
        result.append(char)
    return "".join(result)


def _is_raw_text_header(line: str) -> bool:
    """Return whether an event line starts a multiline RAW_TEXT block."""
    match = _EVENT_RE.match(line)
    if match is None:
        return False
    return match.group("rest").split(None, 1)[0].upper() == "RAW_TEXT"


def _parse_raw_text_event(
    line: str,
    body_lines: List[str],
    variables: Dict[str, OrionValue],
    loc: SourceLoc,
) -> OrionEvent:
    """Parse and validate a RAW_TEXT header and attach its unparsed body."""
    event = _parse_event_line(line, variables, loc)
    allowed = {"PLACEMENT", "ANCHOR", "PRIORITY"}
    unknown = set(event.attributes) - allowed
    if unknown:
        raise OrionParseError(
            f"Unknown RAW_TEXT attribute(s): {', '.join(sorted(unknown))}", loc
        )
    if "PLACEMENT" not in event.attributes:
        raise OrionParseError("RAW_TEXT requires PLACEMENT", loc)
    if not body_lines:
        raise OrionParseError("RAW_TEXT body must not be empty", loc)

    placement_value = event.attributes["PLACEMENT"].value
    placement = placement_value.upper() if isinstance(placement_value, str) else ""
    valid_placements = {
        "AFTER_DATE",
        "BEFORE_KEYWORD",
        "AFTER_KEYWORD",
        "END_OF_DATE",
    }
    if placement not in valid_placements:
        raise OrionParseError(
            "RAW_TEXT PLACEMENT must be AFTER_DATE, BEFORE_KEYWORD, "
            "AFTER_KEYWORD, or END_OF_DATE",
            loc,
        )

    anchor: Optional[str] = None
    if "ANCHOR" in event.attributes:
        anchor_value = event.attributes["ANCHOR"].value
        if not isinstance(anchor_value, str) or not anchor_value.strip():
            raise OrionParseError("RAW_TEXT ANCHOR must be a keyword name", loc)
        anchor = anchor_value.strip().upper()

    anchored = placement in {"BEFORE_KEYWORD", "AFTER_KEYWORD"}
    if anchored and anchor is None:
        raise OrionParseError(
            "RAW_TEXT ANCHOR is required for BEFORE_KEYWORD and AFTER_KEYWORD",
            loc,
        )
    if not anchored and anchor is not None:
        raise OrionParseError(
            "RAW_TEXT ANCHOR is only valid for BEFORE_KEYWORD and AFTER_KEYWORD",
            loc,
        )

    priority = 0
    if "PRIORITY" in event.attributes:
        priority_value = event.attributes["PRIORITY"].value
        if isinstance(priority_value, bool) or not isinstance(priority_value, int):
            raise OrionParseError("RAW_TEXT PRIORITY must be an integer", loc)
        priority = priority_value

    event.raw_text = "\n".join(body_lines) + "\n"
    event.raw_placement = placement
    event.raw_anchor = anchor
    event.raw_priority = priority
    return event


def _parse_event_line(
    line: str, variables: Dict[str, OrionValue], loc: SourceLoc
) -> OrionEvent:
    match = _EVENT_RE.match(line)
    if match is None:
        raise OrionParseError(f"Malformed event line: {line!r}", loc)

    event_date = _eval_date_expr(
        match.group("base"), match.group("terms"), variables, loc
    )
    rest = match.group("rest").strip()
    parts = rest.split(None, 1)
    event_type = parts[0]
    attr_str = parts[1] if len(parts) > 1 else ""
    attributes = _parse_attributes(attr_str, loc)

    event_filter: Optional[EventFilter] = None
    if event_type.upper() == "PERFORATION" and "FILTER" in attributes:
        event_filter = _resolve_event_filter(attributes["FILTER"], variables, loc)

    return OrionEvent(
        event_type=event_type,
        event_date=event_date,
        attributes=attributes,
        loc=loc,
        filter=event_filter,
    )


def _resolve_event_filter(
    attr: AttrValue, variables: Dict[str, OrionValue], loc: SourceLoc
) -> EventFilter:
    """Resolve a PERFORATION FILTER attribute to a declared or inline filter."""
    if attr.quoted:
        return EventFilter(name=None, expr=_parse_filter_expr(attr.raw, loc))
    if re.fullmatch(_IDENT, attr.raw) is None:
        raise OrionParseError(
            "FILTER must name a declared FILTER variable or be a quoted "
            f"expression, got {attr.raw!r}",
            loc,
        )
    value = _lookup_var(attr.raw, "FILTER", variables, loc)
    assert isinstance(value.value, FilterExpr)
    return EventFilter(name=attr.raw, expr=value.value)


def _parse_attributes(attr_str: str, loc: SourceLoc) -> Dict[str, AttrValue]:
    attributes: Dict[str, AttrValue] = {}
    pos = 0
    for match in _ATTR_RE.finditer(attr_str):
        gap = attr_str[pos : match.start()]
        if gap.strip():
            raise OrionParseError(f"Malformed attribute near {gap.strip()!r}", loc)
        key = match.group("key").upper()

        qval = match.group("qval")
        if qval is not None:
            attributes[key] = AttrValue(raw=qval, value=qval, quoted=True)
        else:
            raw = match.group("val")
            attributes[key] = AttrValue(raw=raw, value=_infer_value(raw), quoted=False)
        pos = match.end()

    trailing = attr_str[pos:]
    if trailing.strip():
        raise OrionParseError(f"Malformed attribute near {trailing.strip()!r}", loc)
    return attributes


def _infer_value(raw: str) -> AttrScalar:
    """Infer bool, int, then float; otherwise keep the raw string."""
    upper = raw.upper()
    if upper == "TRUE":
        return True
    if upper == "FALSE":
        return False

    try:
        return int(raw)
    except ValueError:
        pass
    try:
        return float(raw)
    except ValueError:
        pass
    return raw


# ---------------------------------------------------------------------------
# Layer B: applier
# ---------------------------------------------------------------------------


@dataclass
class ApplyReport:
    """Summary of applying an :class:`OrionDocument` to a timeline."""

    events_applied: int = 0
    events_skipped: int = 0
    report_dates: List[str] = field(default_factory=list)
    warnings: List[str] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)


def _record_event_exception(
    event: OrionEvent, error: Exception, report: ApplyReport
) -> None:
    """Record an event failure with its Orion source context."""
    report.errors.append(_event_message(event, str(error)))
    report.events_skipped += 1


def _record_event_parse_error(
    event: OrionEvent, error: OrionParseError, report: ApplyReport
) -> None:
    """Record parser-style event failures without duplicating their line number."""
    for issue in error.errors:
        report.errors.append(_event_message(event, issue.message))
    report.events_skipped += 1


# Policies for handling unresolved references: skip silently, warn, or raise.
_POLICIES = ("warn", "error", "skip")

# Attributes accepted on a keyword event but intentionally not emitted.
_IGNORED_KEYWORD_ATTRS = {"FILTER"}

# Completion event attribute handling: (required, known-optional) per type.
# FILTER is applied on PERFORATION events; it is accepted on the other
# completion events but ignored with a warning.
_PERF_REQUIRED = ("MDSTART", "MDEND")
_PERF_KNOWN = {
    "MDSTART",
    "MDEND",
    "DIAMETER",
    "SKIN",
    "COMPLETION_NUMBER",
    "FILTER",
    "COMMENT",
}
_SEGMENT_REQUIRED = ("MDSTART", "MDEND")
_SEGMENT_KNOWN = {
    "MDSTART",
    "MDEND",
    "INNER_DIAMETER",
    "ROUGHNESS",
    "PRESSURE_COMPONENTS",
    "COMMENT",
}
_PRESSURE_COMPONENTS = {"H--", "HF-", "HFA"}
_VALVE_REQUIRED = ("MD", "TYPE")
_VALVE_KNOWN = {"MD", "TYPE", "STATE", "CV", "AREA", "COMMENT"} | {
    "AICD_STRENGTH",
    "AICD_DENSITY_CALIB_FLUID",
    "AICD_VISCOSITY_CALIB_FLUID",
    "AICD_VOL_FLOW_EXP",
    "AICD_VISC_FUNC_EXP",
}
_STATE_REQUIRED = ("STATE",)
_STATE_KNOWN = {"STATE", "COMMENT"}
_WELLSPEC_KNOWN = {"GROUP", "CROSSFLOW", "REFDEPTH", "PHASE", "COMMENT"}
_WELLSPEC_PHASES = {"OIL", "GAS", "WATER", "LIQUID"}
_COMPLETION_IGNORED = {"FILTER"}
_PERF_IGNORED = _COMPLETION_IGNORED  # backwards-compatible alias

# ORIONEVENTS -> Eclipse item-name translations per keyword.
_WCONHIST_FIELD_MAP = {"VFP": "VFP_TABLE"}
_WELTARG_FIELD_MAP = {"VALUE": "NEW_VALUE"}


def _apply_event_comment(event: OrionEvent, timeline_event: Any) -> None:
    """Copy an optional COMMENT attribute to the created timeline event."""
    comment = event.attributes.get("COMMENT")
    if comment is None:
        return
    timeline_event.comment = str(comment.value)
    timeline_event.update()


def apply_orion_events_file(
    path: Union[str, "os.PathLike[str]"],
    timeline: Any,
    project: Any,
    *,
    case: Any = None,
    **options: str,
) -> ApplyReport:
    """Parse an ORIONEVENTS file and apply it to ``timeline``."""
    document = parse_orion_events_file(path)
    return apply_orion_document(document, timeline, project, case=case, **options)


_NON_COALESCING_EVENT_TYPES = {
    "MEMBER",
    "PERFORATION",
    "RAW_TEXT",
    "RESTART",
    "SEGMENT",
    "STATE",
    "VALVE",
    "WELSPECS",
}
_NON_HISTORICAL_KEYWORD_ATTRS = {"COMMENT", "FILTER"}


def coalesce_orion_document(document: OrionDocument) -> OrionDocument:
    """Return a copy with matching keyword events merged.

    Keyword events with the same owner, type and timestamp are merged. The first
    event retains its position, and attributes from later matching events are
    applied in source order. Well keyword attributes are then carried forward
    chronologically to later events of the same type. Events that create or
    expand domain objects are kept separate so, for example, same-date
    perforation intervals are not lost.
    """
    result = copy.deepcopy(document)

    def merge_events(
        events: List[OrionEvent], *, inherit_history: bool = False
    ) -> List[OrionEvent]:
        merged: List[OrionEvent] = []
        by_key: Dict[
            Tuple[Union[datetime.date, datetime.datetime], str], OrionEvent
        ] = {}
        attribute_locs: Dict[
            Tuple[Union[datetime.date, datetime.datetime], str], Dict[str, SourceLoc]
        ] = {}
        for event in events:
            event_type = event.event_type.upper()
            if event_type in _NON_COALESCING_EVENT_TYPES:
                merged.append(event)
                continue

            key = (event.event_date, event_type)
            existing = by_key.get(key)
            if existing is None:
                by_key[key] = event
                attribute_locs[key] = {name: event.loc for name in event.attributes}
                merged.append(event)
                continue

            repeated = (
                set(existing.attributes)
                & set(event.attributes) - _NON_HISTORICAL_KEYWORD_ATTRS
            )
            for name in sorted(repeated):
                previous = existing.attributes[name]
                replacement = event.attributes[name]
                if previous.value != replacement.value:
                    result.warnings.append(
                        ParseWarning(
                            f"{_event_context(event)}: conflicting {event_type} "
                            f"attribute '{name}' (previous value on line "
                            f"{attribute_locs[key][name].line}); using "
                            f"{replacement.raw!r}",
                            event.loc,
                        )
                    )

            existing.attributes.update(event.attributes)
            attribute_locs[key].update({name: event.loc for name in event.attributes})
            if "FILTER" in event.attributes:
                existing.filter = event.filter
            existing.loc = event.loc

        if inherit_history:
            history: Dict[str, Dict[str, AttrValue]] = {}
            for event in sorted(merged, key=lambda item: _as_datetime(item.event_date)):
                event_type = event.event_type.upper()
                if event_type in _NON_COALESCING_EVENT_TYPES:
                    continue

                attributes = history.get(event_type, {}).copy()
                attributes.update(event.attributes)
                event.attributes = attributes
                history[event_type] = {
                    name: value
                    for name, value in attributes.items()
                    if name not in _NON_HISTORICAL_KEYWORD_ATTRS
                }

        return merged

    def merge_well_blocks(blocks: List[WellBlock]) -> List[WellBlock]:
        merged_blocks: List[WellBlock] = []
        by_name: Dict[str, WellBlock] = {}
        for block in blocks:
            block_events = block.events
            existing = by_name.get(block.well_name)
            if existing is None:
                block.events = []
                by_name[block.well_name] = block
                merged_blocks.append(block)
            by_name[block.well_name].events.extend(block_events)
        for block in merged_blocks:
            block.events = merge_events(block.events, inherit_history=True)
        return merged_blocks

    def merge_group_blocks(blocks: List[GroupBlock]) -> List[GroupBlock]:
        merged_blocks: List[GroupBlock] = []
        by_name: Dict[str, GroupBlock] = {}
        for block in blocks:
            block_events = block.events
            existing = by_name.get(block.group_name)
            if existing is None:
                block.events = []
                by_name[block.group_name] = block
                merged_blocks.append(block)
            by_name[block.group_name].events.extend(block_events)
        for block in merged_blocks:
            block.events = merge_events(block.events)
        return merged_blocks

    result.wells = merge_well_blocks(result.wells)
    result.groups = merge_group_blocks(result.groups)
    result.schedule_events = merge_events(result.schedule_events)
    return result


def _enum_text(value: Any) -> str:
    """Return the serialized text of a generated enum or plain string."""
    return str(getattr(value, "value", value)).upper()


def _prepare_wellspec_events(
    events: List[OrionEvent], completion_settings: Any, report: ApplyReport
) -> None:
    """Validate WELSPECS attributes and resolve partial updates chronologically."""
    state = WellSpecState(
        group=str(completion_settings.group_name_for_export),
        crossflow=bool(completion_settings.allow_well_cross_flow),
        refdepth=completion_settings.reference_depth_for_export,
        phase=_enum_text(completion_settings.well_type_for_export),
    )

    wellspecs = sorted(
        (event for event in events if event.event_type.upper() == "WELSPECS"),
        key=lambda event: event.event_date,
    )
    for event in wellspecs:
        attrs = event.attributes
        unknown = set(attrs) - _WELLSPEC_KNOWN
        if unknown:
            report.errors.append(
                _event_message(
                    event,
                    f"unknown WELSPECS attribute(s): {', '.join(sorted(unknown))}",
                )
            )
            report.events_skipped += 1
            continue
        if not (set(attrs) - {"COMMENT"}):
            report.errors.append(
                _event_message(event, "WELSPECS needs at least one setting attribute")
            )
            report.events_skipped += 1
            continue

        next_state = copy.copy(state)
        errors: List[str] = []
        if "GROUP" in attrs:
            value = attrs["GROUP"].value
            if not isinstance(value, str) or not value:
                errors.append("GROUP must be a non-empty string")
            else:
                next_state.group = value
        if "CROSSFLOW" in attrs:
            value = attrs["CROSSFLOW"].value
            if not isinstance(value, bool):
                errors.append("CROSSFLOW must be True or False")
            else:
                next_state.crossflow = value
        if "REFDEPTH" in attrs:
            value = attrs["REFDEPTH"].value
            if isinstance(value, bool) or not isinstance(value, (int, float)):
                errors.append("REFDEPTH must be numeric")
            else:
                next_state.refdepth = float(value)
        if "PHASE" in attrs:
            value = attrs["PHASE"].value
            phase = value.upper() if isinstance(value, str) else ""
            if phase not in _WELLSPEC_PHASES:
                errors.append("PHASE must be OIL, GAS, WATER, or LIQUID")
            else:
                next_state.phase = phase

        if errors:
            report.errors.extend(_event_message(event, error) for error in errors)
            report.events_skipped += 1
            continue

        state = next_state
        event.well_spec = copy.copy(state)


def apply_orion_document(
    document: OrionDocument,
    timeline: Any,
    project: Any,
    *,
    case: Any = None,
    on_unknown_well: str = "warn",
    on_unknown_event: str = "warn",
) -> ApplyReport:
    """Apply a parsed :class:`OrionDocument` to a ``WellEventTimeline``.

    Arguments:
        document: The parsed document (see :func:`parse_orion_events`).
        timeline: A ``rips.WellEventTimeline`` object.
        project: A ``rips.Project`` used to resolve well names to well paths.
        case: The ``rips.Case`` used to resolve filter result names and to own
            the combined data filters created from FILTER expressions. Only
            needed when the document uses filters; defaults to the project's
            first case. Raises :class:`RipsError` if a filter is used and no
            case is available, or if a filter references a result that does
            not exist in the case (checked up front, before applying events).
        on_unknown_well: ``"warn"`` (default), ``"error"`` or ``"skip"`` -
            behavior when a well name has no matching well path.
        on_unknown_event: same policy values, for event types that closely
            resemble a misspelled built-in event type. Other unrecognized
            event types are passed through as generic Eclipse keywords.

    Returns:
        ApplyReport: counts plus collected warnings/errors. ``INSERT_DATE`` dates
            from the document are returned as sorted, deduplicated ISO strings
            on ``report_dates`` — they do not create timeline events; pass them
            to ``timeline.generate_schedule_text(additional_dates=...)`` to
            emit them as DATES keywords.
    """
    _validate_policy(on_unknown_well, "on_unknown_well")
    _validate_policy(on_unknown_event, "on_unknown_event")
    document = coalesce_orion_document(document)
    report = ApplyReport()
    report.report_dates = sorted({d.isoformat() for d in document.report_dates})
    report.warnings.extend(
        f"Line {warning.loc.line}: {warning.message}" for warning in document.warnings
    )

    ctx = _prepare_filter_context(document, project, case)

    for well in document.wells:
        well_path = project.well_path_by_name(well.well_name)
        if well_path is None:
            message = f"Unknown well '{well.well_name}' (line {well.loc.line})"
            if on_unknown_well == "error":
                raise RipsError(message)
            if on_unknown_well == "warn":
                report.warnings.append(message)
            report.events_skipped += len(well.events)
            continue

        _prepare_wellspec_events(well.events, well_path.completion_settings(), report)

        for event in well.events:
            event_type = event.event_type.upper()
            dispatch = _EVENT_DISPATCH.get(event_type)
            if dispatch is None:
                typo_of = _suspected_typo(event_type)
                if typo_of is not None:
                    message = _event_message(
                        event,
                        f"Unknown event type '{event.event_type}'; "
                        f"did you mean '{typo_of}'?",
                    )
                    if on_unknown_event == "error":
                        raise RipsError(message)
                    if on_unknown_event == "warn":
                        report.warnings.append(message)
                    report.events_skipped += 1
                    continue
                dispatch = _apply_generic_well_keyword
            dispatch(event, well_path, timeline, report, ctx)

    for group in document.groups:
        for event in group.events:
            _apply_schedule_event(event, timeline, report, group.group_name)

    for event in document.schedule_events:
        _apply_schedule_event(event, timeline, report)

    return report


# Search order for unqualified filter result names.
_UNQUALIFIED_RESULT_SEARCH_ORDER = ("STATIC_NATIVE", "DYNAMIC_NATIVE", "GENERATED")


@dataclass
class _FilterContext:
    """Per-apply-run state for materializing filter expressions on a case."""

    case: Any
    resolved_types: Dict[FilterTerm, str] = field(default_factory=dict)
    combined_by_key: Dict[str, Any] = field(default_factory=dict)
    properties_by_type: Dict[str, List[str]] = field(default_factory=dict)

    def available(self, result_type: str) -> List[str]:
        if result_type not in self.properties_by_type:
            try:
                names = list(self.case.available_properties(result_type))
            except RipsError:
                # The gRPC service reports NOT_FOUND when the case has no
                # results of this type at all; treat that as an empty list.
                names = []
            self.properties_by_type[result_type] = names
        return self.properties_by_type[result_type]


def _prepare_filter_context(
    document: OrionDocument, project: Any, case: Any
) -> Optional[_FilterContext]:
    """Build a filter context when the document uses filters; else None.

    Resolves every filter term's result type up front so a missing result
    raises before any event has been applied.
    """
    filtered_events: List[Tuple[OrionEvent, EventFilter]] = []
    for well in document.wells:
        for event in well.events:
            if event.filter is not None:
                filtered_events.append((event, event.filter))
    if not filtered_events:
        return None

    if case is None:
        cases = project.cases()
        case = cases[0] if cases else None
    if case is None:
        raise RipsError(
            "The document uses FILTER but no case is available; pass case= "
            "or load a case in the project"
        )

    ctx = _FilterContext(case=case)
    missing: List[str] = []
    for event, event_filter in filtered_events:
        label = event_filter.name or event_filter.expr.raw
        for term in event_filter.expr.terms:
            if term in ctx.resolved_types:
                continue
            resolved = _resolve_result_type(ctx, term)
            if resolved is not None:
                ctx.resolved_types[term] = resolved
                continue
            if term.result_type is not None:
                missing.append(
                    _event_message(
                        event,
                        f"filter '{label}': result '{term.result_name}' not found "
                        f"among {term.result_type} results",
                    )
                )
            else:
                missing.append(
                    _event_message(
                        event,
                        f"filter '{label}': result '{term.result_name}' not found "
                        f"(searched {', '.join(_UNQUALIFIED_RESULT_SEARCH_ORDER)})",
                    )
                )
    if missing:
        raise RipsError("\n".join(missing))
    return ctx


def _resolve_result_type(ctx: _FilterContext, term: FilterTerm) -> Optional[str]:
    """Return the result type holding this term's result, or None if missing."""
    search_order = (
        (term.result_type,)
        if term.result_type is not None
        else _UNQUALIFIED_RESULT_SEARCH_ORDER
    )
    for result_type in search_order:
        if term.result_name in ctx.available(result_type):
            return result_type
    return None


def _materialize_filter(ctx: _FilterContext, event_filter: EventFilter) -> Any:
    """Create (or reuse) the case-level combined filter for an event filter.

    Declared filters are created once per declaration name and shared between
    the perforations that reference them; inline filters are shared when their
    expression text is identical.
    """
    key = event_filter.name or event_filter.expr.raw
    existing = ctx.combined_by_key.get(key)
    if existing is not None:
        return existing

    expr = event_filter.expr
    combined = ctx.case.data_filter_collection().add_combined_filter(
        name=event_filter.name or "", combine_mode=expr.combine_mode
    )
    for term in expr.terms:
        property_filter = combined.add_property_filter(
            result_variable=term.result_name,
            result_type=ctx.resolved_types[term],
        )
        # Only one bound is set; the other keeps the default result min/max,
        # so '>' and '>=' are equivalent (bounds are inclusive).
        if term.op in (">", ">="):
            property_filter.lower_bound = term.value
        else:
            property_filter.upper_bound = term.value
        property_filter.update()

    ctx.combined_by_key[key] = combined
    return combined


def _suspected_typo(event_type: str) -> Optional[str]:
    """Return the built-in event type this one looks like a misspelling of."""
    if event_type == "TUBING":
        return "SEGMENT"
    close = difflib.get_close_matches(event_type, _EVENT_DISPATCH, n=1, cutoff=0.8)
    return close[0] if close else None


def _apply_member_event(
    event: OrionEvent,
    timeline: Any,
    report: ApplyReport,
    group_name: Optional[str],
) -> None:
    """Expand a GROUP MEMBER event into one GRUPTREE event per member."""
    if group_name is None:
        report.errors.append(_event_message(event, "MEMBER needs a GROUP block"))
        report.events_skipped += 1
        return

    unknown = set(event.attributes) - {"MEMBERS", "COMMENT"}
    if unknown:
        report.errors.append(
            _event_message(
                event,
                f"unknown MEMBER attribute(s): {', '.join(sorted(unknown))}",
            )
        )
        report.events_skipped += 1
        return
    if "MEMBERS" not in event.attributes:
        report.errors.append(
            _event_message(event, "MEMBER missing required attribute: MEMBERS")
        )
        report.events_skipped += 1
        return

    raw_members = str(event.attributes["MEMBERS"].value)
    members = [member.strip() for member in raw_members.split(",")]
    if not members or any(not member for member in members):
        report.errors.append(
            _event_message(
                event, "MEMBERS must be a comma-delimited list of non-empty names"
            )
        )
        report.events_skipped += 1
        return

    unique_members = list(dict.fromkeys(members))
    for member in unique_members:
        timeline_event = timeline.add_keyword_event(
            event_date=_iso_event_date(event.event_date),
            keyword_name="GRUPTREE",
            keyword_data={"CHILD_GROUP": member, "PARENT_GROUP": group_name},
        )
        _apply_event_comment(event, timeline_event)
        report.events_applied += 1


def _apply_schedule_event(
    event: OrionEvent,
    timeline: Any,
    report: ApplyReport,
    group_name: Optional[str] = None,
) -> None:
    """Apply one GROUP- or SCHEDULE-block event as an Eclipse keyword."""
    event_type = event.event_type.upper()
    if event_type == "RAW_TEXT":
        timeline.add_raw_text_event(
            event_date=_iso_event_date(event.event_date),
            text=event.raw_text,
            placement=event.raw_placement,
            anchor_keyword=event.raw_anchor or "",
            priority=event.raw_priority,
        )
        report.events_applied += 1
        return
    if event_type == "RESTART":
        timeline.add_keyword_event(
            event_date=_iso_event_date(event.event_date),
            keyword_name="RESTART",
            keyword_data={},
        )
        report.events_applied += 1
        return
    if event_type == "MEMBER":
        _apply_member_event(event, timeline, report, group_name)
        return
    if event_type in _COMPLETION_EVENT_TYPES:
        report.errors.append(
            _event_message(
                event,
                f"{event_type} is a completion event and needs a WELL block, "
                "not GROUP or SCHEDULE",
            )
        )
        report.events_skipped += 1
        return

    keyword_data: Dict[str, Any] = {}
    for key, attr in event.attributes.items():
        if key == "COMMENT":
            continue
        if key in _IGNORED_KEYWORD_ATTRS:
            report.warnings.append(
                _event_message(
                    event,
                    f"attribute '{key}' on {event_type} is ignored (not yet supported)",
                )
            )
            continue
        keyword_data[key] = attr.value
    if group_name is not None:
        keyword_data["GROUP"] = group_name

    try:
        timeline_event = timeline.add_keyword_event(
            event_date=_iso_event_date(event.event_date),
            keyword_name=event_type,
            keyword_data=keyword_data,
        )
    except RipsError as exc:
        _record_event_exception(event, exc, report)
        return
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


def _validate_policy(value: str, name: str) -> None:
    if value not in _POLICIES:
        raise ValueError(f"{name} must be one of {_POLICIES}, got {value!r}")


def _check_completion_attrs(
    event: OrionEvent,
    type_name: str,
    known: Set[str],
    required: Tuple[str, ...],
    report: ApplyReport,
    ignored: Set[str] = _COMPLETION_IGNORED,
) -> bool:
    """Validate a completion event's attributes; False means skip the event."""
    attrs = event.attributes
    unknown = set(attrs) - known - ignored
    if unknown:
        report.errors.append(
            _event_message(
                event,
                f"unknown {type_name} attribute(s): {', '.join(sorted(unknown))}",
            )
        )
        report.events_skipped += 1
        return False

    for key in sorted(ignored & set(attrs)):
        report.warnings.append(
            _event_message(
                event,
                f"attribute '{key}' on {type_name} is ignored (not yet supported)",
            )
        )

    missing = [name for name in required if name not in attrs]
    if missing:
        report.errors.append(
            _event_message(
                event,
                f"{type_name} missing required attribute(s): {', '.join(missing)}",
            )
        )
        report.events_skipped += 1
        return False
    return True


def _apply_perforation(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if not _check_completion_attrs(
        event, "PERFORATION", _PERF_KNOWN, _PERF_REQUIRED, report, ignored=set()
    ):
        return

    attrs = event.attributes
    try:
        kwargs: Dict[str, Any] = {
            "event_date": _iso_event_date(event.event_date),
            "well_path": well_path,
            "start_md": float(_as_number(attrs["MDSTART"], event.loc)),
            "end_md": float(_as_number(attrs["MDEND"], event.loc)),
            "state": "OPEN",
        }
        if "DIAMETER" in attrs:
            kwargs["diameter"] = float(_as_number(attrs["DIAMETER"], event.loc))
        if "SKIN" in attrs:
            kwargs["skin_factor"] = float(_as_number(attrs["SKIN"], event.loc))
        if "COMPLETION_NUMBER" in attrs:
            kwargs["completion_number"] = int(
                _as_number(attrs["COMPLETION_NUMBER"], event.loc)
            )
    except OrionParseError as exc:
        _record_event_parse_error(event, exc, report)
        return

    perf_event = timeline.add_perf_event(**kwargs)
    if event.filter is not None and ctx is not None:
        perf_event.add_filter(filter=_materialize_filter(ctx, event.filter))
    _apply_event_comment(event, perf_event)
    report.events_applied += 1


def _apply_segment(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if not _check_completion_attrs(
        event, "SEGMENT", _SEGMENT_KNOWN, _SEGMENT_REQUIRED, report
    ):
        return

    attrs = event.attributes
    try:
        start_md = float(_as_number(attrs["MDSTART"], event.loc))
        end_md = float(_as_number(attrs["MDEND"], event.loc))
        kwargs: Dict[str, Any] = {
            "event_date": _iso_event_date(event.event_date),
            "well_path": well_path,
            "start_md": start_md,
            "end_md": end_md,
        }
        if "INNER_DIAMETER" in attrs:
            kwargs["inner_diameter"] = float(
                _as_number(attrs["INNER_DIAMETER"], event.loc)
            )
        if "ROUGHNESS" in attrs:
            kwargs["roughness"] = float(_as_number(attrs["ROUGHNESS"], event.loc))
        pressure_components = None
        if "PRESSURE_COMPONENTS" in attrs:
            pressure_components = str(attrs["PRESSURE_COMPONENTS"].value).upper()
            if pressure_components not in _PRESSURE_COMPONENTS:
                raise OrionParseError(
                    "PRESSURE_COMPONENTS must be H--, HF-, or HFA", event.loc
                )
    except OrionParseError as exc:
        _record_event_parse_error(event, exc, report)
        return

    timeline_event = timeline.add_tubing_event(**kwargs)
    well_path.completion_settings().add_custom_segment_interval(
        start_md=start_md, end_md=end_md
    )
    if pressure_components is not None:
        msw_settings = well_path.msw_settings()
        msw_settings.pressure_drop = pressure_components
        msw_settings.update()
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


# VALVE attribute -> add_valve_event keyword-argument name (numeric values).
_VALVE_NUMERIC_KWARGS = {
    "CV": "flow_coefficient",
    "AREA": "area",
    "AICD_STRENGTH": "aicd_strength",
    "AICD_DENSITY_CALIB_FLUID": "aicd_density_calib_fluid",
    "AICD_VISCOSITY_CALIB_FLUID": "aicd_viscosity_calib_fluid",
    "AICD_VOL_FLOW_EXP": "aicd_vol_flow_exp",
    "AICD_VISC_FUNC_EXP": "aicd_visc_func_exp",
}


def _apply_valve(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if not _check_completion_attrs(
        event, "VALVE", _VALVE_KNOWN, _VALVE_REQUIRED, report
    ):
        return

    attrs = event.attributes
    try:
        kwargs: Dict[str, Any] = {
            "event_date": _iso_event_date(event.event_date),
            "well_path": well_path,
            "measured_depth": float(_as_number(attrs["MD"], event.loc)),
            "valve_type": str(attrs["TYPE"].value),
        }
        if "STATE" in attrs:
            kwargs["state"] = str(attrs["STATE"].value)
        for attr_name, kwarg_name in _VALVE_NUMERIC_KWARGS.items():
            if attr_name in attrs:
                kwargs[kwarg_name] = float(_as_number(attrs[attr_name], event.loc))
    except OrionParseError as exc:
        _record_event_parse_error(event, exc, report)
        return

    timeline_event = timeline.add_valve_event(**kwargs)
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


def _apply_state(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if not _check_completion_attrs(
        event, "STATE", _STATE_KNOWN, _STATE_REQUIRED, report
    ):
        return

    timeline_event = timeline.add_state_event(
        event_date=_iso_event_date(event.event_date),
        well_path=well_path,
        well_state=str(event.attributes["STATE"].value),
    )
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


def _apply_wellspec(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if event.well_spec is None:
        return

    state = event.well_spec
    timeline_event = timeline.add_wellspec_event(
        event_date=_iso_event_date(event.event_date),
        well_path=well_path,
        group_name=state.group,
        allow_cross_flow=state.crossflow,
        reference_depth=state.refdepth,
        well_type=state.phase,
    )
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


def _apply_keyword(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    keyword_name: str,
    field_map: Dict[str, str],
) -> None:
    keyword_data: Dict[str, Any] = {"WELL": well_path.name}
    for key, attr in event.attributes.items():
        if key == "COMMENT":
            continue
        if key in _IGNORED_KEYWORD_ATTRS:
            report.warnings.append(
                _event_message(
                    event,
                    f"attribute '{key}' on {keyword_name} is ignored "
                    "(not yet supported)",
                )
            )
            continue
        keyword_data[field_map.get(key, key)] = attr.value

    try:
        timeline_event = timeline.add_well_keyword_event(
            event_date=_iso_event_date(event.event_date),
            well_path=well_path,
            keyword_name=keyword_name,
            keyword_data=keyword_data,
        )
    except RipsError as exc:
        _record_event_exception(event, exc, report)
        return
    _apply_event_comment(event, timeline_event)
    report.events_applied += 1


def _apply_wconhist(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    _apply_keyword(event, well_path, timeline, report, "WCONHIST", _WCONHIST_FIELD_MAP)


def _apply_weltarg(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    _apply_keyword(event, well_path, timeline, report, "WELTARG", _WELTARG_FIELD_MAP)


def _as_number(attr: AttrValue, loc: SourceLoc) -> Union[int, float]:
    if isinstance(attr.value, bool) or not isinstance(attr.value, (int, float)):
        raise OrionParseError(f"Expected a numeric value, got {attr.raw!r}", loc)
    return attr.value


def _apply_generic_well_keyword(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    _apply_keyword(event, well_path, timeline, report, event.event_type.upper(), {})


_EventDispatch = Callable[
    [OrionEvent, Any, Any, ApplyReport, Optional[_FilterContext]], None
]

# Built-in event types. Any other event type inside a WELL block is passed
# through as a generic Eclipse well keyword (unless it looks like a typo of a
# built-in, which is governed by the on_unknown_event policy).
_EVENT_DISPATCH: Dict[str, _EventDispatch] = {
    "PERFORATION": _apply_perforation,
    "SEGMENT": _apply_segment,
    "VALVE": _apply_valve,
    "STATE": _apply_state,
    "WELSPECS": _apply_wellspec,
    "WCONHIST": _apply_wconhist,
    "WELTARG": _apply_weltarg,
}

# Completion event types that require a well and cannot appear in a SCHEDULE
# block or be emitted as Eclipse keywords.
_COMPLETION_EVENT_TYPES = (
    "PERFORATION",
    "SEGMENT",
    "VALVE",
    "STATE",
    "WELSPECS",
)


# ---------------------------------------------------------------------------
# Standalone validator CLI: python3 -m rips.orion_events <file.orion>
# ---------------------------------------------------------------------------


def _apply_to_running_instance(document: OrionDocument) -> int:
    """Apply a parsed document to the well event timeline of a running
    ResInsight instance found through the RESINSIGHT_GRPC_PORT environment
    variable. Returns a process exit code."""

    import rips

    resinsight = rips.Instance.find()
    if resinsight is None:
        print("Error: no running ResInsight instance found")
        return 1

    project = resinsight.project
    well_path_collections = project.descendants(rips.WellPathCollection)
    if not well_path_collections:
        print("Error: the project has no well path collection")
        return 1
    timeline = well_path_collections[0].event_timeline()

    # A case is only needed to materialize FILTER declarations; default to the
    # first case when one is loaded.
    cases = project.cases()
    case = cases[0] if cases else None

    try:
        report = apply_orion_document(
            document, timeline, project, case=case, on_unknown_well="warn"
        )
    except RipsError as exc:
        print(f"Error: {exc}")
        return 1

    print(f"  Events applied: {report.events_applied}")
    print(f"  Events skipped: {report.events_skipped}")
    if report.report_dates:
        print(f"  Report dates:   {', '.join(report.report_dates)}")
    for warning in report.warnings:
        print(f"  Warning: {warning}")
    for error in report.errors:
        print(f"  Error: {error}")
    return 1 if report.errors else 0


def _cli(argv: Optional[List[str]] = None) -> int:
    import argparse

    arg_parser = argparse.ArgumentParser(
        prog="python3 -m rips.orion_events",
        description="Validate an ORIONEVENTS file (parse only; no ResInsight "
        "needed), and optionally apply it to a running ResInsight instance.",
    )
    arg_parser.add_argument("file", help="path to the ORIONEVENTS file")
    arg_parser.add_argument(
        "--apply",
        action="store_true",
        help="apply the events to a running ResInsight instance after validating",
    )
    args = arg_parser.parse_args(argv)

    try:
        document = parse_orion_events_file(args.file)
    except OSError as exc:
        print(f"Error: {exc}")
        return 1
    except OrionParseError as exc:
        for issue in exc.errors:
            if issue.loc is not None:
                print(f"Line {issue.loc.line}: {issue.message}")
            else:
                print(issue.message)
        print(f"{args.file}: {len(exc.errors)} error(s) found")
        return 1

    event_count = sum(len(well.events) for well in document.wells)
    group_event_count = sum(len(group.events) for group in document.groups)
    print(
        f"{args.file}: OK (ORIONEVENTS {document.version}, "
        f"units {document.unit_system})"
    )
    print(
        f"  {len(document.variables)} variable(s), {len(document.wells)} "
        f"well block(s), {event_count} well event(s), "
        f"{len(document.groups)} group block(s), {group_event_count} group event(s), "
        f"{len(document.schedule_events)} schedule event(s), "
        f"{len(document.report_dates)} report date(s)"
    )
    normalized = coalesce_orion_document(document)
    for warning in normalized.warnings:
        print(f"  Warning line {warning.loc.line}: {warning.message}")

    if args.apply:
        return _apply_to_running_instance(document)
    return 0


if __name__ == "__main__":
    import sys

    sys.exit(_cli())
