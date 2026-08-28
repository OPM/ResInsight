#!/usr/bin/env python3

"""
Example: the well_event_schedule.py timeline expressed as an ORIONEVENTS file.

This is the ORIONEVENTS counterpart to well_event_schedule.py: instead of
calling the WellEventTimeline API methods one by one, the same events are
written as ORIONEVENTS 2.0 text (see rips/orion_events.py for the grammar) and
applied in one go with rips.orion_events.apply_orion_document().

It demonstrates the full event coverage of the format:
1. SEGMENT, PERFORATION (incl. a time-of-day date), VALVE and STATE completion
   events on a well
2. Partial WELSPECS updates that cumulatively change completion export settings
   and generate dated WELSPECS records
3. A FILTER declaration (qualified result name) referenced by a perforation,
   materialized as a case-level combined data filter
4. COMMENT attributes preserved on timeline events and emitted before their
   generated schedule keywords
5. Same-owner/type/date WCONHIST lines merged with conflict diagnostics, and
   historical keyword values carried forward to later partial events, while
   same-date perforations remain separate
6. Well keyword events: WCONHIST and WELTARG (with attribute translation) and
   WRFTPLT (generic Eclipse well keyword pass-through)
7. A GROUP-level MEMBER event expanded to one GRUPTREE record per member
8. SCHEDULE-level keyword events not tied to a well: RPTRST, GRUPTREE, TUNING
9. Multiline RAW_TEXT inserted at a chosen position without parsing its contents
10. Recurring REPORT dates with explicit and implicit end dates, passed to
    generate_schedule_text(additional_dates=...) as summary-report triggers
11. Schedule metadata, COMPORD generation and aligned-column output

The ORIONEVENTS text is built inline with the name of the first well path in
the project (like well_event_schedule.py, which uses wells[0]), so the example
works with any project that has at least one well path. Applying a FILTER needs
a loaded Eclipse case (to resolve the result name and own the created filter),
so the FILTER parts are included only when the project has a case.
"""

import rips
import rips.orion_events


