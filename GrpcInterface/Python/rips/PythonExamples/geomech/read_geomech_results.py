######################################################################
# This script loads a geomechanical case (.odb, .inp or VTK .pvd) and
# reads time steps, result names and result values into Python lists.
#
# Usage: python read_geomech_results.py <path to geomech case (.odb, .inp or VTK .pvd)>
######################################################################
import math
import sys

import rips

if len(sys.argv) < 2:
    print("Usage: read_geomech_results.py <path to geomech case>")
    sys.exit(1)

try:
    resinsight = rips.Instance.find()
    case = resinsight.project.load_case(sys.argv[1])
except Exception as exc:
    print(f"Failed to load geomechanical case: {exc}")
    sys.exit(1)

if not isinstance(case, rips.GeoMechCase):
    print("The file is not a geomechanical case")
    sys.exit(1)

time_steps = case.time_step_strings()
print("Time steps:", time_steps)

# Result names are "FIELD:COMPONENT", or "FIELD" for results without components
for position in rips.GeoMechResultPosition:
    if position in (
        rips.GeoMechResultPosition.WELLPATH_DERIVED,
        rips.GeoMechResultPosition.DIFFERENTIALS,
    ):
        continue
    print(position, case.result_names(position))

# Print statistics for every result with defined values at one position.
# A negative frame_index (the default) selects the last frame of the time step.
position = rips.GeoMechResultPosition.NODAL
for name in case.result_names(position):
    field_name, _, component_name = name.partition(":")
    for time_step, label in enumerate(time_steps):
        values = case.result_values(
            field_name,
            component_name,
            time_step=time_step,
            result_position=position,
        )
        finite = [v for v in values if math.isfinite(v)]
        if finite:
            print(
                f"{name}, {label}: {len(values)} values, "
                f"min={min(finite):.4g}, max={max(finite):.4g}"
            )
