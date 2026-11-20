#######################################################
#
# Runs a well target mapping on a reservoir grid
# ensemble where the realizations have varying geometry.
#
# The mapping is computed for every realization. The
# results are resampled onto a regular statistics grid
# covering all realizations, and P10/P50/P90/MEAN/MIN/MAX
# statistics are computed per cell together with the
# probability of a cell being part of a well target.
#
# Finally, a vertical well is added through each of the
# two most promising targets.
#
# Assumes a reservoir grid ensemble with SOIL, PRESSURE
# and PERMX is already loaded in the running ResInsight
# instance.
#
#######################################################

import math

import rips

resinsight = rips.Instance.find()

ensembles = resinsight.project.descendants(rips.ReservoirGridEnsemble)
assert ensembles, "No reservoir grid ensemble loaded in ResInsight."
ensemble = ensembles[0]
print(f"Using ensemble: {ensemble.name}")

# The realizations have different grids, so the results are resampled onto a
# regular statistics grid. The cell counts set the resolution of this grid, and
# the bounding box of the realizations is expanded by the given percentages.
# The mapping is created with the same defaults as the user interface, using the
# last time step.
mapping = ensemble.add_well_target_mapping(name="Ensemble Well Targets")
mapping.volume_type = rips.VolumeType.OIL
mapping.max_num_targets = 2
mapping.cell_count_i = 50
mapping.cell_count_j = 50
mapping.cell_count_k = 10
mapping.expand_bounding_box_xy_percent = 5.0
mapping.expand_bounding_box_z_percent = 10.0
mapping.update()
mapping.compute()

# Per cluster statistics are only available for single cases
assert len(mapping.cluster_statistics()) == 0

statistics_case = mapping.ensemble_statistics_case()
assert statistics_case is not None, "No ensemble statistics case was created."

print(f"Statistics case: {statistics_case.name}")
result_names = ["PROBABILITY", "OCCURRENCE"] + [
    f"TOTAL_PORV_SOIL_{suffix}"
    for suffix in ["P10", "P50", "P90", "MEAN", "MIN", "MAX"]
]
for result_name in result_names:
    # The ensemble statistics are static results stored on time step 0
    values = statistics_case.active_cell_property(
        rips.PropertyType.GENERATED, result_name, 0
    )
    defined = [v for v in values if math.isfinite(v) and v > 0.0]
    if defined:
        print(
            f"  {result_name:24s}: {len(defined):6d} cells, "
            f"sum={sum(defined):.4g}, max={max(defined):.4g}"
        )
    else:
        print(f"  {result_name:24s}: no cells")

# Rank the cells by the P10 oil volume of the target, and use the probability
# of being part of a well target to break ties
probability = statistics_case.active_cell_property(
    rips.PropertyType.GENERATED, "PROBABILITY", 0
)
p10_volume = statistics_case.active_cell_property(
    rips.PropertyType.GENERATED, "TOTAL_PORV_SOIL_P10", 0
)
cells = statistics_case.cell_info_for_active_cells()
centers = statistics_case.active_cell_centers()  # x, y and positive depth

candidates = [
    (p, v if math.isfinite(v) else 0.0, cell, center)
    for p, v, cell, center in zip(probability, p10_volume, cells, centers)
    if math.isfinite(p) and p > 0.0
]
candidates.sort(key=lambda c: (c[1], c[0]), reverse=True)
assert candidates, "No well targets found in the ensemble."

# Pick the two most promising targets. Neighbouring cells usually belong to the
# same target, so the second well must be at least MIN_WELL_SPACING away
# (horizontally) from the first.
MIN_WELL_SPACING = 500.0
selected = [candidates[0]]
for candidate in candidates[1:]:
    first_center = selected[0][3]
    center = candidate[3]
    spacing = math.hypot(center.x - first_center.x, center.y - first_center.y)
    if spacing >= MIN_WELL_SPACING:
        selected.append(candidate)
        break


# Add a vertical well through each selected target. The wells start above the
# top of the reservoir and end below the bottom.
min_depth, max_depth = statistics_case.reservoir_depth_range()
DEPTH_MARGIN = 50.0
top_depth = min_depth - DEPTH_MARGIN
bottom_depth = max_depth + DEPTH_MARGIN

well_path_collection = resinsight.project.well_path_collection()
for index, (p, volume, cell, center) in enumerate(selected, start=1):
    ijk = cell.local_ijk
    well_path = well_path_collection.add_new_object(rips.ModeledWellPath)
    well_path.name = f"Well Target {index}"
    well_path.update()

    geometry = well_path.well_path_geometry()
    geometry.reference_point = [center.x, center.y, top_depth]
    geometry.update()

    # Coordinates are x, y and positive depth
    geometry.append_well_target([center.x, center.y, top_depth], absolute=True)
    geometry.append_well_target([center.x, center.y, bottom_depth], absolute=True)

    print(
        f"Added '{well_path.name}' through cell [{ijk.i + 1}, {ijk.j + 1}, "
        f"{ijk.k + 1}] at ({center.x:.1f}, {center.y:.1f}, {center.z:.1f}): "
        f"probability={p:.2f}, P10 PORV*SOIL={volume:.4g}"
    )

if len(selected) < 2:
    print(f"Found no second target at least {MIN_WELL_SPACING} m from the first.")
