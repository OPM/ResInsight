# SIMEVENTS File Format — Version 1.1

## Overview

SIMEVENTS is a compact, human-authored text format describing dated well events (completions and Eclipse schedule keywords) over time. ResInsight reads it through the pure-Python parser in `GrpcInterface/Python/rips/simulator_events.py` and applies the events to a project via the `WellEventTimeline` API, from which Eclipse schedule text (COMPDAT, WELSEGS, WCONHIST, ...) can be generated.

The implementation has two independent layers:

| Layer | Entry points | Needs ResInsight? |
|---|---|---|
| A — parser | `rips.simulator_events.parse_simulator_events(text)` / `parse_simulator_events_file(path)` | No |
| B — applier | `rips.simulator_events.apply_simulator_events_document(document, timeline, project)` / `apply_simulator_events_file(path, timeline, project)` | Yes |

A standalone validator runs Layer A only:

```
python3 -m rips.simulator_events myfile.events
```

Runnable examples: `rips/PythonExamples/experimental/import_simulator_events.py` (reads the shipped sample `rips/example_input_files/simulator_events.events`) and `well_event_schedule_simulator_events.py` (full event coverage, generates schedule text).

## Quick example

```
SIMEVENTS 1.1
UNIT METRIC

# Typed declarations
DATE     STARTUP  = 2024-01-01
DATE     PHASE2   = 2024-03-01 + 9d
DURATION RAMP     = 31d
FILTER   POROPERM = "PORO > 0.4 AND PERMX > 100.0"

WELL A1 = "55_33-A-1"            # alias declaration (has '=')

WELL A1                          # opens an event block via the alias
  @STARTUP         SEGMENT      MDSTART=0     MDEND=2500  INNER_DIAMETER=0.15  ROUGHNESS=1.0e-5  PRESSURE_COMPONENTS=HFA
  @STARTUP + RAMP  PERFORATION  MDSTART=2000  MDEND=2200  RADIUS=0.05  SKIN=0.5  COMPLETION_NUMBER=1  FILTER=POROPERM
  @2024-05-15T14:45:30  PERFORATION  MDSTART=2300  MDEND=2350  RADIUS=0.05
  @2024-03-01      VALVE        MD=2100  TYPE=ICV  STATE=OPEN  CV=0.7  AREA=0.0001
  @STARTUP + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1
  @2024-05-01      WELTARG      CMODE=ORAT  VALUE=5000.0
  @2024-06-01      WRFTPLT      OUTPUT_RFT=YES  OUTPUT_PLT=NO  OUTPUT_SEGMENT=NO

WELL "55_33-A-2"                 # quoted form: literal well name
  @PHASE2 - 1d  PERFORATION  MDSTART=1692.79  MDEND=1706  RADIUS=0.12065  SKIN=5

SCHEDULE                         # schedule-level keywords, not tied to a well
  @STARTUP  RPTRST    BASIC=2  FREQ=1
  @STARTUP  GRUPTREE  CHILD=OP  PARENT=FIELD
  @STARTUP  TUNING    TSINIT=1  TSMAXZ=30  NEWTMX=12
```

## Grammar (EBNF-ish)

