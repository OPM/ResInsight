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
    statement       = unit_directive | declaration | report_line | well_block_open
                    | group_block_open | schedule_block_open | event_line ;
    unit_directive  = "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;
    report_line     = "REPORT" , date_expr ;            (* REPORT 2024-06-01 *)

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
    event_line      = "@" , date_expr , event_type , { attribute } ;

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
  ``GROUP``, ``SCHEDULE`` or ``@``. Anything else is an error. Keywords are
  uppercase and case-sensitive (the ``DAYS`` suffix is also accepted as ``days``).
* Comments start with ``#`` (outside of double quotes) and run to end of line.
* Variables are **typed**: ``DATE``, ``DURATION`` (whole days), ``WELL``
  (well-name alias) and ``FILTER`` (cell filter expression) declarations share
  one namespace and must precede use.
  Using a variable of the wrong type is an error that cites both the use and
  the declaration site. Redeclaring a name with the same type warns and the
  last value wins; redeclaring with a different type is an error.
* Date arithmetic is a chain of signed whole-day terms, each an integer or a
  ``DURATION`` variable: ``@START + RAMP - 2``. Whitespace around ``+``/``-``
  is optional but conventional. An event date may carry a time-of-day
  (``@2024-05-15T14:45:30.500``), which the schedule generator preserves as
  the optional TIME field of the DATES keyword.
* ``WELL <ident>`` opens an event block for a declared ``WELL`` alias;
  ``WELL "<name>"`` opens a block for the literal well name and never consults
  variables. A ``WELL`` line containing ``=`` is always a declaration. A bare
  ``GROUP "<name>"`` opens a block of group-level Eclipse keyword events; the
  group name is injected as the ``GROUP`` item when each event is applied.
  A bare ``SCHEDULE`` line opens a block of schedule-level keyword events not
  tied to any well (RPTRST, GRUPTREE, TUNING, ...). Empty blocks are legal.
* ``REPORT <date_expr>`` (one date per line, anywhere after the header) names
  a date that should appear as a bare ``DATES`` keyword in the generated
  schedule even when no events fall on it — in Eclipse/Flow a ``DATES`` entry
  ensures a summary report at that date. The dates are collected on
  :attr:`OrionDocument.report_dates` and surfaced by the applier as sorted ISO
  strings on :attr:`ApplyReport.report_dates`, ready to pass to
  ``WellEventTimeline.generate_schedule_text(additional_dates=...)``. A
  ``REPORT`` line is not tied to any well and does not close an open block.
* Double quotes are used everywhere: well names, filter expressions and
  attribute values, e.g. ``FILTER="SOIL > 0.8 AND PERMX > 200"``.
