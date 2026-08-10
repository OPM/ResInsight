# ORIONEVENTS File Format — Version 2.0

## Overview

ORIONEVENTS is a compact, human-authored text format describing dated well events (completions and Eclipse schedule keywords) over time. ResInsight reads it through the pure-Python parser in `GrpcInterface/Python/rips/orion_events.py` and applies the events to a project via the `WellEventTimeline` API, from which Eclipse schedule text (COMPDAT, WELSEGS, WCONHIST, ...) can be generated.

The implementation has two independent layers:

| Layer | Entry points | Needs ResInsight? |
|---|---|---|
| A — parser | `rips.orion_events.parse_orion_events(text)` / `parse_orion_events_file(path)` | No |
| B — applier | `rips.orion_events.apply_orion_document(document, timeline, project)` / `apply_orion_events_file(path, timeline, project)` | Yes |

A standalone validator runs Layer A only:

```
python3 -m rips.orion_events myfile.orion
```

Runnable examples: `rips/PythonExamples/experimental/import_orion_events.py` (reads the shipped sample `rips/example_input_files/well_events.orion`) and `well_event_schedule_orion.py` (full event coverage, generates schedule text).

## Quick example

```
ORIONEVENTS 2.0
UNIT METRIC

# Typed declarations
DATE     STARTUP  = 2024-01-01
DATE     PHASE2   = 2024-03-01 + 9
DURATION RAMP     = 31 DAYS
FILTER   POROPERM = "PORO > 0.4 AND PERMX > 100.0"

WELL A1 = "55_33-A-1"            # alias declaration (has '=')

WELL A1                          # opens an event block via the alias
  @STARTUP         TUBING       MDSTART=0     MDEND=2500  INNER_DIAMETER=0.15  ROUGHNESS=1.0e-5
  @STARTUP + RAMP  PERFORATION  MDSTART=2000  MDEND=2200  RADIUS=0.05  SKIN=0.5  COMPLETION_NUMBER=1  FILTER=POROPERM
  @2024-05-15T14:45:30.500  PERFORATION  MDSTART=2300  MDEND=2350  RADIUS=0.05
  @2024-03-01      VALVE        MD=2100  TYPE=ICV  STATE=OPEN  CV=0.7  AREA=0.0001
  @STARTUP + RAMP  WCONHIST     STATUS=OPEN  CMODE=ORAT  VFP=1
  @2024-05-01      WELTARG      CMODE=ORAT  VALUE=5000.0
  @2024-06-01      WRFTPLT      OUTPUT_RFT=YES  OUTPUT_PLT=NO  OUTPUT_SEGMENT=NO

WELL "55_33-A-2"                 # quoted form: literal well name
  @PHASE2 - 1  PERFORATION  MDSTART=1692.79  MDEND=1706  RADIUS=0.12065  SKIN=5

SCHEDULE                         # schedule-level keywords, not tied to a well
  @STARTUP  RPTRST    BASIC=2  FREQ=1
  @STARTUP  GRUPTREE  CHILD=OP  PARENT=FIELD
  @STARTUP  TUNING    TSINIT=1  TSMAXZ=30  NEWTMX=12
```

## Grammar (EBNF-ish)

```
document        = header , { statement } ;
header          = "ORIONEVENTS" , "2.0" ;           (* first meaningful line *)
statement       = unit_directive | declaration | well_block_open
                | schedule_block_open | event_line ;
unit_directive  = "UNIT" , ( "METRIC" | "FIELD" | "LAB" ) ;

declaration     = date_decl | duration_decl | well_decl | filter_decl ;
date_decl       = "DATE" , ident , "=" , date_expr ;         (* DATE X = 2018-03-01 + 9 *)
duration_decl   = "DURATION" , ident , "=" , duration_expr ; (* DURATION RAMP = 5 DAYS *)
well_decl       = "WELL" , ident , "=" , quoted_string ;     (* WELL A1 = "55_33-A-1" *)
filter_decl     = "FILTER" , ident , "=" , '"' , filter_expr , '"' ;
                                        (* FILTER F = "PORO > 0.4 AND PERMX > 100.0" *)
filter_expr     = filter_term , { ( "AND" | "OR" ) , filter_term } ;
                                        (* one combine mode; mixing AND and OR is an error *)
filter_term     = [ result_type , "." ] , ident , comp_op , number ;
comp_op         = ">" | ">=" | "<" | "<=" ;

well_block_open     = "WELL" , ( quoted_string | ident ) ;   (* no "=" present *)
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
```

