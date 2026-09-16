"""
Grid Case Group statistics module
"""

from .pdmobject import add_method
from .view import View as View
from .case import Case as Case

import warnings
from .resinsight_classes import GridCaseGroup
from .resinsight_classes import EclipseView
from .resinsight_classes import RimStatisticalCalculation


@add_method(GridCaseGroup)
def statistics_cases(self):
    """Get a list of all statistics cases in the Grid Case Group

    Returns:
        List of :class:`rips.generated.generated_classes.EclipseCase`

    """
    return self.descendants(RimStatisticalCalculation)


@add_method(GridCaseGroup)
def views(self):
    """Get a list of views belonging to a grid case group

    Returns:
        List of :class:`rips.generated.generated_classes.EclipseView`

    """
    resinsight_classes = self.descendants(EclipseView)
    view_list = []
    for pdm_object in resinsight_classes:
        view_list.append(pdm_object)
    return view_list


@add_method(GridCaseGroup)
def view(self, view_id):
    """Get a particular view belonging to a case group by providing view id

    Arguments:
        id(int): view id

    Returns:
        List of :class:`rips.generated.generated_classes.EclipseView`

    """
    views = self.views()
    for view_object in views:
        if view_object.id == view_id:
            return view_object
    return None


@add_method(GridCaseGroup)
def compute_statistics(self, case_ids=None):
    """Compute statistics for the given case ids

    Deprecated: use RimStatisticalCalculation.compute_statistics(update_views=True) on each
    statistics case (see statistics_cases()) instead.

    Arguments:
        case_ids(list of integers): List of case ids. If this is None all cases in group are included

    """
    warnings.warn(
        "GridCaseGroup.compute_statistics() is deprecated, use compute_statistics() on each statistics case instead",
        DeprecationWarning,
        stacklevel=3,
    )
    for statistics_case in self.statistics_cases():
        if case_ids is None or statistics_case.id in case_ids:
            statistics_case.compute_statistics(update_views=True)