def build_orion_text(well_name, with_filter):
    # 'static.PORO' restricts the result lookup to STATIC_NATIVE results; an
    # unqualified name would search STATIC_NATIVE, DYNAMIC_NATIVE, GENERATED.
    filter_decl = 'FILTER   HIPORO  = "static.PORO > 0.15"\n' if with_filter else ""
    filter_comment = (
        "\n  # The first one is restricted to cells passing the HIPORO filter."
        if with_filter
        else ""
    )
    filter_ref = "  FILTER=HIPORO" if with_filter else ""
    return f"""\
ORIONEVENTS 2.0
UNIT METRIC

# Typed declarations
DATE     STARTUP = 2024-01-01
DURATION RAMP    = 31 DAYS
{filter_decl}
WELL W1 = "{well_name}"

WELL W1
  # WELSPECS updates completion export settings and emits WELSPECS. Attributes
  # are optional: the second event inherits GROUP from the first event.
  2024-01-05      WELSPECS     GROUP="ORION_GROUP"  CROSSFLOW=True   REFDEPTH=1002  PHASE=WATER
  2024-04-15      WELSPECS                          CROSSFLOW=False  REFDEPTH=1000  PHASE=OIL

  # COMMENT is stored on the event and safely emitted as a schedule comment.
  STARTUP         SEGMENT      MDSTART=0        MDEND=2500  INNER_DIAMETER=0.15  ROUGHNESS=1.0e-5  PRESSURE_COMPONENTS=HFA  COMMENT="Install production segment"

  # Perforations; COMPLETION_NUMBER groups connections for COMPLUMP. Same-date
  # perforations are kept as separate events during normalization.{filter_comment}
  STARTUP + RAMP  PERFORATION  MDSTART=2000  MDEND=2200  RADIUS=0.05  SKIN=0.5  COMPLETION_NUMBER=1{filter_ref}  COMMENT="Open high-priority interval"
  STARTUP + RAMP  PERFORATION  MDSTART=2400  MDEND=2600  RADIUS=0.05  SKIN=0.3  COMPLETION_NUMBER=2

  # Time-of-day is preserved and emitted as the TIME field of DATES
  2024-05-15T14:45:30.500  PERFORATION  MDSTART=2300  MDEND=2350  RADIUS=0.05  SKIN=0.4  COMPLETION_NUMBER=3

  # Valve in the first perforation; state event for documentation
  2024-03-01      VALVE        MD=2100  TYPE=ICV  STATE=OPEN  CV=0.7  AREA=0.0001
  2024-02-15      STATE        STATE=OPEN

  # Matching owner/type/date lines merge. The second line extends the first.
  # Conflicting GRAT values produce a warning, and the later value wins.
  2024-01-15      WCONHIST     STATUS=OPEN  CMODE=RESV  GRAT=4756545.5  COMMENT="Start production history controls"
  2024-01-15      WCONHIST     ORAT=3999.99  WRAT=0.01  GRAT=550678.44  VFP=1

  # Later partial keyword events inherit historical values for the same well
  # and keyword. This event overrides WRAT and inherits STATUS, CMODE, ORAT,
  # GRAT and VFP from January 15; COMMENT is event-local and is not inherited.
  2024-01-20      WCONHIST     WRAT=0.03

  # WRFTPLT is passed through as a generic Eclipse keyword.
  2024-05-01      WELTARG      CMODE=ORAT  VALUE=5000.0
  2024-06-01      WRFTPLT      OUTPUT_RFT=YES  OUTPUT_PLT=NO  OUTPUT_SEGMENT=NO

# MEMBER expands into one GRUPTREE record per unique comma-delimited member.
GROUP "OP"
  STARTUP  MEMBER  MEMBERS="{well_name},OBSERVER"  COMMENT="Define operating group members"

# Schedule-level keywords (not tied to a well)
SCHEDULE
  STARTUP  RPTRST    BASIC=2  FREQ=1
  STARTUP  GRUPTREE  CHILD_GROUP=OP  PARENT_GROUP=FIELD
  STARTUP  TUNING    TSINIT=1  TSMAXZ=30  TMAXWC=1  NEWTMX=12  NEWTMN=1  LITMAX=50  LITMIN=1  MXWSIT=50  MXWPIT=50

  # RAW_TEXT preserves its body verbatim. This block is emitted after RPTRST;
  # PRIORITY orders multiple raw blocks sharing the same placement and anchor.
  STARTUP  RAW_TEXT  PLACEMENT=AFTER_KEYWORD  ANCHOR=RPTRST  PRIORITY=10
-- Custom schedule text not modeled by the timeline API
WTRACER
  '{well_name}'  'ORION_TRACER'  1.0 /
/
END_RAW_TEXT

# Recurring report dates become bare DATES keywords. The first series ends at
# the last  event; the second uses an explicit inclusive end date.
REPORT STARTUP EVERY MONTH
REPORT 2024-07-01 EVERY 3 MONTHS UNTIL STARTUP + 365
"""


