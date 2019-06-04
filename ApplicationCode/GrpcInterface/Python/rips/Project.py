import grpc
import os
import sys

from .Case import Case
from .Commands import Commands

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

from Empty_pb2 import Empty
import Project_pb2
import Project_pb2_grpc

class Project:
    def __init__(self, channel):
        self.channel = channel
        self.project = Project_pb2_grpc.ProjectStub(channel)
    
    def open(self, path):
        Commands(self.channel).openProject(path)
        return self

    def close(self):
        Commands(self.channel).closeProject()

    def selectedCases(self):
        caseInfos = self.project.GetSelectedCases(Empty())
        cases = []
        for caseInfo in caseInfos.data:
            cases.append(Case(self.channel, caseInfo.id))
        return cases

    def cases(self):
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
        try:
            case = Case(self.channel, id)
            return case
        except grpc.RpcError as e:
            return None

    def loadCase(self, path):
        return Commands(self.channel).loadCase(path)