## Line types

The format is line-oriented. Every non-blank line is dispatched on its first token; anything else is an error. Keywords are uppercase and case-sensitive (the `DAYS` suffix is also accepted as `days`).

| First token | Meaning |
|---|---|
| `ORIONEVENTS` | Header with version; must be the first meaningful line, exactly once |
| `UNIT` | Unit system: `METRIC`, `FIELD` or `LAB` (default `METRIC`) |
| `DATE` | Declare a typed date variable |
| `DURATION` | Declare a typed whole-day duration variable |
| `FILTER` | Declare a typed cell-filter expression |
| `WELL` | With `=`: declare a well-name alias. Without `=`: open a well event block |
| `SCHEDULE` | Open a block of schedule-level keyword events (bare keyword, no arguments) |
| `@` | An event line, appended to the enclosing WELL or SCHEDULE block |
| `#` | Comment (also allowed trailing on any line, outside double quotes) |

## Typed declarations

Variables are typed — `DATE`, `DURATION` (whole days), `WELL` (well-name alias) and `FILTER` (cell filter expression) — and share one namespace. They must be declared before use (single-pass, no forward references).

```
DATE     STARTUP  = 2024-01-01
DURATION RAMP     = 31 DAYS              # DAYS suffix optional
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

A date expression is an ISO date, an ISO datetime, or a `DATE` variable, followed by a chain of signed whole-day terms. Each term is a non-negative integer or a `DURATION` variable:

```
@2024-01-01
@STARTUP + 5
@STARTUP + RAMP - 2
@2024-05-15T14:45:30.500       # time-of-day, millisecond precision
```

Whitespace around `+`/`-` is optional but conventional. A time-of-day is preserved through the applier and emitted as the optional TIME field of the generated DATES keyword.

## WELL blocks

`WELL <ident>` opens an event block for a declared `WELL` alias; `WELL "<name>"` opens a block for the literal well name and never consults variables — so there is no shadowing ambiguity. Well names containing special characters must use the quoted form. Empty blocks are legal. When applied, the well name must match a well path in the project (see the `on_unknown_well` policy below).

## Event types

Event lines are `@<date_expr> <EVENT_TYPE> KEY=VALUE ...`. Inside a WELL block, the event type is either one of the four built-in completion events or any Eclipse well keyword.

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

**TUBING** → `add_tubing_event`

| Attribute | Required | Maps to |
|---|---|---|
| `MDSTART` | yes | `start_md` |
| `MDEND` | yes | `end_md` |
| `INNER_DIAMETER` | no | `inner_diameter` [m] |
| `ROUGHNESS` | no | `roughness` [m] |

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

Unknown attributes on a completion event are an error and skip the event. `PERFID` is accepted but ignored with a warning (reserved for future use); on completion events other than `PERFORATION`, `FILTER` is likewise ignored with a warning.

### Well keyword events

Any other event type in a WELL block is passed through as an Eclipse well keyword via `add_well_keyword_event`, with the well name injected as the `WELL` item and all attributes forwarded (type-inferred as int, float or string). Two keywords have convenience attribute translations:

| Event type | Translation |
|---|---|
| `WCONHIST` | `VFP` → `VFP_TABLE` |
| `WELTARG` | `VALUE` → `NEW_VALUE` |

All other keywords (`WRFTPLT`, `WCONPROD`, `WELOPEN`, ...) forward attributes unchanged, so attribute keys must match the Eclipse item names ResInsight uses for that keyword. `DSHIFT`, `FILTER` and `PERFID` are ignored with a warning.

An event type that closely resembles a misspelled built-in (e.g. `PERFORATIN`) is **not** passed through as a keyword; it is handled by the `on_unknown_event` policy with a "did you mean" hint.

### SCHEDULE blocks

A bare `SCHEDULE` line opens a block of schedule-level keyword events applied through `add_keyword_event` — no well association, no `WELL` item injected:

```
SCHEDULE
  @STARTUP  RPTRST    BASIC=2  FREQ=1
  @STARTUP  GRUPTREE  CHILD=OP  PARENT=FIELD
  @STARTUP  TUNING    TSINIT=1  TSMAXZ=30  NEWTMX=12