```
document        = header , { statement } ;
header          = "SIMEVENTS" , "1.1" ;           (* first meaningful line *)
statement       = unit_directive | declaration | report_line | well_block_open
                | group_block_open | schedule_block_open | event_line ;
unit_directive  = "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;
report_line     = "REPORT" , date_expr ;

declaration     = date_decl | duration_decl | well_decl | filter_decl ;
date_decl       = "DATE" , ident , "=" , date_expr ;         (* DATE X = 2018-03-01 + 9d *)
duration_decl   = "DURATION" , ident , "=" , duration_expr ; (* DURATION RAMP = 5d12h *)
well_decl       = "WELL" , ident , "=" , quoted_string ;     (* WELL A1 = "55_33-A-1" *)
filter_decl     = "FILTER" , ident , "=" , '"' , filter_expr , '"' ;
                                        (* FILTER F = "PORO > 0.4 AND PERMX > 100.0" *)
filter_expr     = filter_term , { ( "AND" | "OR" ) , filter_term } ;
                                        (* one combine mode; mixing AND and OR is an error *)
filter_term     = [ result_type , "." ] , ident , comp_op , number ;
comp_op         = ">" | ">=" | "<" | "<=" ;

well_block_open     = "WELL" , ( quoted_string | ident ) ;   (* no "=" present *)
group_block_open    = "GROUP" , quoted_string ;      (* group keyword events *)
schedule_block_open = "SCHEDULE" ;                  (* well-less keyword events *)
event_line          = "@" , date_expr , event_type , { attribute } ;
event_type          = completion_event_type | eclipse_keyword ;
completion_event_type = "PERFORATION" | "SEGMENT" | "VALVE" | "STATE" | "WELSPECS" ;
segment_attribute   = "MDSTART" | "MDEND" | "INNER_DIAMETER" | "ROUGHNESS"
                    | "PRESSURE_COMPONENTS" | "COMMENT" ;
                    (* accepted attribute names when event_type is "SEGMENT" *)

date_expr       = ( iso_datetime | date_ident ) , { sign , term } ;
duration_expr   = term , { sign , term } ;
term            = duration_lit | dur_ident ;        (* e.g. 2d, -12h30m, RAMP *)
sign            = "+" | "-" ;
duration_lit    = [ sign ] , component , { component } ;
                                        (* units strictly descending, each at most once *)
component       = number , ( "mon" | "d" | "h" | "m" | "s" ) ;
number          = digits | digits , "." , digits ;
                                        (* a fraction only on the last component,
                                           never on "mon" or "s" *)
iso_datetime    = 4digit , "-" , 2digit , "-" , 2digit ,
                  [ "T" , 2digit , ":" , 2digit , ":" , 2digit , [ "." , digits ] ] ;
                                        (* 2024-05-15 or 2024-05-15T14:45:30 *)
ident           = letter_or_underscore , { word_char } ;
attribute       = ident , "=" , ( quoted_string | bareword ) ;
comment         = "#" , rest-of-line ;              (* line or trailing *)
```

## Line types

The format is line-oriented. Every non-blank line is dispatched on its first token; anything else is an error. Keywords are uppercase and case-sensitive; duration units (`mon`, `d`, `h`, `m`, `s`) are lowercase.

| First token | Meaning |
|---|---|
| `SIMEVENTS` | Header with version; must be the first meaningful line, exactly once |
| `UNIT` | Unit system: `METRIC`, `FIELD` or `LAB` (default `METRIC`) |
| `DATE` | Declare a typed date variable |
| `DURATION` | Declare a typed duration variable (Go-style literal, e.g. `5d`, `12h30m`) |
| `FILTER` | Declare a typed cell-filter expression |
| `REPORT` | Add a date that must appear in generated schedule output |
| `WELL` | With `=`: declare a well-name alias. Without `=`: open a well event block |
| `GROUP` | Open a block of group-level keyword events |
| `SCHEDULE` | Open a block of schedule-level keyword events (bare keyword, no arguments) |
| `@` | An event line, appended to the enclosing WELL, GROUP or SCHEDULE block |
| `#` | Comment (also allowed trailing on any line, outside double quotes) |

## Typed declarations

Variables are typed — `DATE`, `DURATION`, `WELL` (well-name alias) and `FILTER` (cell filter expression) — and share one namespace. They must be declared before use (single-pass, no forward references).

```
DATE     STARTUP  = 2024-01-01
DURATION RAMP     = 31d                  # Go-style duration literal
DATE     PHASE2   = STARTUP + RAMP       # date arithmetic with variables
WELL     A1       = "55_33-A-1"
FILTER   POROPERM = "PORO > 0.4 AND PERMX > 100.0"
```

Using a variable of the wrong type is an error citing both the use and the declaration site:

```
Line 18: Variable 'RAMP' is a DURATION (declared line 10) but a DATE is required here
```

Redeclaring a name with the same type warns and the last value wins; redeclaring with a different type is an error.

## Date expressions

