"""
Example demonstrating how to create a stand-alone ICV valve,
shut it and move it deeper
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

# Add a stand-alone valve at md 240
valve = main_well_path.add_icv_valve(240.0)

# default for valve should be open
if valve.is_open:
    print("Valve %s is open!" % valve.name)
else:
    print("Valve shut!")

# close valve and move it to md = 275
valve.is_open = False
valve.start_measured_depth = 275
valve.update()

# valve should now be shut
if valve.is_open:
    print("Valve %s is open!" % valve.name)
else:
    print("Valve shut!")

# valve should be at md = 275
print("Valve measured depth: " + str(valve.start_measured_depth))
