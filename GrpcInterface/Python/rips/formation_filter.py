"""
Formation filters

Formation filters select cells by formation. The selection is stored as formation
indices, the position of a formation in the formation list of the filter case.
For a grid ensemble, the filter case is the main case of the ensemble, and the
same selection is applied to all realizations. Realizations with the same
formation names in the same order work as expected, also if the K ranges of the
formations differ. Use realizations_with_mismatched_formations() to find
realizations where this does not hold.
"""

from typing import List

from .pdmobject import add_method
from .resinsight_classes import EclipseCase, FormationFilter, ReservoirGridEnsemble


@add_method(FormationFilter)
def selected_formation_names(self: FormationFilter) -> List[str]:
    """Get the names of the selected formations

    A formation occurring more than once in the formation list is returned once.

    Returns:
        List of formation names, in formation list order
    """
    return list(self.selected_formation_names_internal().values)


@add_method(FormationFilter)
def realizations_with_mismatched_formations(
    self: FormationFilter,
) -> List[EclipseCase]:
    """Get the ensemble realizations with formation names different from the main case

    The selection of the filter is resolved against the main case of the ensemble.
    In realizations with other formation names, or the same names in another order,
    the filter can select other formations than intended.

    Returns:
        List of realizations. Empty if all realizations match, or if the filter
        does not belong to a grid ensemble.
    """
    case_ids = {
        int(case_id) for case_id in self.mismatched_realization_ids_internal().values
    }
    if not case_ids:
        return []

    ensemble = self.ancestor(ReservoirGridEnsemble)
    if ensemble is None:
        return []

    return [
        realization
        for realization in ensemble.descendants(EclipseCase)
        if realization.id in case_ids
    ]


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
