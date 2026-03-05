"""
Example demonstrating how to create well paths from XYZ coordinates
and add a tie-in valve, adjusting the valve MD to a custom value.
"""

import rips

rips_instance = rips.Instance.find()


well_path_coll = rips_instance.project.well_path_collection()

# Create main well path using ModeledWellPath with targets
main_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
main_well_path.name = "main_well_check_connection"
main_well_path.update()

# Add geometry targets to main well path
main_geometry = main_well_path.well_path_geometry()
main_geometry.append_well_target([1000.0, 2000.0, 0.0])  # Surface
main_geometry.append_well_target([1000.0, 2000.0, -100.0])  # 100m down
main_geometry.append_well_target([1000.0, 2000.0, -300.0])  # 300m down
main_geometry.append_well_target([1000.0, 2000.0, -500.0])  # 500m down
main_geometry.use_auto_generated_target_at_sea_level = False
main_geometry.update()

measured_depth = 150
lateral = main_well_path.append_lateral(measured_depth)

# Get the valve template collection
valve_templates = rips_instance.project.valve_templates()

# create an ICV valve template
icv_template = valve_templates.add_template(
    completion_type="ICV",
    orifice_diameter=12.5,
    flow_coefficient=0.85,
    user_label="Custom ICV for Lateral",
)

# Enable well valve
valve = lateral.enable_outlet_valve(
    enable=True, icv_template=icv_template, use_custom_valve_md=False
)

# Enable a custom md for the outlet valve
tiein = lateral.tie_in()

custom_enabled, custom_md = tiein.custom_outlet_valve_md
print("Custom outlet valve md: " + str(custom_enabled))

tiein.custom_outlet_valve_md = (True, 200.0)
tiein.update()

custom_enabled, custom_md = tiein.custom_outlet_valve_md
print("Custom outlet valve md: " + str(custom_enabled) + " at " + str(custom_md))
