#######################################################
#
# Creates a well target mapping for the first case in
# the project, prints the statistics for each cluster
# and re-runs the mapping with fewer targets.
#
# The mapping is limited to K-layers 1-5 by a grow data
# filter, and avoids growing clusters across faults.
#
# Assumes a case with SOIL, PRESSURE and PERMX is
# already loaded in the running ResInsight instance.
#
#######################################################

import rips

resinsight = rips.Instance.find()

cases = resinsight.project.cases()
assert cases, "No case loaded in ResInsight. Open a case before running this script."
case = cases[0]

layers = case.data_filter_collection().add_combined_filter(name="K 1-5")
layers.add_range_filter(name="K-slice 1-5", start_k=1, cell_count_k=5)

# time_step=-1 uses the last time step
mapping = case.add_well_target_mapping(
    name="Python Well Targets",
    time_step=-1,
    volume_type=rips.VolumeType.OIL,
    max_num_targets=5,
    grow_data_filter=layers,
    avoid_faults=True,
)

for stat in mapping.cluster_statistics():
    print(
        f"Cluster {stat.cluster_id}: {stat.num_cells} cells, "
        f"PORV*SOIL={stat.total_porv_soil:.1f}, "
        f"pressure={stat.pressure:.1f}, permeability={stat.permeability:.1f}"
    )

# The cluster id for each active cell is stored as a generated result
cluster_ids = case.active_cell_property(
    rips.PropertyType.GENERATED, "WELL_TARGET_NUM", mapping.time_step
)

# Edit the mapping and compute again
mapping.max_num_targets = 2
mapping.update()
mapping.compute()
print(f"Clusters after re-compute: {len(mapping.cluster_statistics())}")
