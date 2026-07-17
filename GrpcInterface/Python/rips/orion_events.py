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
    statement       = unit_directive | declaration | well_block_open | event_line ;
    unit_directive  = "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;

    declaration     = date_decl | duration_decl | well_decl ;
    date_decl       = "DATE" , ident , "=" , date_expr ;         (* DATE X = 2018-03-01 + 9 *)
    duration_decl   = "DURATION" , ident , "=" , duration_expr ; (* DURATION RAMP = 5 DAYS *)
    well_decl       = "WELL" , ident , "=" , quoted_string ;     (* WELL A1 = "55_33-A-1" *)

    well_block_open = "WELL" , ( quoted_string | ident ) ;       (* no "=" present *)
    event_line      = "@" , date_expr , event_type , { attribute } ;

    date_expr       = ( iso_date | date_ident ) , { sign , term } ;
    duration_expr   = ( integer | dur_ident ) , { sign , term } , [ "DAYS" | "days" ] ;
    term            = integer | dur_ident ;             (* whole days *)
    sign            = "+" | "-" ;
    iso_date        = 4digit , "-" , 2digit , "-" , 2digit ;
    ident           = letter_or_underscore , { word_char } ;
    attribute       = ident , "=" , ( quoted_string | bareword ) ;
    comment         = "#" , rest-of-line ;              (* line or trailing *)

Notes on the grammar:

* The format is line-oriented; every non-blank line is dispatched on its first
  token: ``ORIONEVENTS`` (once), ``UNIT``, ``DATE``, ``DURATION``, ``WELL`` or
  ``@``. Anything else is an error. Keywords are uppercase and case-sensitive
  (the ``DAYS`` suffix is also accepted as ``days``).
* Comments start with ``#`` (outside of double quotes) and run to end of line.
* Variables are **typed**: ``DATE``, ``DURATION`` (whole days) and ``WELL``
  (well-name alias) declarations share one namespace and must precede use.
  Using a variable of the wrong type is an error that cites both the use and
  the declaration site. Redeclaring a name with the same type warns and the
  last value wins; redeclaring with a different type is an error.
* Date arithmetic is a chain of signed whole-day terms, each an integer or a
  ``DURATION`` variable: ``@START + RAMP - 2``. Whitespace around ``+``/``-``
  is optional but conventional.
* ``WELL <ident>`` opens an event block for a declared ``WELL`` alias;
  ``WELL "<name>"`` opens a block for the literal well name and never consults
  variables. A ``WELL`` line containing ``=`` is always a declaration. Empty
  well blocks are legal.
* Double quotes are used everywhere: well names and attribute values, e.g.
  ``FILTER="SOIL(0) > 0.8 AND PERMX > 200"``.
* Every attribute is ``KEY=VALUE``; bare positional tokens are rejected. Any
  key parses; keys the applier does not support yet (``FILTER``, ``PERFID``,
  ``DSHIFT``) are ignored with a warning when the document is applied.
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
from typing import Any, Callable, Dict, List, Optional, Union

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
class OrionValue:
    """A typed variable: kind is ``DATE``, ``DURATION`` or ``WELL``."""

    kind: str
    value: Union[datetime.date, int, str]
    loc: SourceLoc


@dataclass(frozen=True)
class AttrValue:
    """A single ``KEY=VALUE`` attribute with its type-inferred value."""

    raw: str
    value: AttrScalar
    quoted: bool


@dataclass
class OrionEvent:
    """One event line: a dated action on the enclosing well."""

    event_type: str
    event_date: datetime.date
    attributes: Dict[str, AttrValue]
    loc: SourceLoc


@dataclass
class WellBlock:
    """A ``WELL`` block header followed by its events."""

    well_name: str
    events: List[OrionEvent] = field(default_factory=list)
    loc: SourceLoc = SourceLoc(0, "")


@dataclass
class OrionDocument:
    """Parsed, lossless representation of an ORIONEVENTS file."""

    version: str
    unit_system: str = "METRIC"
    variables: Dict[str, OrionValue] = field(default_factory=dict)
    wells: List[WellBlock] = field(default_factory=list)
    warnings: List[ParseWarning] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Layer A: pure parser
# ---------------------------------------------------------------------------

_KEYWORDS = ("ORIONEVENTS", "UNIT", "DATE", "DURATION", "WELL")

_IDENT = r"[A-Za-z_]\w*"
_DATE_BASE = rf"(?P<base>\d{{4}}-\d{{2}}-\d{{2}}|{_IDENT})"
_TERMS = rf"(?P<terms>(?:\s*[-+]\s*(?:\d+|{_IDENT}))*)"

