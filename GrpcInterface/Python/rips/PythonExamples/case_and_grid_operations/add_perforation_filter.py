#######################################################
#
# Demonstrates associating a case-level cell filter
# with one or more perforation intervals on a well path.
#
# The combined filter is created at case level (under
# "Data Filters") and shared between two perforation
# intervals via add_filter(). The filter is stored as a
# typed reference, so two perforations can point to the
# same filter without duplicating its configuration.
#
# This is metadata only at the moment — completion
# export is not yet wired to consume the perforation's
# cell filter.
#
# Assumes a case is already loaded in the running
# ResInsight instance.
#
#######################################################

import rips

resinsight = rips.Instance.find()

cases = resinsight.project.cases()
assert cases, "No case loaded in ResInsight. Open a case before running this script."
case = cases[0]

data_filters = case.data_filter_collection()

# Create a case-level combined filter that AND-combines a PERMX property filter
# (100..20000) with a K-slice 5..10 range filter.
combined = data_filters.add_combined_filter(combine_mode="AND")

permx = combined.add_property_filter(
    result_variable="PERMX",
    result_type="STATIC_NATIVE",
)
permx.lower_bound = 100.0
permx.upper_bound = 20000.0
permx.update()

combined.add_range_filter(name="K=5..10", start_k=5, cell_count_k=6)

# Place the well in the centre of the case's bounding box so the example works
# regardless of where the loaded reservoir lives in domain space.
#
# Note: bounding-box z is positive-up (sea level = 0, subsurface negative), but
# append_well_target expects z positive-downward (depth). Convert with -z.
bbox = case.reservoir_boundingbox()
center_x = 0.5 * (bbox.min_x + bbox.max_x)
center_y = 0.5 * (bbox.min_y + bbox.max_y)
depth_top = -bbox.max_z  # depth to reservoir top (positive)
depth_bot = -bbox.min_z  # depth to reservoir bottom (positive)

# Build a vertical modeled well path: surface -> reservoir top -> just below the
# reservoir bottom, all on the central (x, y).
well_path_coll = resinsight.project.well_path_collection()
well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
well_path.name = "Perforation Filter Demo Well"
well_path.update()

geometry = well_path.well_path_geometry()
geometry.append_well_target([center_x, center_y, 0.0])
geometry.append_well_target([center_x, center_y, depth_top])
geometry.append_well_target([center_x, center_y, depth_bot + 50.0])
geometry.update()

# Vertical well -> MD is depth below surface. Place two perforation intervals
# inside the reservoir's vertical extent.
md_span = depth_bot - depth_top
perf1_start = depth_top + 0.20 * md_span
perf1_end = perf1_start + 0.10 * md_span
perf2_start = depth_top + 0.55 * md_span
perf2_end = perf2_start + 0.20 * md_span

perf1 = well_path.append_perforation_interval(perf1_start, perf1_end, 0.2, 0.76)
perf1.add_filter(combined)

perf2 = well_path.append_perforation_interval(perf2_start, perf2_end, 0.2, 0.76)
perf2.add_filter(combined)

# Read back through the well path / completions tree to confirm both perforations
# resolve to the same filter object.
perforations = well_path.completions().perforations().perforations()
for p in perforations:
    f = p.cell_filter()
    print(
        f"  perforation {p.start_measured_depth} .. {p.end_measured_depth} -> "
        f"{f.name if f else '<no filter>'}"
    )
