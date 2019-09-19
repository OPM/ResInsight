import grpc
import os
import sys
from rips.PdmObject import PdmObject
from rips.View import View
from rips.Case import Case

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import PdmObject_pb2

class GridCaseGroup (PdmObject):
    """ResInsight Grid Case Group class
    
    Operate on a ResInsight case group specified by a Case Group Id integer.

    Attributes:
        group_id (int): Grid Case Group Id corresponding to case group Id in ResInsight project.
    """
    def __init__(self, pdm_object):
        self.group_id = pdm_object.get_value("GroupId")
        PdmObject.__init__(self, pdm_object.pb2Object, pdm_object.channel)

    def create_statistics_case(self):
        """Create a Statistics case in the Grid Case Group

        Returns:
            A new Case
        """
        commandReply = self.__executeCmd(createStatisticsCase=Cmd.CreateStatisticsCaseRequest(caseGroupId=case_group_id))
        return Case(self.channel, commandReply.createStatisticsCaseResult.caseId)

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

    def view(self, id):
        """Get a particular view belonging to a case group by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for view_object in views:
            if view_object.id == id:
                return view_object
        return None