def main():
    resinsight = rips.Instance.find()
    project = resinsight.project

    print("Well Event Schedule (ORIONEVENTS) Example")
    print("=" * 50)

    print("\n1. Finding well")
    wells = project.well_paths()
    if not wells:
        print("   No well paths in project - load a project with wells first.")
        return
    well_path = wells[0]
    print("   Well name:", well_path.name)

    # A FILTER needs a case; without one, build the text without the filter.
    cases = project.cases()
    case = cases[0] if cases else None
    if case is None:
        print("   No Eclipse case loaded - FILTER parts are left out.")

    print("\n2. Parsing ORIONEVENTS text...")
    orion_text = build_orion_text(well_path.name, with_filter=case is not None)
    print(orion_text)
    document = rips.orion_events.parse_orion_events(orion_text)
    print(f"   Wells: {[w.well_name for w in document.wells]}")
    source_well_event_count = sum(len(w.events) for w in document.wells)
    source_perforation_count = sum(
        event.event_type == "PERFORATION"
        for well in document.wells
        for event in well.events
    )
    normalized = rips.orion_events.coalesce_orion_document(document)
    merged_well_event_count = sum(len(w.events) for w in normalized.wells)
    merged_perforation_count = sum(
        event.event_type == "PERFORATION"
        for well in normalized.wells
        for event in well.events
    )
    print(f"   Source well-event lines: {source_well_event_count}")
    print(f"   Events after same-date merge: {merged_well_event_count}")
    print(
        "   Perforations retained during merge: "
        f"{source_perforation_count} -> {merged_perforation_count}"
    )
    print(f"   Groups: {[group.group_name for group in document.groups]}")
    print(f"   Schedule events: {len(document.schedule_events)}")

    print("\n3. Applying events to the timeline...")
    well_path_coll = project.descendants(rips.WellPathCollection)[0]
    timeline = well_path_coll.event_timeline()
    report = rips.orion_events.apply_orion_document(
        document, timeline, project, case=case
    )
    print(f"   Events applied: {report.events_applied}")
    print(f"   Events skipped: {report.events_skipped}")
    print(f"   Report dates:   {report.report_dates}")
    for warning in report.warnings:
        print(f"   WARNING: {warning}")
    for error in report.errors:
        print(f"   ERROR:   {error}")

    # Apply events up to a date to materialize completions
    timeline.set_timestamp(timestamp="2024-12-24")

    print("\n4. Verifying created completions and WELSPECS settings...")
    completion_settings = well_path.completion_settings()
    print("   Completion export settings after the latest WELSPECS:")
    print(f"      Group:       {completion_settings.group_name_for_export}")
    print(f"      Cross-flow:  {completion_settings.allow_well_cross_flow}")
    print(f"      Ref. depth:  {completion_settings.reference_depth_for_export}")
    print(f"      Phase:       {completion_settings.well_type_for_export}")

    perforations = well_path.completions().perforations().perforations()
    print(f"   Perforations created: {len(perforations)}")
    for perf in perforations:
        # The HIPORO filter was carried from the perforation event onto the
        # materialized perforation interval.
        cell_filter = perf.cell_filter()
        filter_note = f"  (filter: {cell_filter.name})" if cell_filter else ""
        print(
            f"      - MD {perf.start_measured_depth:.0f} to "
            f"{perf.end_measured_depth:.0f}m{filter_note}"
        )

    print("\n5. Generating Eclipse schedule text from events...")
    if case is None:
        print("   No Eclipse case loaded - skipping schedule generation.")
        return
    # REPORT dates become bare DATES keywords via additional_dates. Aligned output
    # adds column-title comments; the schedule header identifies its timestamp and
    # user, and each generated WELSPECS record has a matching COMPORD INPUT record.
    schedule_text = timeline.generate_schedule_text(
        eclipse_case=case,
        export_msw_for_wells=[well_path],
        additional_dates=report.report_dates,
        align_columns=True,
    )
    if schedule_text:
        print(f"   Generated schedule text ({len(schedule_text)} characters)")
        print("   " + "=" * 60)
        for line in schedule_text.split("\n"):
            print(f"   {line}")
        print("   " + "=" * 60)

        expected_keywords = [
            "DATES",
            "WELSPECS",
            "COMPORD",
            "COMPDAT",
            "COMPLUMP",
            "WCONHIST",
            "WELTARG",
            "WRFTPLT",
            "RPTRST",
            "GRUPTREE",
            "TUNING",
            "WTRACER",
        ]
        found = [kw for kw in expected_keywords if kw in schedule_text]
        print(f"\n   Keywords found: {', '.join(found)}")
        if "14:45:30.500" in schedule_text:
            print("   DATES keyword preserves event time-of-day (14:45:30.500)")
    else:
        print("   Warning: No schedule text was generated")

    print("\nExample completed.")


if __name__ == "__main__":
    main()
