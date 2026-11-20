"""
Formation filters

Formation filters select cells by formation. The selection is stored as formation
indices, the position of a formation in the formation list of the filter case.
For a grid ensemble, the filter case is the main case of the ensemble. Each
realization resolves the selected formation names against its own formation
list, so the filter selects the same formations also if a realization has its
formations in another order. Selected formations missing in a realization select
no cells in that realization.
"""

from typing import List

from .pdmobject import add_method
from .resinsight_classes import FormationFilter, ReservoirGridEnsemble


@add_method(FormationFilter)
def selected_formation_names(self: FormationFilter) -> List[str]:
    """Get the names of the selected formations

    A formation occurring more than once in the formation list is returned once.

    Returns:
        List of formation names, in formation list order
    """
    return list(self.selected_formation_names_internal().values)


@add_method(ReservoirGridEnsemble)
def formation_names(self: ReservoirGridEnsemble, union: bool = False) -> List[str]:
    """Get the formation names of the ensemble

    Arguments:
        union (bool): If False, return the formation names of the main case, which
            are used by formation filters. If True, return the union of the
            formation names of all realizations, in order of first appearance.

    Returns:
        List of formation names
    """
    return list(self.formation_names_internal(union=union).values)
