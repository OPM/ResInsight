#!/usr/bin/env python3

"""
Example demonstrating how to use well event timeline and schedule data generation.

This example shows:
1. Creating well events (perforations, tubing, valves) on a timeline
2. Applying events up to a specific date using set_timestamp()
3. Viewing the created completions after applying events
4. Generating Eclipse schedule text from the event timeline

This workflow is useful for:
- Time-dependent well completion modeling
- Simulating well workover schedules
- Planning and visualizing completion changes over time
- Generating schedule files for Eclipse simulation
"""

import rips


def main():
    # Connect to ResInsight instance
    resinsight = rips.Instance.find()
    project = resinsight.project

    print("Well Event Schedule Example")
    print("=" * 50)

    # Create a modeled well path for demonstration
    print("\n1. Finding well")
    wells = project.well_paths()

    if len(wells) > 0:
        well_path = wells[0]

    print("Well name: ", well_path.name)

    # Get the event timeline
    print("\n2. Adding well events to the timeline...")
    well_path_coll = project.descendants(rips.WellPathCollection)[0]
    timeline = well_path_coll.event_timeline()

    # Add tubing event (installed early)
    _tubing_event = timeline.add_tubing_event(
        event_date="2024-01-01",
        well_path=well_path,
        start_md=0.0,
        end_md=2500.0,
        inner_diameter=0.15,
        roughness=1.0e-5,
    )
    print("   Added tubing event on 2024-01-01 (MD 0-2500m)")

    # Add first perforation event
    _perf_event1 = timeline.add_perf_event(
        event_date="2024-02-01",
        well_path=well_path,
        start_md=2000.0,
        end_md=2200.0,
        diameter=0.1,
        skin_factor=0.5,
        state="OPEN",
    )
    print("   Added perforation event on 2024-02-01 (MD 2000-2200m)")

    # Add second perforation event (later)
    _perf_event2 = timeline.add_perf_event(
        event_date="2024-04-01",
        well_path=well_path,
        start_md=2400.0,
        end_md=2600.0,
        diameter=0.1,
        skin_factor=0.3,
        state="OPEN",
    )
    print("   Added perforation event on 2024-04-01 (MD 2400-2600m)")

    # Add valve event (requires existing perforation)
    _valve_event = timeline.add_valve_event(
        event_date="2024-03-01",
        well_path=well_path,
        measured_depth=2100.0,
        valve_type="ICV",
        state="OPEN",
        flow_coefficient=0.7,
        area=0.0001,
    )
    print("   Added valve event on 2024-03-01 (MD 2100m)")

    # Add state events (for documentation, not applied to completions)
    _state_event = timeline.add_state_event(
        event_date="2024-02-15",
        well_path=well_path,
        well_state="OPEN",
    )
    print("   Added state event on 2024-02-15 (OPEN)")

    # Add well keyword events (arbitrary Eclipse keywords)
    print("\n   Adding well keyword events...")

    # Example 1: WCONHIST - Historical production data
    _wconhist_event = timeline.add_well_keyword_event(
        event_date="2024-01-15",
        well_path=well_path,
        keyword_name="WCONHIST",
        keyword_data={
            "WELL": well_path.name,
            "STATUS": "OPEN",
            "CMODE": "RESV",
            "ORAT": 3999.99,
            "WRAT": 0.01,
            "GRAT": 550678.44,
            "VFP_TABLE": 1,
        },
    )
    print("   Added WCONHIST event on 2024-01-15 (historical production control)")

    # Example 2: WELTARG - Well target change
    _weltarg_event = timeline.add_well_keyword_event(
        event_date="2024-05-01",
        well_path=well_path,
        keyword_name="WELTARG",
        keyword_data={
            "WELL": well_path.name,
            "CMODE": "ORAT",
            "NEW_VALUE": 5000.0,
        },
    )
    print("   Added WELTARG event on 2024-05-01 (well target change)")

    # Example 3: WRFTPLT - RFT/PLT output control
    _wrftplt_event = timeline.add_well_keyword_event(
        event_date="2024-06-01",
        well_path=well_path,
        keyword_name="WRFTPLT",
        keyword_data={
            "WELL": well_path.name,
            "OUTPUT_RFT": "YES",
            "OUTPUT_PLT": "NO",
            "OUTPUT_SEGMENT": "NO",
        },
    )
    print("   Added WRFTPLT event on 2024-06-01 (RFT/PLT output control)")

    # Add schedule-level keyword events (not tied to a well)
    print("\n   Adding schedule-level keyword events...")

    # Example 4: RPTRST - Report restart settings (schedule-level, not well-specific)
    _rptrst_event = timeline.add_keyword_event(
        event_date="2024-01-01",
        keyword_name="RPTRST",
        keyword_data={
            "BASIC": 2,
            "FREQ": 1,
        },
    )
    print("   Added RPTRST event on 2024-01-01 (report restart settings)")

    # Example 5: GRUPTREE - Group tree definition
    _gruptree_event = timeline.add_keyword_event(
        event_date="2024-01-01",
        keyword_name="GRUPTREE",
        keyword_data={
            "CHILD": "OP",
            "PARENT": "FIELD",
        },
    )
    print("   Added GRUPTREE event on 2024-01-01 (group tree definition)")

    # Apply events up to March 15, 2024
    # This should create:
    # - Tubing interval (Jan 1)
    # - First perforation (Feb 1)
    # - Valve in first perforation (Mar 1)
    # But NOT the second perforation (Apr 1)
    timeline.set_timestamp(timestamp="2024-12-24")

    # Get the Eclipse case (if available)
    # Show what was created
    print("\n3. Verifying created completions...")

    # Check perforations
    perforation_coll = well_path.completions().perforations()
    perforations = perforation_coll.perforations()
    print(f"   Perforations created: {len(perforations)}")
    for perf in perforations:
        print(
            f"      - MD {perf.start_measured_depth:.0f} to {perf.end_measured_depth:.0f}m"
        )
        valves = perf.valves()
        if valves:
            print(f"        Valves: {len(valves)}")

    # Check MSW settings (tubing intervals)
    msw_settings = well_path.msw_settings()
    if msw_settings:
        print(f"   MSW diameter/roughness mode: {msw_settings.diameter_roughness_mode}")

    # Generate Eclipse schedule text
    print("\n4. Generating Eclipse schedule text from events...")

    # Get the Eclipse case (if available)
    cases = project.cases()
    if cases:
        case = cases[0]
        print(f"   Using Eclipse case: {case.name}")

        # Generate schedule text
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)

        if schedule_text:
            print(f"\n   Generated schedule text ({len(schedule_text)} characters)")
            print("   " + "=" * 60)
            # Show the full schedule
            lines = schedule_text.split("\n")
            for line in lines:
                print(f"   {line}")
            print("   " + "=" * 60)

            # Validate expected keywords are present
            print("\n7. Validating generated Eclipse keywords...")
            expected_keywords = [
                "DATES",
                "WELSEGS",
                "COMPSEGS",
                "WCONHIST",
                "WELTARG",
                "WRFTPLT",
                "RPTRST",
                "GRUPTREE",
            ]
            found_keywords = [kw for kw in expected_keywords if kw in schedule_text]

            print(f"   Keywords found: {', '.join(found_keywords)}")

            if "WELSEGS" in schedule_text:
                print("   ✓ WELSEGS keyword generated (MSW well segments)")
                # Count segment lines (lines with segment data)
                welsegs_lines = [
                    line
                    for line in lines
                    if line.strip() and line.strip()[0].isdigit() and " 1 " in line
                ]
                print(f"     Number of segments: {len(welsegs_lines)}")

            if "COMPSEGS" in schedule_text:
                print("   ✓ COMPSEGS keyword generated (completion segments)")

            if "WSEGVALV" in schedule_text:
                print("   ✓ WSEGVALV keyword generated (segment valves)")
                # Extract valve parameters
                wsegvalv_idx = schedule_text.find("WSEGVALV")
                if wsegvalv_idx > 0:
                    wsegvalv_section = schedule_text[wsegvalv_idx : wsegvalv_idx + 500]
                    if "0.7" in wsegvalv_section:
                        print("     Flow coefficient (Cv) = 0.7 detected")

            if "WCONPROD" in schedule_text or "WCONINJE" in schedule_text:
                kw_name = "WCONPROD" if "WCONPROD" in schedule_text else "WCONINJE"
                print(f"   ✓ {kw_name} keyword generated (well control)")

            # Show keyword summary
            print("\n   Eclipse Keyword Summary:")
            print(f"   - DATES entries: {schedule_text.count('DATES')}")
            print(f"   - WELSEGS entries: {schedule_text.count('WELSEGS')}")
            print(f"   - COMPSEGS entries: {schedule_text.count('COMPSEGS')}")
            print(f"   - WSEGVALV entries: {schedule_text.count('WSEGVALV')}")
            print(f"   - WCONHIST entries: {schedule_text.count('WCONHIST')}")
            print(f"   - WELTARG entries: {schedule_text.count('WELTARG')}")
            print(f"   - WRFTPLT entries: {schedule_text.count('WRFTPLT')}")
            print(f"   - RPTRST entries: {schedule_text.count('RPTRST')}")
            print(f"   - GRUPTREE entries: {schedule_text.count('GRUPTREE')}")

            # Save to file
            output_file = "generated_schedule.sch"
            with open(output_file, "w") as f:
                f.write(schedule_text)
            print(f"\n   Schedule text saved to: {output_file}")

            # Show example of generated keywords
            print("\n8. Example of generated Eclipse keywords:")
            print("   (See generated_schedule.sch for complete output)")
            if "WELSEGS" in schedule_text:
                print("\n   Sample WELSEGS segment:")
                for line in lines:
                    if "WELSEGS" in line:
                        idx = lines.index(line)
                        # Show keyword and first data line
                        if idx + 2 < len(lines):
                            print(f"   {lines[idx]}")
                            print(f"   {lines[idx + 1]}")
                            print(f"   {lines[idx + 2]}")
                        break
        else:
            print("   Warning: No schedule text was generated")
    else:
        print("   Note: No Eclipse case loaded - skipping schedule generation")
        print("   Load a case first to generate schedule text")

    print("\nExample completed successfully!")
    print("\nAPI Usage Summary:")
    print("- timeline = well_path.event_timeline()")
    print("- timeline.add_perf_event(event_date='2024-01-01', well_name='WellA', ...)")
    print(
        "- timeline.add_tubing_event(event_date='2024-01-01', well_name='WellA', ...)"
    )
    print("- timeline.add_valve_event(event_date='2024-01-01', well_name='WellA', ...)")
    print("- timeline.add_well_keyword_event(  # Well-specific Eclipse keywords:")
    print("      event_date='2024-01-01',")
    print("      well_path=well_path,")
    print("      keyword_name='WCONHIST',  # WCONHIST, WELTARG, WRFTPLT, etc.")
    print("      keyword_data={'WELL': 'WellA', 'ORAT': 1000.0, ...}")
    print("  )")
    print(
        "- timeline.add_keyword_event(  # Schedule-level keywords (not tied to wells):"
    )
    print("      event_date='2024-01-01',")
    print("      keyword_name='RPTRST',  # RPTRST, GRUPTREE, RPTSCHED, etc.")
    print("      keyword_data={'BASIC': 2, 'FREQ': 1}")
    print("  )")
    print("- timeline.set_timestamp(timestamp='2024-06-01')  # Apply events up to date")
    print("- schedule_text = timeline.generate_schedule_text(eclipse_case=case)")
    print("  # Generate Eclipse schedule text")


if __name__ == "__main__":
    main()
