import grpc
import os
import sys
from .PdmObject import PdmObject
from .View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import PdmObject_pb2

class GridCaseGroup (PdmObject):
    """ResInsight Grid Case Group class
    
    Operate on a ResInsight case group specified by a Case Group Id integer.

    Attributes:
        id (int): Grid Case Group Id corresponding to case group Id in ResInsight project.
        name (str): Case name
    """
    def __init__(self, pdmObject):
        self.groupId = pdmObject.getValue("GroupId")
        PdmObject.__init__(self, pdmObject.pb2Object, pdmObject.channel)

    def statisticsCases(self):
        """Get a list of all statistics cases in the Grid Case Group"""
        statCaseCollection = self.children("StatisticsCaseCollection")[0]
        return statCaseCollection.children("Reservoirs")
    
    def views(self):
        """Get a list of views belonging to a grid case group"""
        pbmObjects = self.descendants("ReservoirView")
        viewList = []
        for pbmObject in pbmObjects:
            viewList.append(View(pbmObject))
        return viewList

    def view(self, id):
        """Get a particular view belonging to a case group by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for viewObject in views:
            if viewObject.id == id:
                return viewObject
        return None