```

Completion event types (`PERFORATION`, `TUBING`, `VALVE`, `STATE`) are rejected in a SCHEDULE block. A later `WELL` line switches back to well events; blocks can be interleaved freely.

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

The parser recovers per line and reports **all** errors in one pass. The raised `OrionParseError` carries one `ParseIssue` (message + source line) per problem, and unknown names get difflib-based "did you mean" suggestions:

```
Line 3:  SET is ORIONEVENTS 1.x syntax; declare a typed variable instead, e.g. DATE NAME = 2018-01-01
Line 5:  Malformed WELL line: 'WELL 55_33-A-2' (well names containing special characters must be double-quoted, e.g. WELL "55_33-A-1")
Line 6:  Unknown variable 'A1_STRTUP'; did you mean 'A1_STARTUP'?
Line 7:  Variable 'RAMP' is a DURATION (declared line 3) but a DATE is required here
```

The validator CLI prints these and exits non-zero:

```
$ python3 -m rips.orion_events well_events.orion
well_events.orion: OK (ORIONEVENTS 2.0, units METRIC)
  4 variable(s), 2 well block(s), 8 well event(s), 0 schedule event(s)
```

## Applying to a project

```python
import rips

resinsight = rips.Instance.find()
project = resinsight.project

well_path_coll = project.descendants(rips.WellPathCollection)[0]
timeline = well_path_coll.event_timeline()

report = rips.orion_events.apply_orion_events_file(
    "well_events.orion", timeline, project, case=case, on_unknown_well="warn"
)
print(report.events_applied, report.events_skipped, report.warnings, report.errors)

timeline.set_timestamp(timestamp="2024-12-24")  # materialize completions up to a date
schedule_text = timeline.generate_schedule_text(
    eclipse_case=case, export_msw_for_wells=[well_path]
)
```

`apply_orion_document` / `apply_orion_events_file` accept an optional `case` — the `rips.Case` used to resolve filter result names and to own the created combined filters. It is only needed when the document uses `FILTER` and defaults to the project's first case; a `RipsError` is raised if a filter is used and no case is available, or if a filter references a result missing from the case (checked before any event is applied).

They also accept two policies, each `"warn"` (default), `"error"` or `"skip"`:

| Policy | Governs |
|---|---|
| `on_unknown_well` | A well name with no matching well path in the project |
| `on_unknown_event` | An event type that looks like a misspelled built-in |

The returned `ApplyReport` carries `events_applied`, `events_skipped`, `warnings` and `errors`.

## Version history

- **2.0** — current: typed `DATE`/`DURATION`/`WELL`/`FILTER` declarations, `WELL`/`SCHEDULE` blocks, signed day-offset chains, ISO datetimes, built-in completion events plus generic keyword pass-through, perforation data filters, multi-error diagnostics, validator CLI.
- **1.x** — unsupported. Files using `SET` variables and single-quoted well names are rejected with a migration message pointing at this grammar.
