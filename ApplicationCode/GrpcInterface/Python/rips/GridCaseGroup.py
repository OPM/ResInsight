import grpc
import os
import sys
from rips.PdmObject import PdmObject
from rips.View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import PdmObject_pb2

class GridCaseGroup (PdmObject):
    """ResInsight Grid Case Group class
    
    Operate on a ResInsight case group specified by a Case Group Id integer.

    Attributes:
        id (int): Grid Case Group Id corresponding to case group Id in ResInsight project.
        name (str): Case name
    """
    def __init__(self, pdm_object):
        self.groupId = pdm_object.getValue("GroupId")
        PdmObject.__init__(self, pdm_object.pb2Object, pdm_object.channel)

    def statistics_cases(self):
        """Get a list of all statistics cases in the Grid Case Group"""
        stat_case_collection = self.children("StatisticsCaseCollection")[0]
        return stat_case_collection.children("Reservoirs")
    
    def views(self):
        """Get a list of views belonging to a grid case group"""
        pbm_objects = self.descendants("ReservoirView")
        view_list = []
        for pbm_object in pbm_objects:
            view_list.append(View(pbm_object))
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
