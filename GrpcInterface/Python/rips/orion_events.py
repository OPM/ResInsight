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
  ResInsight instance and can be unit-tested standalone.
* **Layer B - applier** (:func:`apply_orion_document` / :func:`apply_orion_events_file`):
  takes the intermediate representation plus a live ``rips`` project/timeline and
  calls the ``WellEventTimeline`` API, performing all semantic mapping and
  validation.

File format grammar (EBNF-ish)::

    document      = header , [ unit_directive ] , { set_def } , { well_block } ;
    header        = "ORIONEVENTS" , version ;          (* e.g. ORIONEVENTS 1.0 *)
    unit_directive= "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;
    set_def       = "SET" , ident , "=" , date_expr ;  (* SET A1 = 2018-01-01 + 9 *)
    well_block    = quoted_name , { event_line } ;     (* '55_33-A-1' *)
    event_line    = "@" , date_expr , event_type , { attribute } ;
    date_expr     = ( iso_date | ident ) , [ "+" , integer ] ;  (* offset in days *)
    attribute     = ident , "=" , ( quoted_string | bareword ) ;
    comment       = "#" , { any_char } ;               (* line or trailing *)

Notes on the grammar:

* Comments start with ``#`` (outside of quotes) and run to end of line.
* Every attribute is ``KEY=VALUE``; bare positional tokens are rejected.
* Date arithmetic is restricted to ``+ <non-negative int>`` (whole days).
* Attribute values may be double-quoted to contain spaces/operators, e.g.
  ``FILTER="SOIL(0) > 0.8 AND PERMX > 200"``.
* ``FILTER`` and ``PERFID`` are reserved but **not yet supported**: encountering
  them raises :class:`OrionParseError` with the source line. Remove them from the
  file until support is added.
* ``DSHIFT`` on keyword events is accepted but ignored (with a warning); it does
  not shift the event date.
