import grpc
import os
import sys

from .Case import Case
from .Commands import Commands

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../generated'))

from Empty_pb2 import Empty
import Project_pb2
import Project_pb2_grpc

class Project:
    """ResInsight project. Not intended to be created separately.

    Automatically created and assigned to Instance.
    """
    def __init__(self, channel):
        self.channel = channel
        self.project = Project_pb2_grpc.ProjectStub(channel)
    
    def open(self, path):
        """Open a new project from the given path
        
        Argument:
            path(string): path to project file
        
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
            path(string): file path to case
        Returns:
            A rips Case object
        """
        return Commands(self.channel).loadCase(path)

