"""
Grid Case Group statistics module
"""

from rips.pdmobject import PdmObject
from rips.view import View
from rips.case import Case

import rips.generated.Commands_pb2 as Cmd


class GridCaseGroup(PdmObject):
    """ResInsight Grid Case Group class

    Operate on a ResInsight case group specified by a Case Group Id integer.

    Attributes:
        group_id (int): Grid Case Group Id corresponding to case group Id in ResInsight project.
    """
    def __init__(self, pdm_object):
        self.group_id = pdm_object.get_value("GroupId")
        PdmObject.__init__(self, pdm_object._pb2_object, pdm_object._channel)

    def create_statistics_case(self):
        """Create a Statistics case in the Grid Case Group

        Returns:
            A new Case
        """
        command_reply = self._execute_command(
            createStatisticsCase=Cmd.CreateStatisticsCaseRequest(
                caseGroupId=self.group_id))
        return Case(self.channel,
                    command_reply.createStatisticsCaseResult.caseId)

    def statistics_cases(self):
        """Get a list of all statistics cases in the Grid Case Group"""
        stat_case_collection = self.children("StatisticsCaseCollection")[0]
        return stat_case_collection.children("Reservoirs")

    def views(self):
        """Get a list of views belonging to a grid case group"""
        pdm_objects = self.descendants("ReservoirView")
        view_list = []
        for pdm_object in pdm_objects:
            view_list.append(View(pdm_object))
        return view_list

    def view(self, view_id):
        """Get a particular view belonging to a case group by providing view id
        Arguments:
            id(int): view id

        Returns: a view object

        """
        views = self.views()
        for view_object in views:
            if view_object.id == view_id:
                return view_object
        return None

    def compute_statistics(self, case_ids=None):
        """ Compute statistics for the given case ids

        Arguments:
            case_ids(list of integers): list of case ids. If this is None all cases in group are included

        """
        if case_ids is None:
            case_ids = []
        return self._execute_command(
            computeCaseGroupStatistics=Cmd.ComputeCaseGroupStatRequest(
                caseIds=case_ids, caseGroupId=self.group_id))
