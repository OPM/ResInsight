import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Empty_pb2 import Empty
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc
from .Case import Case

class Commands:
    """Command executor which can run ResInsight Command File commands nearly verbatim
    
    Documentation Command File Interface:
        https://resinsight.org/docs/commandfile/

    The differences are:
        * Enum values have to be provided as strings. I.e. "ALL" instead of ALL.
        * Booleans have to be specified as correct Python. True instead of true.
    
    """
    def __init__(self, channel):
        self.channel = channel
        self.commands = CmdRpc.CommandsStub(channel)

    def __execute(self, **commandParams):
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
        """Open a project
        
        Arguments:
            path (str): path to project file
        
        
        """
        return self.__execute(openProject=Cmd.FilePathRequest(path=path))

    def closeProject(self):
        """Close the current project (and reopen empty one)"""
        return self.__execute(closeProject=Empty())

    def setStartDir(self, path):
        """Set current start directory
        
        Arguments:
            path (str): path to directory
        
        """
        return self.__execute(setStartDir=Cmd.FilePathRequest(path=path))

    def loadCase(self, path):
        """Load a case
        
        Arguments:
            path (str): path to EGRID file
        
        Returns:
            A Case object
        
        """
        commandReply = self.__execute(loadCase=Cmd.FilePathRequest(path=path))
        assert(commandReply is not None)
        assert(commandReply.HasField("loadCaseResult"))
        return Case(self.channel, commandReply.loadCaseResult.id)

    def replaceCase(self, newGridFile, caseId=0):
        """Replace the given case with a new case loaded from file
        
        Arguments:
            newGridFile (str): path to EGRID file
            caseId (int): case Id to replace
            
        """
        return self.__execute(replaceCase=Cmd.ReplaceCaseRequest(newGridFile=newGridFile,
                                                                 caseId=caseId))
    
    def replaceSourceCases(self, gridListFile, caseGroupId=0):
        """Replace all source cases within a case group
        
        Arguments:
            gridListFile (str): path to file containing a list of cases
            caseGroupId (int): id of the case group to replace
        
        """
        return self.__execute(replaceSourceCases=Cmd.ReplaceSourceCasesRequest(gridListFile=gridListFile,
                                                                               caseGroupId=caseGroupId))

    def createGridCaseGroup(self, casePaths):
        """Create a Grid Case Group from a list of cases

        Arguments:
            casePaths (list): list of file path strings

        Returns:
            A case group id and name
        """
        commandReply = self.__execute(createGridCaseGroup=Cmd.CreateGridCaseGroupRequest(casePaths=casePaths))
        assert(commandReply is not None)
        assert(commandReply.HasField("createGridCaseGroupResult"))
        return (commandReply.createGridCaseGroupResult.groupId, commandReply.createGridCaseGroupResult.groupName)

    ##################
    # Export Commands
    ##################

    def exportMultiCaseSnapshots(self, gridListFile):
        """Export snapshots for a set of cases
        
        Arguments:
            gridListFile (str): Path to a file containing a list of grids to export snapshot for
        
        """
        return self.__execute(exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(gridListFile=gridListFile))

    def exportSnapshots(self, type = 'ALL', prefix=''):
        """ Export snapshots of a given type
        
        Arguments:
            type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
            prefix (str): Exported file name prefix
        
        """
        return self.__execute(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
                                                                       prefix=prefix))

    def exportProperty(self, caseId, timeStep, property, eclipseKeyword=property, undefinedValue=0.0, exportFile=property):
        """ Export an Eclipse property

        Arguments:
            caseId (int): case id
            timeStep (int): time step index
            property (str): property to export
            eclipseKeyword (str): Eclipse keyword used as text in export header. Defaults to the value of property parameter.
            undefinedValue (double):	Value to use for undefined values. Defaults to 0.0
            exportFile (str):	Filename for export. Defaults to the value of property parameter
        """
        return self.__execute(exportProperty=Cmd.ExportPropertyRequest(caseId=caseId,
                                                                     timeStep=timeStep,
                                                                     property=property,
                                                                     eclipseKeyword=eclipseKeyword,
                                                                     undefinedValue=undefinedValue,
                                                                     exportFile=exportFile))

    def exportPropertyInViews(self, caseId, viewNames, undefinedValue):
        if isinstance(viewNames, str):
            viewNames = [viewNames]

        return self.__execute(exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(caseId=caseId,
                                                                                   viewNames=viewNames,
                                                                                   undefinedValue=undefinedValue))

    def exportWellPathCompletions(self, caseId, timeStep, wellPathNames, fileSplit,
                                  compdatExport, includePerforations, includeFishbones,
                                  excludeMainBoreForFishbones, combinationMode):
        if (isinstance(wellPathNames, str)):
            wellPathNames = [wellPathNames]
        return self.__execute(exportWellPathCompletions=Cmd.ExportWellPathCompRequest(caseId=caseId,
                                                                                    timeStep=timeStep,
                                                                                    wellPathNames=wellPathNames,
                                                                                    fileSplit=fileSplit,
                                                                                    compdatExport=compdatExport,
                                                                                    includePerforations=includePerforations,
                                                                                    includeFishbones=includeFishbones,
                                                                                    excludeMainBoreForFishbones=excludeMainBoreForFishbones,
                                                                                    combinationMode=combinationMode))

    def exportSimWellFractureCompletions(self, caseId, viewName, timeStep, simulationWellNames, fileSplit, compdatExport):
        if(isinstance(simulationWellNames, str)):
            simulationWellNames = [simulationWellNames]
        return self.__execute(exportSimWellFractureCompletions=Cmd.ExportSimWellPathFraqRequest(caseId=caseId,
                                                                                              viewName=viewName,
                                                                                              timeStep=timeStep,
                                                                                              simulationWellNames=simulationWellNames,
                                                                                              fileSplit=fileSplit,
                                                                                              compdatExport=compdatExport))

    def exportMsw(self, caseId, wellPath):
        return self.__execute(exportMsw=Cmd.ExportMswRequest(caseId=caseId,
                                                           wellPath=wellPath))

    def exportWellPaths(self, wellPaths=[], mdStepSize=5.0):
        if isinstance(wellPaths, str):
            wellPaths = [wellPaths]
        return self.__execute(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=wellPaths, mdStepSize=mdStepSize))

    def exportVisibleCells(self, caseId, viewName, exportKeyword='FLUXNUM', visibleActiveCellsValue=1, hiddenActiveCellsValue=0, inactiveCellsValue=0):
        return self.__execute(exportVisibleCells=Cmd.ExportVisibleCellsRequest(caseId=caseId,
                                                                             viewName=viewName,
                                                                             exportKeyword=exportKeyword,
                                                                             visibleActiveCellsValue=visibleActiveCellsValue,
                                                                             hiddenActiveCellsValue=hiddenActiveCellsValue,
                                                                             inactiveCellsValue=inactiveCellsValue))
    def setExportFolder(self, type, path, createFolder=False):
        return self.__execute(setExportFolder=Cmd.SetExportFolderRequest(type=type,
                                                                       path=path,
                                                                       createFolder=createFolder))

    def runOctaveScript(self, path, cases):
        caseIds = []
        for case in cases:
            caseIds.append(case.id)
        return self.__execute(runOctaveScript=Cmd.RunOctaveScriptRequest(path=path,
                                                                       caseIds=caseIds))
        
    def setMainWindowSize(self, width, height):
        return self.__execute(setMainWindowSize=Cmd.SetMainWindowSizeParams(width=width, height=height))

    def computeCaseGroupStatistics(self, caseIds):
        if isinstance(caseIds, int):
            caseIds = [caseIds]
        return self.__execute(computeCaseGroupStatistics=Cmd.ComputeCaseGroupStatRequest(caseIds=caseIds))

    def setTimeStep(self, caseId, timeStep):
        return self.__execute(setTimeStep=Cmd.SetTimeStepParams(caseId=caseId, timeStep=timeStep))

    def scaleFractureTemplate(self, id, halfLength, height, dFactor, conductivity):
        return self.__execute(scaleFractureTemplate=Cmd.ScaleFractureTemplateRequest(id=id,
                                                                                   halfLength=halfLength,
                                                                                   height=height,
                                                                                   dFactor=dFactor,
                                                                                   conductivity=conductivity))

    def setFractureContainment(self, id, topLayer, baseLayer):
        return self.__execute(setFractureContainment=Cmd.SetFracContainmentRequest(id=id,
                                                                                 topLayer=topLayer,
                                                                                 baseLayer=baseLayer))

    def createMultipleFractures(self, caseId, templateId, wellPathNames, minDistFromWellTd,
                                maxFracturesPerWell, topLayer, baseLayer, spacing, action):
        if isinstance(wellPathNames, str):
            wellPathNames = [wellPathNames]
        return self.__execute(createMultipleFractures=Cmd.MultipleFracAction(caseId=caseId,
                                                                           templateId=templateId,
                                                                           wellPathNames=wellPathNames,
                                                                           minDistFromWellTd=minDistFromWellTd,
                                                                           maxFracturesPerWell=maxFracturesPerWell,
                                                                           topLayer=topLayer,
                                                                           baseLayer=baseLayer,
                                                                           spacing=spacing,
                                                                           action=action))

    def createLgrForCompletions(self, caseId, timeStep, wellPathNames, refinementI, refinementJ, refinementK, splitType):
        if isinstance(wellPathNames, str):
            wellPathNames = [wellPathNames]
        return self.__execute(createLgrForCompletions=Cmd.CreateLgrForCompRequest(caseId=caseId,
                                                                                timeStep=timeStep,
                                                                                wellPathNames=wellPathNames,
                                                                                refinementI=refinementI,
                                                                                refinementJ=refinementJ,
                                                                                refinementK=refinementK,
                                                                                splitType=splitType))

    def createSaturationPressurePlots(self, caseIds):
        if isinstance(caseIds, int):
            caseIds = [caseIds]
        return self.__execute(createSaturationPressurePlots=Cmd.CreateSatPressPlotRequest(caseIds=caseIds))

    def exportFlowCharacteristics(self, caseId, timeSteps, injectors, producers, fileName, minimumCommunication=0.0, aquiferCellThreshold=0.1):
        if isinstance(timeSteps, int):
            timeSteps = [timeSteps]
        if isinstance(injectors, str):
            injectors = [injectors]
        if isinstance(producers, str):
            producers = [producers]
        return self.__execute(exportFlowCharacteristics=Cmd.ExportFlowInfoRequest(caseId=caseId,
                                                                                timeSteps=timeSteps,
                                                                                injectors=injectors,
                                                                                producers=producers,
                                                                                fileName=fileName,
                                                                                minimumCommunication = minimumCommunication,
                                                                                aquiferCellThreshold = aquiferCellThreshold))