_HEADER_RE = re.compile(r"^ORIONEVENTS\s+(?P<version>\d+\.\d+)$")
_UNIT_RE = re.compile(r"^UNIT\s+(?P<unit>METRIC|FIELD|LAB)$")
_DATE_DECL_RE = re.compile(rf"^DATE\s+(?P<name>{_IDENT})\s*=\s*{_DATE_BASE}{_TERMS}$")
_DURATION_DECL_RE = re.compile(
    rf"^DURATION\s+(?P<name>{_IDENT})\s*=\s*(?P<base>\d+|{_IDENT}){_TERMS}"
    r"(?:\s+(?:DAYS|days))?$"
)
_WELL_DECL_RE = re.compile(rf'^WELL\s+(?P<name>{_IDENT})\s*=\s*"(?P<well>[^"]*)"$')
_WELL_BLOCK_RE = re.compile(rf'^WELL\s+(?:"(?P<qname>[^"]*)"|(?P<ref>{_IDENT}))$')
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
    warnings: List[ParseWarning] = []
    errors: List[ParseIssue] = []
    current_well: Optional[WellBlock] = None

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
            current_well = _parse_line(
                line, loc, variables, wells, warnings, current_well, unit_holder
            )
        except OrionParseError as exc:
            errors.extend(exc.errors)
            if line.split(None, 1)[0] == "WELL" or (
                line.startswith("@") and current_well is None
            ):
                # Suppress cascading errors from lines belonging to a broken
                # (or missing) block: swallow them into a discarded block.
                current_well = WellBlock(well_name="", loc=loc)

    if version is None:
        raise OrionParseError("Empty file: missing 'ORIONEVENTS' header")

    if errors:
        raise OrionParseError(errors=errors)

    return OrionDocument(
        version=version,
        unit_system=unit_holder[0],
        variables=variables,
        wells=wells,
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
    warnings: List[ParseWarning],
    current_well: Optional[WellBlock],
    unit_holder: List[str],
) -> Optional[WellBlock]:
    """Dispatch one non-header line; returns the (possibly new) current well."""
    if line.startswith("@"):
        if current_well is None:
            raise OrionParseError("Event line found before any WELL block", loc)
        current_well.events.append(_parse_event_line(line, variables, loc))
        return current_well

    first = line.split(None, 1)[0]

    if first == "UNIT":
        match = _UNIT_RE.match(line)
        if match is None:
            raise OrionParseError(
                f"Malformed UNIT line: {line!r} (expected UNIT METRIC|FIELD|LAB)", loc
            )
        unit_holder[0] = match.group("unit")
        return current_well

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
        return current_well

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
        return current_well

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
            return current_well
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
        return new_well

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
) -> datetime.date:
    """Evaluate an ISO date or DATE variable plus optional signed day terms."""
    if base[0].isdigit():
        try:
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
    return OrionEvent(
        event_type=event_type,
        event_date=event_date,
        attributes=attributes,
        loc=loc,
    )


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
    """Infer int, then float, otherwise keep the raw string."""
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
    warnings: List[str] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)


# Policies for handling unresolved references: skip silently, warn, or raise.
_POLICIES = ("warn", "error", "skip")

# Attributes accepted on a keyword event but intentionally not emitted.
_IGNORED_KEYWORD_ATTRS = {"DSHIFT", "FILTER", "PERFID"}

# PERFORATION attribute handling.
_PERF_REQUIRED = ("MDSTART", "MDEND")
_PERF_KNOWN = {"MDSTART", "MDEND", "RADIUS", "SKIN", "COMPLETION_NUMBER"}
_PERF_IGNORED = {"FILTER", "PERFID"}

# ORIONEVENTS -> Eclipse item-name translations per keyword.
_WCONHIST_FIELD_MAP = {"VFP": "VFP_TABLE"}
_WELTARG_FIELD_MAP = {"VALUE": "NEW_VALUE"}


def apply_orion_events_file(
    path: Union[str, "os.PathLike[str]"],
    timeline: Any,
    project: Any,
    **options: str,
) -> ApplyReport:
    """Parse an ORIONEVENTS file and apply it to ``timeline``."""
    document = parse_orion_events_file(path)
    return apply_orion_document(document, timeline, project, **options)


