#######################################################
#
# Adds two case-level "Data Filters" combined filters
# that AND-combine a PERMX property filter with a K-slice
# range filter (K = 5..10):
#   1) one with an explicit name (preserved verbatim).
#   2) one with no name supplied, so the combined
#      filter's display name is auto-derived from its
#      children and updates when their names/bounds
#      change.
#
# Then shows how to mutate the PERMX lower bound after
# creation and confirms the auto-derived combined filter
# tracks the change while the named one stays put.
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


def _build_children(combined):
    """Add a PERMX property filter and a K=5..10 range filter to `combined`."""
    permx = combined.add_property_filter(
        result_variable="PERMX",
        result_type="STATIC_NATIVE",
    )
    permx.lower_bound = 100.0
    permx.upper_bound = 20000.0
    permx.update()

    combined.add_range_filter(
        name="K=5..10",
        start_k=5,
        cell_count_k=6,
    )
    return permx


# 1) Combined filter with a user-supplied name. The name is preserved verbatim
#    even when its children change.
named_combined = data_filters.add_combined_filter(
    name="My PERMX-and-K filter",
    combine_mode="AND",
)
named_permx = _build_children(named_combined)

# 2) Combined filter with no name. The display name is auto-derived from its
#    children and updates when child names change.
auto_combined = data_filters.add_combined_filter(combine_mode="AND")
auto_permx = _build_children(auto_combined)


def _print_state(label):
    print(f"--- {label} ---")
    # Re-read the collection so names reflect the current project tree.
    for c in data_filters.filters():
        print(f"  {c.name}")
        for child in c.filters():
            print(f"    - {child.name}")


_print_state("Initial state")

# Mutate the PERMX lower bound on both.
named_permx.lower_bound = 250.0
named_permx.update()

auto_permx.lower_bound = 250.0
auto_permx.update()

_print_state("After lower_bound 100 -> 250")
