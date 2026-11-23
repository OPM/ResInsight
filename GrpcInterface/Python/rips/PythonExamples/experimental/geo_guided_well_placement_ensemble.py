#######################################################
#
# Geo-guided well placement on a reservoir grid ensemble.
#
# For each realization, a modeled well path is created
# from a source well path. The well targets inside the
# grid are moved vertically to the middle of the zone
# defined by a cell filter, and a perforation interval
# using the same cell filter covers the whole well.
#
# Assumes a reservoir grid ensemble and a well path are
# already loaded in the running ResInsight instance.
#
#######################################################

import rips

resinsight = rips.Instance.find()

ensembles = resinsight.project.descendants(rips.ReservoirGridEnsemble)
assert ensembles, "No reservoir grid ensemble loaded in ResInsight."
ensemble = ensembles[0]

well_paths = resinsight.project.well_paths()
assert well_paths, "No well paths loaded in ResInsight."
source_well = well_paths[0]

# The zone is K layers 3 and 4. The filter is defined on the ensemble, and is
# evaluated on each realization.
zone = ensemble.data_filter_collection().add_combined_filter(name="Zone K3-4")
zone.add_range_filter(name="K3-4", start_k=3, cell_count_k=2)

placement = ensemble.add_geo_guided_well_placement(name="Geo-Guided Wells")
placement.source_well_path = source_well
placement.cell_filter = zone
placement.perforation_filter = zone
placement.fraction = 0.5  # 0 is the bottom of the zone, 1 is the top
placement.resample_spacing = 50.0  # Target spacing used for imported source wells
placement.update()
placement.compute()

for well_path in resinsight.project.well_paths():
    if well_path.name.startswith(source_well.name + "_"):
        print(f"Created {well_path.name}")