def apply_orion_document(
    document: OrionDocument,
    timeline: Any,
    project: Any,
    *,
    on_unknown_well: str = "warn",
    on_unknown_event: str = "warn",
) -> ApplyReport:
    """Apply a parsed :class:`OrionDocument` to a ``WellEventTimeline``.

    Arguments:
        document: The parsed document (see :func:`parse_orion_events`).
        timeline: A ``rips.WellEventTimeline`` object.
        project: A ``rips.Project`` used to resolve well names to well paths.
        on_unknown_well: ``"warn"`` (default), ``"error"`` or ``"skip"`` -
            behavior when a well name has no matching well path.
        on_unknown_event: same policy values, for unknown event types.

    Returns:
        ApplyReport: counts plus collected warnings/errors.
    """
    _validate_policy(on_unknown_well, "on_unknown_well")
    _validate_policy(on_unknown_event, "on_unknown_event")
    report = ApplyReport()

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
            dispatch = _EVENT_DISPATCH.get(event.event_type.upper())
            if dispatch is None:
                message = (
                    f"Unknown event type '{event.event_type}' (line {event.loc.line})"
                )
                close = difflib.get_close_matches(
                    event.event_type.upper(), _EVENT_DISPATCH, n=1, cutoff=0.6
                )
                if close:
                    message += f"; did you mean '{close[0]}'?"
                if on_unknown_event == "error":
                    raise RipsError(message)
                if on_unknown_event == "warn":
                    report.warnings.append(message)
                report.events_skipped += 1
                continue
            dispatch(event, well_path, timeline, report)

    return report


def _validate_policy(value: str, name: str) -> None:
    if value not in _POLICIES:
        raise ValueError(f"{name} must be one of {_POLICIES}, got {value!r}")


def _apply_perforation(
    event: OrionEvent, well_path: Any, timeline: Any, report: ApplyReport
) -> None:
    attrs = event.attributes
    unknown = set(attrs) - _PERF_KNOWN - _PERF_IGNORED
    if unknown:
        report.errors.append(
            f"Line {event.loc.line}: unknown PERFORATION attribute(s): "
            f"{', '.join(sorted(unknown))}"
        )
        report.events_skipped += 1
        return

    for key in sorted(_PERF_IGNORED & set(attrs)):
        report.warnings.append(
            f"Line {event.loc.line}: attribute '{key}' on PERFORATION "
            "is ignored (not yet supported)"
        )

    missing = [name for name in _PERF_REQUIRED if name not in attrs]
    if missing:
        report.errors.append(
            f"Line {event.loc.line}: PERFORATION missing required "
            f"attribute(s): {', '.join(missing)}"
        )
        report.events_skipped += 1
        return

    try:
        kwargs: Dict[str, Any] = {
            "event_date": event.event_date.isoformat(),
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

    timeline.add_perf_event(**kwargs)
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
        event_date=event.event_date.isoformat(),
        well_path=well_path,
        keyword_name=keyword_name,
        keyword_data=keyword_data,
    )
    report.events_applied += 1


def _apply_wconhist(
    event: OrionEvent, well_path: Any, timeline: Any, report: ApplyReport
) -> None:
    _apply_keyword(event, well_path, timeline, report, "WCONHIST", _WCONHIST_FIELD_MAP)


def _apply_weltarg(
    event: OrionEvent, well_path: Any, timeline: Any, report: ApplyReport
) -> None:
    _apply_keyword(event, well_path, timeline, report, "WELTARG", _WELTARG_FIELD_MAP)


def _as_number(attr: AttrValue, loc: SourceLoc) -> Union[int, float]:
    if isinstance(attr.value, bool) or not isinstance(attr.value, (int, float)):
        raise OrionParseError(f"Expected a numeric value, got {attr.raw!r}", loc)
    return attr.value


_EventDispatch = Callable[[OrionEvent, Any, Any, ApplyReport], None]

_EVENT_DISPATCH: Dict[str, _EventDispatch] = {
    "PERFORATION": _apply_perforation,
    "WCONHIST": _apply_wconhist,
    "WELTARG": _apply_weltarg,
}


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
    print(
        f"{args.file}: OK (ORIONEVENTS {document.version}, "
        f"units {document.unit_system})"
    )
    print(
        f"  {len(document.variables)} variable(s), {len(document.wells)} "
        f"well block(s), {event_count} event(s)"
    )
    for warning in document.warnings:
        print(f"  Warning line {warning.loc.line}: {warning.message}")
    return 0


if __name__ == "__main__":
    import sys

    sys.exit(_cli())
