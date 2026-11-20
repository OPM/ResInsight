#######################################################
#
# Formation filters on a case and on grid ensembles.
#
# A formation filter selects cells by formation. The
# selection is stored as formation indices, resolved
# against the case of the filter. For a grid ensemble
# this is the main case, the first realization, and the
# same selection is used for all realizations. Each
# realization resolves the selected formation names
# against its own formation list, so other formation
# order is handled. Selected formations missing in a
# realization select no cells in that realization.
#
# Assumes a case with formation names is loaded in the
# running ResInsight instance. Formations can be imported
# with project.import_formation_names().
#
#######################################################

import rips

resinsight = rips.Instance.find()
project = resinsight.project

# ---------------------------------------------------------------------------
# Single case
# ---------------------------------------------------------------------------
cases = project.cases()
if not cases:
    raise RuntimeError(
        "No case loaded in ResInsight. Open a case before running this script."
    )
case = cases[0]

data_filters = case.data_filter_collection()

# All formations of the case. An empty list selects all.
all_formations = data_filters.add_formation_filter()
formation_names = all_formations.selected_formation_names()
print("Formations:", formation_names)

if formation_names:
    # Formation filter combined with a PERMX property filter
    combined = data_filters.add_combined_filter(combine_mode="AND")
    formation = combined.add_formation_filter(formation_names=formation_names[:1])
    print(f"Created '{formation.name}'")

    permx = combined.add_property_filter(
        result_variable="PERMX", result_type="STATIC_NATIVE"
    )
    permx.lower_bound = 10.0
    permx.update()

    mask = case.filtered_cells(combined)
    print(f"{sum(mask)} of {len(mask)} cells selected")

    # Exclude formations, and change the selection afterwards
    excluded = data_filters.add_formation_filter(
        formation_names=formation_names[:1], filter_mode="EXCLUDE"
    )
    excluded.set_selected_formation_names(formation_names=formation_names[-1:])
    print("Excluded:", excluded.selected_formation_names())

# Unknown names are an error, and the message lists the available formations
try:
    data_filters.add_formation_filter(formation_names=["Does Not Exist"])
except rips.RipsError as e:
    print("Expected error:", e)

# ---------------------------------------------------------------------------
# Grid ensembles
# ---------------------------------------------------------------------------
for ensemble in project.descendants(rips.ReservoirGridEnsemble):
    print(f"Ensemble '{ensemble.name}'")
    print("  Main case formations:", ensemble.formation_names())
    print("  All realizations:    ", ensemble.formation_names(union=True))

    main_case_formations = ensemble.formation_names()
    if not main_case_formations:
        continue

    formation = ensemble.data_filter_collection().add_formation_filter(
        formation_names=main_case_formations[:1]
    )

    # Each realization resolves the selected names against its own formations
    print("  Filter selects:", formation.selected_formation_names())