"""

from __future__ import annotations

import datetime
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


class OrionParseError(Exception):
    """Raised for structural or unsupported-syntax errors while parsing."""

    def __init__(self, message: str, loc: Optional[SourceLoc] = None) -> None:
        self.loc = loc
        if loc is not None:
            super().__init__(f"Line {loc.line}: {message}")
        else:
            super().__init__(message)


@dataclass(frozen=True)
class ParseWarning:
    """A non-fatal issue encountered while parsing."""

    message: str
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
    """A quoted well name followed by its events."""

    well_name: str
    events: List[OrionEvent] = field(default_factory=list)
    loc: SourceLoc = SourceLoc(0, "")


@dataclass
class OrionDocument:
    """Parsed, lossless representation of an ORIONEVENTS file."""

    version: str
    unit_system: str = "METRIC"
    variables: Dict[str, datetime.date] = field(default_factory=dict)
    wells: List[WellBlock] = field(default_factory=list)
    warnings: List[ParseWarning] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Layer A: pure parser
# ---------------------------------------------------------------------------

# Reserved attributes that are recognized but not yet supported.
_UNSUPPORTED_ATTRIBUTES = {"FILTER", "PERFID"}

_BASE = r"(?P<base>\d{4}-\d{2}-\d{2}|[A-Za-z_]\w*)"
_OFFSET = r"(?:\+\s*(?P<offset>\d+))?"

_HEADER_RE = re.compile(r"^ORIONEVENTS\s+(?P<version>\d+\.\d+)$")
_UNIT_RE = re.compile(r"^UNIT\s+(?P<unit>METRIC|FIELD|LAB)$")
_WELL_RE = re.compile(r"^'(?P<name>[^']*)'$")
_SET_RE = re.compile(rf"^SET\s+(?P<name>[A-Za-z_]\w*)\s*=\s*{_BASE}\s*{_OFFSET}$")
_EVENT_RE = re.compile(rf"^@\s*{_BASE}\s*{_OFFSET}\s+(?P<rest>.+)$")
_ATTR_RE = re.compile(r'(?P<key>[A-Za-z_]\w*)\s*=\s*(?:"(?P<qval>[^"]*)"|(?P<val>\S+))')


def parse_orion_events_file(path: Union[str, "os.PathLike[str]"]) -> OrionDocument:
    """Parse an ORIONEVENTS file from disk into an :class:`OrionDocument`."""
    with open(path, "r", encoding="utf-8") as handle:
        return parse_orion_events(handle.read())


def parse_orion_events(text: str) -> OrionDocument:
    """Parse ORIONEVENTS text into an :class:`OrionDocument`.

    Raises:
        OrionParseError: on structural errors or unsupported syntax.
    """
    version: Optional[str] = None
    unit_system = "METRIC"
    variables: Dict[str, datetime.date] = {}
    wells: List[WellBlock] = []
    warnings: List[ParseWarning] = []
    current_well: Optional[WellBlock] = None

    for lineno, raw_line in enumerate(text.splitlines(), start=1):
        loc = SourceLoc(line=lineno, text=raw_line)
        line = _strip_comment(raw_line).strip()
        if not line:
            continue

        # The first meaningful line must be the header.
        if version is None:
            match = _HEADER_RE.match(line)
            if match is None:
                raise OrionParseError(
                    "File must start with 'ORIONEVENTS <version>'", loc
                )
            version = match.group("version")
            continue

        if line.startswith("@"):
            if current_well is None:
                raise OrionParseError("Event line found before any well block", loc)
            current_well.events.append(_parse_event_line(line, variables, loc))
            continue

        set_match = _SET_RE.match(line)
        if set_match is not None:
            name = set_match.group("name")
            if name in variables:
                warnings.append(ParseWarning(f"Duplicate SET '{name}'", loc))
            variables[name] = _resolve_date(
                set_match.group("base"), set_match.group("offset"), variables, loc
            )
            continue

        unit_match = _UNIT_RE.match(line)
        if unit_match is not None:
            unit_system = unit_match.group("unit")
            continue

        well_match = _WELL_RE.match(line)
        if well_match is not None:
            current_well = WellBlock(well_name=well_match.group("name"), loc=loc)
            wells.append(current_well)
            continue

        raise OrionParseError(f"Unrecognized line: {line!r}", loc)

    if version is None:
        raise OrionParseError("Empty file: missing 'ORIONEVENTS' header")

    return OrionDocument(
        version=version,
        unit_system=unit_system,
        variables=variables,
        wells=wells,
        warnings=warnings,
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


def _resolve_date(
    base: str,
    offset: Optional[str],
    variables: Dict[str, datetime.date],
    loc: SourceLoc,
) -> datetime.date:
    """Resolve a date expression (ISO date or variable) plus optional day offset."""
    if base[0].isalpha() or base[0] == "_":
        if base not in variables:
            raise OrionParseError(f"Unknown date variable '{base}'", loc)
        result = variables[base]
    else:
        try:
            result = datetime.date.fromisoformat(base)
        except ValueError as exc:
            raise OrionParseError(f"Invalid date '{base}': {exc}", loc)
    if offset is not None:
        result = result + datetime.timedelta(days=int(offset))
    return result


def _parse_event_line(
    line: str, variables: Dict[str, datetime.date], loc: SourceLoc
) -> OrionEvent:
    match = _EVENT_RE.match(line)
    if match is None:
        raise OrionParseError(f"Malformed event line: {line!r}", loc)

    event_date = _resolve_date(
        match.group("base"), match.group("offset"), variables, loc
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
        if key in _UNSUPPORTED_ATTRIBUTES:
            raise OrionParseError(f"Attribute '{key}' is not yet supported", loc)

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
_IGNORED_KEYWORD_ATTRS = {"DSHIFT"}

# PERFORATION attribute handling.
_PERF_REQUIRED = ("MDSTART", "MDEND")
_PERF_KNOWN = {"MDSTART", "MDEND", "RADIUS", "SKIN", "COMPLETION_NUMBER"}

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
    unknown = set(attrs) - _PERF_KNOWN
    if unknown:
        report.errors.append(
            f"Line {event.loc.line}: unknown PERFORATION attribute(s): "
            f"{', '.join(sorted(unknown))}"
        )
        report.events_skipped += 1
        return

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
