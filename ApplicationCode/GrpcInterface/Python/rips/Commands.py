import grpc
import os
import sys

import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc

class Commands:
    def __init__(self, channel):
        self.commands = CmdRpc.CommandsStub(channel)
        
    def execute(self, commandParams):
        try:
            return self.commands.Execute(commandParams)	
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error")

    def setTimeStep(self, caseId, timeStep):
        return self.execute(Cmd.CommandParams(setTimeStep=Cmd.SetTimeStepParams(caseId=caseId, timeStep=timeStep)))
        
    def setMainWindowSize(self, width, height):
        return self.execute(Cmd.CommandParams(setMainWindowSize=Cmd.SetMainWindowSizeParams(width=width, height=height)))

    def openProject(self, path):
        return self.execute(Cmd.CommandParams(openProject=Cmd.FilePathRequest(path=path)))

    def loadCase(self, path):
        commandReply = self.execute(Cmd.CommandParams(loadCase=Cmd.FilePathRequest(path=path)))
        assert commandReply.HasField("loadCaseResult")
        return commandReply.loadCaseResult.id
        
    def closeProject(self):
        return self.execute(Cmd.CommandParams(closeProject=Empty()))

    def exportWellPaths(self, wellPaths=[], mdStepSize=5.0):
        if isinstance(wellPaths, str):
            wellPathArray = [str]
        elif isinstance(wellPaths, list):
            wellPathArray = wellPaths
        return self.execute(Cmd.CommandParams(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=wellPathArray, mdStepSize=mdStepSize)))