* Every attribute is ``KEY=VALUE``; bare positional tokens are rejected.
* Event types inside a WELL block are either the built-in completion events
  ``PERFORATION``, ``TUBING``, ``VALVE`` and ``STATE``, or any Eclipse well
  keyword (``WCONHIST``, ``WELTARG``, ``WRFTPLT``, ``WCONPROD``, ...), which
  is passed through generically with the well name injected as WELL. Event
  types inside a GROUP block are Eclipse group keywords with the group name
  injected as GROUP. Event types inside a SCHEDULE block are Eclipse schedule
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
class OrionEvent:
    """One event line in an enclosing WELL, GROUP or SCHEDULE block."""

    event_type: str
    event_date: Union[datetime.date, datetime.datetime]
    attributes: Dict[str, AttrValue]
    loc: SourceLoc
    filter: Optional[EventFilter] = None


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
    "REPORT",
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
_REPORT_RE = re.compile(rf"^REPORT\s+{_DATE_BASE}{_TERMS}$")
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
_EVENT_RE = re.compile(rf"^@\s*{_DATE_BASE}{_TERMS}\s+(?P<rest>.+)$")
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
    report_dates: List[Union[datetime.date, datetime.datetime]] = []
    warnings: List[ParseWarning] = []
    errors: List[ParseIssue] = []
    # Event lines append to the current sink: a WellBlock's event list or the
    # document-level schedule_events list.
    current_events: Optional[List[OrionEvent]] = None

    for lineno, raw_line in enumerate(text.splitlines(), start=1):
        loc = SourceLoc(line=lineno, text=raw_line)
        line = _strip_comment(raw_line).strip()
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

        try:
            current_events = _parse_line(
                line,
                loc,
                variables,
                wells,
                groups,
                schedule_events,
                report_dates,
                warnings,
                current_events,
                unit_holder,
            )
        except OrionParseError as exc:
            errors.extend(exc.errors)
            if line.split(None, 1)[0] in ("WELL", "GROUP", "SCHEDULE") or (
                line.startswith("@") and current_events is None
            ):
                # Suppress cascading errors from lines belonging to a broken
                # (or missing) block: swallow them into a discarded list.
                current_events = []

    if version is None:
        raise OrionParseError("Empty file: missing 'ORIONEVENTS' header")

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
    report_dates: List[Union[datetime.date, datetime.datetime]],
    warnings: List[ParseWarning],
    current_events: Optional[List[OrionEvent]],
    unit_holder: List[str],
) -> Optional[List[OrionEvent]]:
    """Dispatch one non-header line; returns the current event sink."""
    if line.startswith("@"):
        if current_events is None:
            raise OrionParseError(
                "Event line found before any WELL or SCHEDULE block", loc
            )
        current_events.append(_parse_event_line(line, variables, loc))
        return current_events

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

    if first == "REPORT":
        match = _REPORT_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed REPORT line: {line!r} "
                "(expected REPORT <iso-date|DATE-var> [+|- <days|DURATION-var> ...])",
                loc,
            )
        report_dates.append(
            _eval_date_expr(match.group("base"), match.group("terms"), variables, loc)
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


# Policies for handling unresolved references: skip silently, warn, or raise.
_POLICIES = ("warn", "error", "skip")

# Attributes accepted on a keyword event but intentionally not emitted.
_IGNORED_KEYWORD_ATTRS = {"FILTER"}

# Completion event attribute handling: (required, known-optional) per type.
# FILTER is applied on PERFORATION events; it is accepted on the other
# completion events but ignored with a warning.
_PERF_REQUIRED = ("MDSTART", "MDEND")
_PERF_KNOWN = {"MDSTART", "MDEND", "RADIUS", "SKIN", "COMPLETION_NUMBER", "FILTER"}
_TUBING_REQUIRED = ("MDSTART", "MDEND")
_TUBING_KNOWN = {"MDSTART", "MDEND", "INNER_DIAMETER", "ROUGHNESS"}
_VALVE_REQUIRED = ("MD", "TYPE")
_VALVE_KNOWN = {"MD", "TYPE", "STATE", "CV", "AREA"} | {
    "AICD_STRENGTH",
    "AICD_DENSITY_CALIB_FLUID",
    "AICD_VISCOSITY_CALIB_FLUID",
    "AICD_VOL_FLOW_EXP",
    "AICD_VISC_FUNC_EXP",
}
_STATE_REQUIRED = ("STATE",)
_STATE_KNOWN = {"STATE"}
_COMPLETION_IGNORED = {"FILTER"}
_PERF_IGNORED = _COMPLETION_IGNORED  # backwards-compatible alias

# ORIONEVENTS -> Eclipse item-name translations per keyword.
_WCONHIST_FIELD_MAP = {"VFP": "VFP_TABLE"}
_WELTARG_FIELD_MAP = {"VALUE": "NEW_VALUE"}


def _iso_event_date(event_date: Union[datetime.date, datetime.datetime]) -> str:
    """Format an event date for the timeline API, keeping ms time-of-day."""
    if isinstance(event_date, datetime.datetime):
        if event_date.microsecond:
            return event_date.isoformat(timespec="milliseconds")
        return event_date.isoformat()
    return event_date.isoformat()


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
        ApplyReport: counts plus collected warnings/errors. ``REPORT`` dates
            from the document are returned as sorted, deduplicated ISO strings
            on ``report_dates`` — they do not create timeline events; pass them
            to ``timeline.generate_schedule_text(additional_dates=...)`` to
            emit them as DATES keywords.
    """
    _validate_policy(on_unknown_well, "on_unknown_well")
    _validate_policy(on_unknown_event, "on_unknown_event")
    report = ApplyReport()
    report.report_dates = sorted({d.isoformat() for d in document.report_dates})

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

        for event in well.events:
            event_type = event.event_type.upper()
            dispatch = _EVENT_DISPATCH.get(event_type)
            if dispatch is None:
                typo_of = _suspected_typo(event_type)
                if typo_of is not None:
                    message = (
                        f"Unknown event type '{event.event_type}' "
                        f"(line {event.loc.line}); did you mean '{typo_of}'?"
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
                    f"Line {event.loc.line}: filter '{label}': result "
                    f"'{term.result_name}' not found among "
                    f"{term.result_type} results"
                )
            else:
                missing.append(
                    f"Line {event.loc.line}: filter '{label}': result "
                    f"'{term.result_name}' not found (searched "
                    f"{', '.join(_UNQUALIFIED_RESULT_SEARCH_ORDER)})"
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
    close = difflib.get_close_matches(event_type, _EVENT_DISPATCH, n=1, cutoff=0.8)
    return close[0] if close else None


def _apply_schedule_event(
    event: OrionEvent,
    timeline: Any,
    report: ApplyReport,
    group_name: Optional[str] = None,
) -> None:
    """Apply one GROUP- or SCHEDULE-block event as an Eclipse keyword."""
    event_type = event.event_type.upper()
    if event_type in _COMPLETION_EVENT_TYPES:
        report.errors.append(
            f"Line {event.loc.line}: {event_type} is a completion event and "
            "needs a WELL block, not GROUP or SCHEDULE"
        )
        report.events_skipped += 1
        return

    keyword_data: Dict[str, Any] = {}
    for key, attr in event.attributes.items():
        if key in _IGNORED_KEYWORD_ATTRS:
            report.warnings.append(
                f"Line {event.loc.line}: attribute '{key}' on {event_type} "
                "is ignored (not yet supported)"
            )
            continue
        keyword_data[key] = attr.value
    if group_name is not None:
        keyword_data["GROUP"] = group_name

    timeline.add_keyword_event(
        event_date=_iso_event_date(event.event_date),
        keyword_name=event_type,
        keyword_data=keyword_data,
    )
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
            f"Line {event.loc.line}: unknown {type_name} attribute(s): "
            f"{', '.join(sorted(unknown))}"
        )
        report.events_skipped += 1
        return False

    for key in sorted(ignored & set(attrs)):
        report.warnings.append(
            f"Line {event.loc.line}: attribute '{key}' on {type_name} "
            "is ignored (not yet supported)"
        )

    missing = [name for name in required if name not in attrs]
    if missing:
        report.errors.append(
            f"Line {event.loc.line}: {type_name} missing required "
            f"attribute(s): {', '.join(missing)}"
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
        if "RADIUS" in attrs:
            kwargs["diameter"] = 2.0 * float(_as_number(attrs["RADIUS"], event.loc))
        if "SKIN" in attrs:
            kwargs["skin_factor"] = float(_as_number(attrs["SKIN"], event.loc))
        if "COMPLETION_NUMBER" in attrs:
            kwargs["completion_number"] = int(
                _as_number(attrs["COMPLETION_NUMBER"], event.loc)
            )
    except OrionParseError as exc:
        report.errors.append(str(exc))
        report.events_skipped += 1
        return

    perf_event = timeline.add_perf_event(**kwargs)
    if event.filter is not None and ctx is not None:
        perf_event.add_filter(filter=_materialize_filter(ctx, event.filter))
    report.events_applied += 1


def _apply_tubing(
    event: OrionEvent,
    well_path: Any,
    timeline: Any,
    report: ApplyReport,
    ctx: Optional[_FilterContext] = None,
) -> None:
    if not _check_completion_attrs(
        event, "TUBING", _TUBING_KNOWN, _TUBING_REQUIRED, report
    ):
        return

    attrs = event.attributes
    try:
        kwargs: Dict[str, Any] = {
            "event_date": _iso_event_date(event.event_date),
            "well_path": well_path,
            "start_md": float(_as_number(attrs["MDSTART"], event.loc)),
            "end_md": float(_as_number(attrs["MDEND"], event.loc)),
        }
        if "INNER_DIAMETER" in attrs:
            kwargs["inner_diameter"] = float(
                _as_number(attrs["INNER_DIAMETER"], event.loc)
            )
        if "ROUGHNESS" in attrs:
            kwargs["roughness"] = float(_as_number(attrs["ROUGHNESS"], event.loc))
    except OrionParseError as exc:
        report.errors.append(str(exc))
        report.events_skipped += 1
        return

    timeline.add_tubing_event(**kwargs)
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
        report.errors.append(str(exc))
        report.events_skipped += 1
        return

    timeline.add_valve_event(**kwargs)
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

    timeline.add_state_event(
        event_date=_iso_event_date(event.event_date),
        well_path=well_path,
        well_state=str(event.attributes["STATE"].value),
    )
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
        if key in _IGNORED_KEYWORD_ATTRS:
            report.warnings.append(
                f"Line {event.loc.line}: attribute '{key}' on {keyword_name} "
                "is ignored (not yet supported)"
            )
            continue
        keyword_data[field_map.get(key, key)] = attr.value

    timeline.add_well_keyword_event(
        event_date=_iso_event_date(event.event_date),
        well_path=well_path,
        keyword_name=keyword_name,
        keyword_data=keyword_data,
    )
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
    "TUBING": _apply_tubing,
    "VALVE": _apply_valve,
    "STATE": _apply_state,
    "WCONHIST": _apply_wconhist,
    "WELTARG": _apply_weltarg,
}

# Completion event types that require a well and cannot appear in a SCHEDULE
# block or be emitted as Eclipse keywords.
_COMPLETION_EVENT_TYPES = ("PERFORATION", "TUBING", "VALVE", "STATE")


# ---------------------------------------------------------------------------
# Standalone validator CLI: python3 -m rips.orion_events <file.orion>
# ---------------------------------------------------------------------------


def _cli(argv: Optional[List[str]] = None) -> int:
    import argparse

    arg_parser = argparse.ArgumentParser(
        prog="python3 -m rips.orion_events",
        description="Validate an ORIONEVENTS file (parse only; no ResInsight needed).",
    )
    arg_parser.add_argument("file", help="path to the ORIONEVENTS file")
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
    for warning in document.warnings:
        print(f"  Warning line {warning.loc.line}: {warning.message}")
    return 0


if __name__ == "__main__":
    import sys

    sys.exit(_cli())
