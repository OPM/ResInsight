#######################################################
#
# Adds a case-level "Data Filters" combined filter that
# AND-combines a PERMX property filter with a K-slice
# range filter (K = 5..10), then shows how to mutate
# the property filter's lower bound after creation.
#
# Assumes a case is already loaded in the running
# ResInsight instance.
#
#######################################################

import rips

resinsight = rips.Instance.find()

# Use the first open case in the project. Add a guard so the script fails
# loudly if the user runs it against an empty project.
cases = resinsight.project.cases()
assert cases, "No case loaded in ResInsight. Open a case before running this script."
case = cases[0]

# Case-level Data Filters: shared by every view of the case.
data_filters = case.data_filter_collection()

# AND-combined parent.
combined = data_filters.add_combined_filter(
    name="PERMX 100..20000 AND K=5..10",
    combine_mode="AND",
)

# PERMX property filter, then override the bounds populated by setToDefaultValues.
permx = combined.add_property_filter(
    result_variable="PERMX",
    result_type="STATIC_NATIVE",
)
permx.lower_bound = 100.0
permx.upper_bound = 20000.0
permx.update()

# K-slice filter for K = 5..10. 1-based Eclipse indexing — start_k=5, count=6 covers 5..10.
combined.add_range_filter(
    name="K=5..10",
    start_k=5,
    cell_count_k=6,
)

# Mutate the lower bound later via field assignment. Re-read from the project
# tree to confirm the change round-trips.
permx.lower_bound = 250.0
permx.update()

permx_refreshed = combined.filters()[0]
print(f"PERMX lower_bound after update: {permx_refreshed.lower_bound}")
print(f"PERMX upper_bound after update: {permx_refreshed.upper_bound}")
