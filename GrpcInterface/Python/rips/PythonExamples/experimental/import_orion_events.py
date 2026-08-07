#!/usr/bin/env python3

"""
Example: import an ORIONEVENTS well-event-timeline file into ResInsight.

This example shows how to:
1. Parse an ORIONEVENTS text file into a structured document
2. Apply its events to the well event timeline (perforations, WCONHIST, WELTARG)
3. Generate Eclipse schedule text from the resulting timeline

The ORIONEVENTS format is a compact, human-authored description of dated well
events. See rips/orion_events.py for the grammar. A sample input file ships at
rips/example_input_files/well_events.orion.

The well names in the file ("55_33-A-1", ...) must match well paths that exist
in the open project, so this example assumes a project with matching wells and
an Eclipse case is already loaded.
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

    # Apply the parsed document to the shared well event timeline.
    print("\n2. Applying events to the timeline...")
    well_path_coll = project.descendants(rips.WellPathCollection)[0]
    timeline = well_path_coll.event_timeline()

    report = rips.orion_events.apply_orion_document(
        document, timeline, project, on_unknown_well="warn"
    )
    print(f"   Events applied: {report.events_applied}")
    print(f"   Events skipped: {report.events_skipped}")
    for warning in report.warnings:
        print(f"   WARNING: {warning}")
    for error in report.errors:
        print(f"   ERROR:   {error}")

    # Generate Eclipse schedule text from the timeline.
    print("\n3. Generating Eclipse schedule text...")
    cases = project.cases()
    if not cases:
        print("   No Eclipse case loaded - skipping schedule generation.")
        return

    case = cases[0]
    schedule_text = timeline.generate_schedule_text(
        eclipse_case=case, export_msw_for_wells=project.well_paths()
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
