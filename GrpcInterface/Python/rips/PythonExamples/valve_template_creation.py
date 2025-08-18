#!/usr/bin/env python3

"""
Example demonstrating how to create valve templates programmatically using the Python API.

This example shows:
1. Creating different types of valve templates (ICD, ICV, AICD)
2. Setting custom parameters for orifice diameter and flow coefficient
3. Using the created templates in well completions
"""

import rips


def main():
    # Connect to ResInsight instance
    resinsight = rips.Instance.find()

    print("Creating valve templates...")

    # Get the valve template collection
    valve_templates = resinsight.project.valve_templates()

    # Check initial number of templates
    initial_templates = valve_templates.valve_definitions()
    print(f"Initial number of valve templates: {len(initial_templates)}")

    # Create an ICD template with default values
    print("\n1. Creating ICD template with default values")
    icd_template = valve_templates.add_template(completion_type="ICD")
    print(f"   Created: {icd_template.name}")
    print(f"   Orifice Diameter: {icd_template.orifice_diameter}")
    print(f"   Flow Coefficient: {icd_template.flow_coefficient}")

    # Create an ICV template with custom values
    print("\n2. Creating ICV template with custom values")
    icv_template = valve_templates.add_template(
        completion_type="ICV",
        orifice_diameter=12.5,
        flow_coefficient=0.85,
        user_label="Custom ICV for High Flow",
    )
    print(f"   Created: {icv_template.name}")
    print(f"   Orifice Diameter: {icv_template.orifice_diameter}")
    print(f"   Flow Coefficient: {icv_template.flow_coefficient}")

    # Create an AICD template (as suggested in the issue)
    print("\n3. Creating AICD template as suggested in issue #12773")
    aicd_template = valve_templates.add_template(
        completion_type="AICD",
        orifice_diameter=7.3,
        flow_coefficient=0.2,
        user_label="Issue Example AICD",
    )
    print(f"   Created: {aicd_template.name}")
    print(f"   Orifice Diameter: {aicd_template.orifice_diameter}")
    print(f"   Flow Coefficient: {aicd_template.flow_coefficient}")

    # Show all valve templates
    current_templates = valve_templates.valve_definitions()
    print(f"\nTotal valve templates now: {len(current_templates)}")
    print("All valve templates:")
    for i, template in enumerate(current_templates):
        print(f"   {i + 1}. {template.name}")

    # Example of using the new template in a completion (requires a loaded case with well paths)
    print("\n4. Example usage in well completion (requires loaded case)")
    try:
        # This assumes you have a case loaded with well paths
        well_paths = resinsight.project.well_paths()
        if well_paths:
            well_path = well_paths[0]
            print(f"   Using well path: {well_path.name}")

            # Add a perforation interval
            perf_interval = well_path.append_perforation_interval(
                start_md=2450, end_md=2500, diameter=0.25, skin_factor=0.1
            )

            # Add a valve using our new template
            valve = perf_interval.add_valve(
                template=aicd_template, start_md=2451, end_md=2499, valve_count=3
            )
            print(f"   Created valve: {valve.name}")
            print(f"   Number of valves in interval: {len(perf_interval.valves())}")
        else:
            print("   No well paths available - skipping completion example")
            print("   Load a case with well paths to see this in action")

    except Exception as e:
        print(f"   Could not create completion example: {e}")
        print("   This is expected if no case/well paths are loaded")

    print("\nExample completed successfully!")
    print("\nAPI Usage Summary:")
    print("- valve_templates.add_template(completion_type='ICD')  # Use defaults")
    print(
        "- valve_templates.add_template(completion_type='AICD', orifice_diameter=7.3, flow_coefficient=0.2)"
    )
    print(
        "- valve_templates.add_template(completion_type='ICV', user_label='My Custom Template')"
    )


if __name__ == "__main__":
    main()
