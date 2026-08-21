#!/usr/bin/env python3

"""
Example: import an ORIONEVENTS well-event-timeline file into ResInsight.

This example shows how to:
1. Parse an ORIONEVENTS text file into a structured document
2. Normalize matching keyword events while retaining same-date perforations
3. Apply its events to the well event timeline (perforations, WCONHIST, WELTARG),
   materializing FILTER declarations as case-level combined data filters
   attached to the perforations
4. Insert multiline RAW_TEXT at a selected position in the generated schedule
5. Generate Eclipse schedule text from the resulting timeline

The ORIONEVENTS format is a compact, human-authored description of dated well
events. See rips/orion_events.py for the grammar. A sample input file ships at
rips/example_input_files/well_events.orion.

The well names in the file ("55_33-A-1", ...) must match well paths that exist
in the open project, so this example assumes a project with matching wells and
an Eclipse case is already loaded. The case must also hold the results the
FILTER expressions reference (PORO and PERMX in the sample file); a missing
result raises before any event is applied.
"""

import os

import rips
import rips.orion_events


def main():
    resinsight = rips.Instance.find()
    project = resinsight.project

    print("Import ORIONEVENTS Example")
    print("=" * 50)

    # Locate the sample ORIONEVENTS file shipped alongside the rips package.
    orion_file = os.path.join(
        os.path.dirname(rips.__file__), "example_input_files", "well_events.orion"
    )
    print(f"\n1. Parsing: {orion_file}")
    document = rips.orion_events.parse_orion_events_file(orion_file)
    print(f"   Version: {document.version}, units: {document.unit_system}")
    print(f"   Wells: {[w.well_name for w in document.wells]}")
    print(
        f"   Variables: { {k: f'{v.kind} {v.value}' for k, v in document.variables.items()} }"
    )

    # Normalization merges matching keyword events, but events that create or
    # expand domain objects remain separate. The sample has three perforations
    # at A1_STARTUP; all three are retained.
    normalized = rips.orion_events.coalesce_orion_document(document)
    source_perforation_count = sum(
        event.event_type == "PERFORATION"
        for well in document.wells
        for event in well.events
    )
    normalized_perforation_count = sum(
        event.event_type == "PERFORATION"
        for well in normalized.wells
        for event in well.events
    )
    print(
        "   Perforations retained during normalization: "
        f"{source_perforation_count} -> {normalized_perforation_count}"
    )

    # RAW_TEXT bodies bypass keyword parsing and formatting. Placement and an
    # optional anchor control where each block appears in the generated schedule.
    raw_text_events = [
        event for event in document.schedule_events if event.event_type == "RAW_TEXT"
    ]
    for event in raw_text_events:
        print(
            f"   RAW_TEXT: {event.raw_placement} {event.raw_anchor or ''} "
            f"(priority {event.raw_priority})"
        )

    # The sample file uses FILTER declarations, so a case is needed to resolve
    # the referenced result names and to own the created combined filters.
    cases = project.cases()
    if not cases:
        print(
            "\nNo Eclipse case loaded - the sample file uses FILTER, "
            "which needs a case. Load a case and rerun."
        )
        return
    case = cases[0]

    # Apply the parsed document to the shared well event timeline.
    print("\n2. Applying events to the timeline...")
    well_path_coll = project.descendants(rips.WellPathCollection)[0]
    timeline = well_path_coll.event_timeline()

    report = rips.orion_events.apply_orion_document(
        document, timeline, project, case=case, on_unknown_well="warn"
    )
    print(f"   Events applied: {report.events_applied}")
    print(f"   Events skipped: {report.events_skipped}")
    print(f"   Report dates:   {report.report_dates}")
    for warning in report.warnings:
        print(f"   WARNING: {warning}")
    for error in report.errors:
        print(f"   ERROR:   {error}")

    # FILTER declarations referenced by applied perforations now exist as
    # combined data filters under the case's "Data Filters" node; they are
    # carried onto the perforation intervals when completions are materialized
    # with timeline.set_timestamp().
    print("\n3. Case-level data filters created from FILTER declarations:")
    data_filters = case.data_filter_collection().filters()
    if data_filters:
        for cell_filter in data_filters:
            print(f"   {cell_filter.name}")
    else:
        print("   (none - no applied perforation referenced a filter)")

    # Generate Eclipse schedule text from the timeline.
    print("\n4. Generating Eclipse schedule text...")
    if report.events_applied == 0 and not report.report_dates:
        print("   No events or report dates found - skipping schedule generation.")
        return
    schedule_text = timeline.generate_schedule_text(
        eclipse_case=case,
        export_msw_for_wells=project.well_paths(),
        additional_dates=report.report_dates,
        align_columns=True,
    )
    if schedule_text:
        print(f"   Generated schedule text ({len(schedule_text)} characters):")
        print("   " + "=" * 60)
        for line in schedule_text.split("\n"):
            print(f"   {line}")
        print("   " + "=" * 60)
    else:
        print("   No schedule text generated.")

    print("\nExample completed.")


if __name__ == "__main__":
    main()