A date expression is an ISO datetime or a `DATE` variable, followed by a chain of signed terms. Each term is a [duration literal](#duration-literals) or a `DURATION` variable:

```
@2024-01-01
@STARTUP + 5d
@STARTUP + RAMP - 2d
@STARTUP + 12h30m
@2024-05-15T14:45:30       # time-of-day, second precision
@2024-01-31 + 1mon         # calendar month: 2024-02-29
```

Every date is a datetime with **second** resolution. A date-only literal (`2024-01-01`) means midnight; a time-of-day is written with the `T` separator (`YYYY-MM-DDTHH:MM:SS`). Fractional seconds are accepted and rounded to the nearest second. An operand may carry its own sign (`START + -2d` equals `START - 2d`); whitespace around `+`/`-` is optional but conventional. A non-midnight time-of-day is preserved through the applier and emitted as the optional TIME field of the generated DATES keyword.

## Duration literals

A duration literal is written Go-style: one or more `<number><unit>` components with no spaces, using the units below in strictly descending order, each at most once.

| Unit | Meaning |
|---|---|
| `mon` | calendar month |
| `d` | day (24 h) |
| `h` | hour |
| `m` | minute |
| `s` | second |

```
DURATION RAMP    = 5d
DURATION SHIFT   = 12h30m
DURATION QUARTER = 3mon
DURATION BACK    = -3d
DURATION HALF    = 1.5h                  # fraction allowed on the last component only
DURATION TOTAL   = RAMP + SHIFT - 6h     # duration arithmetic with variables
```

Rules:

* A unit is mandatory: `5` and the pre-1.1 `5 DAYS` are errors (write `5d`).
* Units must be in the order `mon`, `d`, `h`, `m`, `s` and may not repeat: `12h30m` is valid, `30m12h` and `1h1h` are not.
* A fraction is allowed only on the **last** component, and never on `mon` or `s`: `1.5h` and `1d1.5h` are valid, `1.5h30m`, `1.5s` and `1.5mon` are not. The result is rounded to whole seconds.
* `mon` is calendar-relative and is applied before the fixed part of a duration. Adding months clamps to the last day of a shorter month: `2024-01-31 + 1mon` is `2024-02-29` and `2024-01-31 + 1mon1d` is `2024-03-01`.
* A leading `-` negates the whole literal (`-1mon12h` is minus one month and minus twelve hours).

In Python a `DURATION` value is a `rips.simulator_events.Duration` with `months` and `delta` (`datetime.timedelta`) fields; `str()` renders the Go-style form, e.g. `1mon5d12h30m`.

## WELL blocks

`WELL <ident>` opens an event block for a declared `WELL` alias; `WELL "<name>"` opens a block for the literal well name and never consults variables — so there is no shadowing ambiguity. Well names containing special characters must use the quoted form. Empty blocks are legal. When applied, the well name must match a well path in the project (see the `on_unknown_well` policy below).

## Event types

Event lines are `@<date_expr> <EVENT_TYPE> KEY=VALUE ...`. Inside a WELL block, the event type is either one of the five built-in completion events or any Eclipse well keyword.

### Built-in completion events

**PERFORATION** → `add_perf_event` (state is always OPEN)

| Attribute | Required | Maps to |
|---|---|---|
| `MDSTART` | yes | `start_md` |
| `MDEND` | yes | `end_md` |
| `RADIUS` | no | `diameter` = 2 × radius |
| `SKIN` | no | `skin_factor` |
| `COMPLETION_NUMBER` | no | `completion_number` (COMPLUMP grouping) |
| `FILTER` | no | case-level combined data filter attached to the perforation: a declared `FILTER` variable (`FILTER=POROPERM`) or an inline quoted expression (`FILTER="PORO > 0.4"`) — see [Filter expressions](#filter-expressions) |

**SEGMENT** → `add_tubing_event` and `add_segment_interval`

Each event adds the underlying tubing timeline event and a corresponding **Segment Interval** with the same measured-depth range, diameter, and roughness.

| Attribute | Required | Maps to |
|---|---|---|
| `MDSTART` | yes | tubing and segment interval `start_md` |
| `MDEND` | yes | tubing and segment interval `end_md` |
| `INNER_DIAMETER` | no | tubing `inner_diameter` [m] |
| `ROUGHNESS` | no | tubing `roughness` [m] |
| `PRESSURE_COMPONENTS` | no | MSW `pressure_drop`: `H--`, `HF-` or `HFA` |

**VALVE** → `add_valve_event` (requires an existing perforation at the MD)

| Attribute | Required | Maps to |
|---|---|---|
| `MD` | yes | `measured_depth` |
| `TYPE` | yes | `valve_type`: `ICV`, `ICD` or `AICD` |
| `STATE` | no | `state`: `OPEN` or `SHUT` |
| `CV` | no | `flow_coefficient` |
| `AREA` | no | `area` [m²] |
| `AICD_STRENGTH`, `AICD_DENSITY_CALIB_FLUID`, `AICD_VISCOSITY_CALIB_FLUID`, `AICD_VOL_FLOW_EXP`, `AICD_VISC_FUNC_EXP` | no | the corresponding AICD parameters |

**STATE** → `add_state_event`

| Attribute | Required | Maps to |
|---|---|---|
| `STATE` | yes | `well_state`: `OPEN`, `SHUT` or `STOP` |

Unknown attributes on a completion event are an error and skip the event. On completion events other than `PERFORATION`, `FILTER` is ignored with a warning.

### Well keyword events

Any other event type in a WELL block is passed through as an Eclipse well keyword via `add_well_keyword_event`, with the well name injected as the `WELL` item and all attributes forwarded (type-inferred as int, float or string). Two keywords have convenience attribute translations:

| Event type | Translation |
|---|---|
| `WCONHIST` | `VFP` → `VFP_TABLE` |
| `WELTARG` | `VALUE` → `NEW_VALUE` |

All other keywords (`WRFTPLT`, `WCONPROD`, `WELOPEN`, ...) forward attributes unchanged, so attribute keys must match the Eclipse item names ResInsight uses for that keyword. `FILTER` is ignored with a warning.

An event type that closely resembles a misspelled built-in (e.g. `PERFORATIN`) is **not** passed through as a keyword; it is handled by the `on_unknown_event` policy with a "did you mean" hint.

### SCHEDULE blocks

A bare `SCHEDULE` line opens a block of schedule-level keyword events applied through `add_keyword_event` — no well association, no `WELL` item injected:

```
SCHEDULE
  @STARTUP  RPTRST    BASIC=2  FREQ=1
  @STARTUP  GRUPTREE  CHILD=OP  PARENT=FIELD
  @STARTUP  TUNING    TSINIT=1  TSMAXZ=30  NEWTMX=12
```

Completion event types (`PERFORATION`, `SEGMENT`, `VALVE`, `STATE`, `WELSPECS`) are rejected in a SCHEDULE block. A later `WELL` line switches back to well events; blocks can be interleaved freely.

## Attributes and quoting

Every attribute is `KEY=VALUE`; bare positional tokens are rejected. Values are type-inferred (int, then float, otherwise string). Double quotes are used everywhere — well names, filter expressions and attribute values — and allow spaces and operators inside a value:

```
FILTER="SOIL > 0.8 AND PERMX > 200"
```

## Filter expressions

A filter expression — the quoted value of a `FILTER` declaration or of an inline `FILTER="..."` attribute — is a list of comparison terms joined by `AND` or `OR` (uppercase, one mode per expression; mixing them is an error, since a combined filter has a single combine mode):

```
FILTER POROPERM = "PORO > 0.4 AND PERMX > 100.0"
FILTER LOWSAT   = "dynamic.SOIL < 0.2 OR PRESSURE < 150"
```

Each term is `[TYPE.]NAME <op> NUMBER`:

* Operators are `>`, `>=`, `<` and `<=`. A term sets only one bound of the generated property filter; the other bound keeps the result's minimum/maximum. Bounds are inclusive, so `>` behaves as `>=` (and `<` as `<=`).
* The result name is case-insensitive (uppercased on parse). An unqualified name is searched in the case's `STATIC_NATIVE`, then `DYNAMIC_NATIVE`, then `GENERATED` results; the first type containing it wins.
* A `TYPE.` qualifier restricts the search to one result type: `STATIC`/`STATIC_NATIVE`, `DYNAMIC`/`DYNAMIC_NATIVE` or `GENERATED`, case-insensitive (`DYNAMIC_NATIVE.MY_PROPERTY` and `dynamic.my_property` are equivalent).
* A result that does not exist in the case raises an error. All filter terms are checked up front, before any event is applied.

When applied, each used filter becomes a case-level **combined data filter** (under the case's *Data Filters*) holding one property filter per term. A declared filter is created once, named after its declaration, and shared between every perforation that references it; an inline filter gets an auto-derived name from its content (identical inline expressions share one filter). The filter is attached to the perforation event and carried onto the perforation interval when completions are materialized with `set_timestamp`. Declared-but-unreferenced filters create nothing.

## Diagnostics

The parser recovers per line and reports **all** errors in one pass. The raised `SimulatorEventsParseError` carries one `ParseIssue` (message + source line) per problem, and unknown names get difflib-based "did you mean" suggestions:

```
Line 3:  SET is not supported; declare a typed variable instead, e.g. DATE NAME = 2018-01-01
Line 5:  Malformed WELL line: 'WELL 55_33-A-2' (well names containing special characters must be double-quoted, e.g. WELL "55_33-A-1")
Line 6:  Unknown variable 'A1_STRTUP'; did you mean 'A1_STARTUP'?
Line 7:  Variable 'RAMP' is a DURATION (declared line 3) but a DATE is required here
```

The validator CLI prints these and exits non-zero:

```
$ python3 -m rips.simulator_events simulator_events.events
simulator_events.events: OK (SIMEVENTS 1.1, units METRIC)
  4 variable(s), 2 well block(s), 8 well event(s), 0 schedule event(s)
```

## Applying to a project

```python
import rips

resinsight = rips.Instance.find()
project = resinsight.project

well_path_coll = project.descendants(rips.WellPathCollection)[0]
timeline = well_path_coll.event_timeline()

report = rips.simulator_events.apply_simulator_events_file(
    "simulator_events.events", timeline, project, case=case, on_unknown_well="warn"
)
print(report.events_applied, report.events_skipped, report.warnings, report.errors)

timeline.set_timestamp(timestamp="2024-12-24")  # materialize completions up to a date
schedule_text = timeline.generate_schedule_text(
    eclipse_case=case, export_msw_for_wells=[well_path]
)
```

`apply_simulator_events_document` / `apply_simulator_events_file` accept an optional `case` — the `rips.Case` used to resolve filter result names and to own the created combined filters. It is only needed when the document uses `FILTER` and defaults to the project's first case; a `RipsError` is raised if a filter is used and no case is available, or if a filter references a result missing from the case (checked before any event is applied).

They also accept two policies, each `"warn"` (default), `"error"` or `"skip"`:

| Policy | Governs |
|---|---|
| `on_unknown_well` | A well name with no matching well path in the project |
| `on_unknown_event` | An event type that looks like a misspelled built-in |

The returned `ApplyReport` carries `events_applied`, `events_skipped`, `warnings` and `errors`.

## Exporting a schedule from the GUI

The schedule generated from the well event timeline can be written to file without scripting.
Enable *Preferences -> System -> Experimental Features -> Simulator Events Schedule Export*, then
use **Export Schedule from Events** from either:

- the context menu of the *Well Event Timeline* item (shown under *Wells* once the timeline has
  events),
- the context menu of the *Wells* (well path collection) item, or
- *File -> Export*.

The only thing asked for is the file name (default `schedule.SCH` in the last used export folder).
Everything else uses fixed defaults:

| Setting | Value |
|---|---|
| Eclipse case | the case of the active 3D view, else the first Eclipse case of the project |
| Multi-segment-well keywords | exported for every well with events |
| First date | written as a `-- Date: ...` comment instead of a `DATES` keyword |
| Formatting | aligned columns with a `--`-prefixed column header per keyword |

As in `generate_schedule_text()`, a `set_timestamp()`/*Apply events up to date* timestamp limits the
export to the events up to that date, while inserted dates (`INSERT_DATE`) are always emitted.
The written file name and the event count are logged to the message panel; an export without events
or without a loaded Eclipse case reports the reason in a dialog.

## Version history

- **1.1** — current: all dates are datetimes with second resolution (`T` separator required for a time-of-day; fractional seconds rounded); `DURATION` values and offset terms are Go-style literals (`5d`, `12h30m`, `1.5h`, `3mon`, `-3d`) with mandatory units, strict descending unit order, and calendar-aware months. Unitless numbers and the `DAYS` suffix are rejected; `1.0` files are rejected with a migration hint.
- **1.0** — typed `DATE`/`DURATION`/`WELL`/`FILTER` declarations, `WELL`/`SCHEDULE` blocks, signed day-offset chains, ISO datetimes, built-in completion events (including `SEGMENT` custom intervals and pressure components) plus generic keyword pass-through, perforation data filters, multi-error diagnostics, validator CLI.
