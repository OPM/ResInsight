import grpc
import os
import sys

from .Case import Case
from .Commands import Commands
from .GridCaseGroup import GridCaseGroup
from .PdmObject import PdmObject
from .View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Definitions_pb2 import Empty
import Project_pb2
import Project_pb2_grpc

class Project (PdmObject):
    """ResInsight project. Not intended to be created separately.

    Automatically created and assigned to Instance.
    """
    def __init__(self, channel):
        self.channel = channel
        self.project = Project_pb2_grpc.ProjectStub(channel)
        PdmObject.__init__(self, self.project.GetPdmObject(Empty()), self.channel)
    
    def open(self, path):
        """Open a new project from the given path
        
        Arguments:
            path(str): path to project file
        
        """
        Commands(self.channel).openProject(path)
        return self

    def close(self):
        """Close the current project (and open new blank project)"""
        Commands(self.channel).closeProject()

    def selectedCases(self):
        """Get a list of all cases selected in the project tree

        Returns:
            A list of rips Case objects
        """
        caseInfos = self.project.GetSelectedCases(Empty())
        cases = []
        for caseInfo in caseInfos.data:
            cases.append(Case(self.channel, caseInfo.id))
        return cases

    def cases(self):
        """Get a list of all cases in the project
        
        Returns:
            A list of rips Case objects
        """
        try:
            caseInfos = self.project.GetAllCases(Empty())

            cases = []
            for caseInfo in caseInfos.data:
                cases.append(Case(self.channel, caseInfo.id))
            return cases
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return []
            else:
                print("ERROR: ", e)
                return []

    def case(self, id):
        """Get a specific case from the provided case Id

        Arguments:
            id(int): case id
        Returns:
            A rips Case object
        """
        try:
            case = Case(self.channel, id)
            return case
        except grpc.RpcError as e:
            return None

    def loadCase(self, path):
        """Load a new case from the given file path

        Arguments:
            path(str): file path to case
        Returns:
            A rips Case object
        """
        return Commands(self.channel).loadCase(path)

    def views(self):
        """Get a list of views belonging to a project"""
        pdmObjects = self.descendants("ReservoirView")
        viewList = []
        for pdmObject in pdmObjects:
            viewList.append(View(pdmObject))
        return viewList

    def view(self, id):
        """Get a particular view belonging to a case by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for viewObject in views:
            if viewObject.id == id:
                return viewObject
        return None

    def gridCaseGroups(self):
        """Get a list of all grid case groups in the project"""
        caseGroups = self.descendants("RimIdenticalGridCaseGroup");

        caseGroupList = []
        for pb2Group in caseGroups:
            caseGroupList.append(GridCaseGroup(pb2Group))
        return caseGroupList
    
    def gridCaseGroup(self, groupId):
        """Get a particular grid case group belonging to a project
        Arguments:
            groupId(int): group id
        
        Returns: a grid case group object
        """
        caseGroups = self.gridCaseGroups()
        for caseGroup in caseGroups:
            if caseGroup.groupId == groupId:
                return caseGroup
        return None

    def createGridCaseGroup(self, casePaths):
        """Create a new grid case group from the provided case paths
        Arguments:
            casePaths(list): a list of paths to the cases to be loaded and included in the group
        Returns:
            A new grid case group object
        """
        groupId, groupName = Commands(self.channel).createGridCaseGroup(casePaths)
        return self.gridCaseGroup(groupId)