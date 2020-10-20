"""
Grid Case Group statistics module
"""

from .pdmobject import add_method
from .view import View
from .case import Case

import Commands_pb2
from resinsight_classes import GridCaseGroup


@add_method(GridCaseGroup)
def create_statistics_case(self):
    """Create a Statistics case in the Grid Case Group

    Returns:
        :class:`rips.generated.resinsight_classes.EclipseCase`
    """
    command_reply = self._execute_command(
        createStatisticsCase=Commands_pb2.CreateStatisticsCaseRequest(
            caseGroupId=self.group_id))
    return Case(self.channel,
                command_reply.createStatisticsCaseResult.caseId)


@add_method(GridCaseGroup)
def statistics_cases(self):
    """Get a list of all statistics cases in the Grid Case Group

    Returns: 
        List of :class:`rips.generated.resinsight_classes.EclipseCase`

    """
    stat_case_collection = self.children("StatisticsCaseCollection")[0]
    return stat_case_collection.children("Reservoirs")


@add_method(GridCaseGroup)
def views(self):
    """Get a list of views belonging to a grid case group

    Returns: 
        List of :class:`rips.generated.resinsight_classes.EclipseView`

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
        List of :class:`rips.generated.resinsight_classes.EclipseView`

    """
    views = self.views()
    for view_object in views:
        if view_object.id == view_id:
            return view_object
    return None


@add_method(GridCaseGroup)
def compute_statistics(self, case_ids=None):
    """ Compute statistics for the given case ids

    Arguments:
        case_ids(list of integers): List of case ids. If this is None all cases in group are included

    """
    if case_ids is None:
        case_ids = []
    return self._execute_command(
        computeCaseGroupStatistics=Commands_pb2.ComputeCaseGroupStatRequest(
            caseIds=case_ids, caseGroupId=self.group_id))
