import grpc
import os
import sys

from Empty_pb2 import Empty
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc
from .Case import Case

class Commands:
    def __init__(self, channel):
        self.channel = channel
        self.commands = CmdRpc.CommandsStub(channel)

    def execute(self, **commandParams):
        try:
            return self.commands.Execute(Cmd.CommandParams(**commandParams))
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found", commandParams.keys())
            else:
                print("Other error", e)

    ########################
    # Case Control Commands
    ########################

    def openProject(self, path):
        return self.execute(openProject=Cmd.FilePathRequest(path=path))

    def closeProject(self):
        return self.execute(closeProject=Empty())

    def setStartDir(self, path):
        return self.execute(setStartDir=Cmd.FilePathRequest(path=path))

    def loadCase(self, path):
        commandReply = self.execute(loadCase=Cmd.FilePathRequest(path=path))
        assert commandReply.HasField("loadCaseResult")
        return Case(self.channel, commandReply.loadCaseResult.id)

    def replaceCase(self, path, caseId=0):
        return self.execute(replaceCase=Cmd.ReplaceCaseRequest(newGridFile=path,
                                                               caseId=caseId))
    
    def replaceSourceCases(self, gridListFile, caseGroupId=0):
        return self.execute(replaceSourceCases=Cmd.ReplaceSourceCasesRequest(gridListFile=gridListFile,
                                                                             caseGroupId=caseGroupId))
    ##################
    # Export Commands
    ##################

    def exportMultiCaseSnapshots(self, gridListFile):
        return self.execute(exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(gridListFile=gridListFile))

    def exportSnapshots(self, type = 'ALL', prefix=''):
        return self.execute(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
                                                                       prefix=prefix))

    def exportProperty(self, caseId, timeStep, property, eclipseKeyword=property, undefinedValue=0.0, exportFile=property):
        return self.execute(exportProperty=Cmd.ExportPropertyRequest(caseId=caseId,
                                                                     timeStep=timeStep,
                                                                     property=property,
                                                                     eclipseKeyword=eclipseKeyword,
                                                                     undefinedValue=undefinedValue,
                                                                     exportFile=exportFile))

    def exportPropertyInViews(self, caseId, viewNames, undefinedValue):
        if isinstance(viewNames, str):
            viewNames = [viewNames]

        return self.execute(exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(caseId=caseId,
                                                                                   viewNames=viewNames,
                                                                                   undefinedValue=undefinedValue))

    def exportWellPaths(self, wellPaths=[], mdStepSize=5.0):
        if isinstance(wellPaths, str):
            wellPaths = [wellpaths]
        return self.execute(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=wellPaths, mdStepSize=mdStepSize))

    def exportVisibleCells(self, caseId, viewName, exportKeyword='FLUXNUM', visibleActiveCellsValue=1, hiddenActiveCellsValue=0, inactiveCellsValue=0):
        return self.execute(exportVisibleCells=Cmd.ExportVisibleCellsRequest(caseId=caseId,
                                                                             viewName=viewName,
                                                                             exportKeyword=exportKeyword,
                                                                             visibleActiveCellsValue=visibleActiveCellsValue,
                                                                             hiddenActiveCellsValue=hiddenActiveCellsValue,
                                                                             inactiveCellsValue=inactiveCellsValue))
    def setExportFolder(self, type, path, createFolder=False):
        return self.execute(setExportFolder=Cmd.SetExportFolderRequest(type=type,
                                                                       path=path,
                                                                       createFolder=createFolder))

    def runOctaveScript(self, path, cases):
        caseIds = []
        for case in cases:
            caseIds.append(case.id)
        return self.execute(runOctaveScript=Cmd.RunOctaveScriptRequest(path=path,
                                                                       caseIds=caseIds))
        
    def setMainWindowSize(self, width, height):
        return self.execute(setMainWindowSize=Cmd.SetMainWindowSizeParams(width=width, height=height))

    def setTimeStep(self, caseId, timeStep):
        return self.execute(setTimeStep=Cmd.SetTimeStepParams(caseId=caseId, timeStep=timeStep))
    

