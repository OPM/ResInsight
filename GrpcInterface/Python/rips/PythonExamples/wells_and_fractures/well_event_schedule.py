#!/usr/bin/env python3

"""
Example demonstrating how to use well event timeline and schedule data generation.

This example shows:
1. Creating well events (perforations, tubing, valves) on a timeline
2. Applying events up to a specific date using set_timestamp()
3. Viewing the created completions after applying events

This workflow is useful for:
- Time-dependent well completion modeling
- Simulating well workover schedules
- Planning and visualizing completion changes over time
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
    well_path_coll = project.well_path_collection()
    wells = project.well_paths()

    if (len(wells) > 0):
        well_path = wells[0]

    print("Well name: ", well_path.name)

    # Get the event timeline
    print("\n2. Adding well events to the timeline...")
    timeline = well_path.event_timeline()

    # Add tubing event (installed early)
    _tubing_event = timeline.add_tubing_event(
        event_date="2024-01-01",
        well_name=well_path.name,
        start_md=0.0,
        end_md=2500.0,
        inner_diameter=0.15,
        roughness=1.0e-5,
    )
    print("   Added tubing event on 2024-01-01 (MD 0-2500m)")

    # Add first perforation event
    _perf_event1 = timeline.add_perf_event(
        event_date="2024-02-01",
        well_name=well_path.name,
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
        well_name=well_path.name,
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
        well_name=well_path.name,
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
        well_name=well_path.name,
        well_state="OPEN",
    )
    print("   Added state event on 2024-02-15 (OPEN)")

    # Demonstrate applying events up to different dates
    print("\n3. Applying events up to March 15, 2024...")
    print("   This will create completions from events dated on or before March 15")

    # Apply events up to March 15, 2024
    # This should create:
    # - Tubing interval (Jan 1)
    # - First perforation (Feb 1)
    # - Valve in first perforation (Mar 1)
    # But NOT the second perforation (Apr 1)
    timeline.set_timestamp(timestamp="2024-03-15")

    # Show what was created
    print("\n4. Verifying created completions...")

    # Check perforations
    perforation_coll = well_path.completions().perforations()
    perforations = perforation_coll.perforations()
    print(f"   Perforations created: {len(perforations)}")
    for perf in perforations:
        print(f"      - MD {perf.start_measured_depth:.0f} to {perf.end_measured_depth:.0f}m")
        valves = perf.valves()
        if valves:
            print(f"        Valves: {len(valves)}")

    # Check MSW settings (tubing intervals)
    msw_settings = well_path.msw_settings()
    if msw_settings:
        print(f"   MSW diameter/roughness mode: {msw_settings.diameter_roughness_mode}")

    print("\n5. Now applying remaining events (up to Dec 31, 2024)...")
    timeline.set_timestamp(timestamp="2024-12-31")

    # Show updated completions
    perforations = perforation_coll.perforations()
    print(f"   Perforations after full application: {len(perforations)}")
    for perf in perforations:
        print(f"      - MD {perf.start_measured_depth:.0f} to {perf.end_measured_depth:.0f}m")

    print("\nExample completed successfully!")
    print("\nAPI Usage Summary:")
    print("- timeline = well_path.event_timeline()")
    print("- timeline.add_perf_event(event_date='2024-01-01', well_name='WellA', ...)")
    print(
        "- timeline.add_tubing_event(event_date='2024-01-01', well_name='WellA', ...)"
    )
    print("- timeline.add_valve_event(event_date='2024-01-01', well_name='WellA', ...)")
    print("- timeline.set_timestamp(timestamp='2024-06-01')  # Apply events up to date")


if __name__ == "__main__":
    main()
