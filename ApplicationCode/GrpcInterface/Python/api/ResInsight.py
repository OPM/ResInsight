from __future__ import print_function

import grpc
import os
import sys
import socket
import logging


sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

from Empty_pb2 import Empty
import CaseInfo_pb2
import CaseInfo_pb2_grpc
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc
import GridInfo_pb2
import GridInfo_pb2_grpc
import ProjectInfo_pb2
import ProjectInfo_pb2_grpc
import ResInfo_pb2
import ResInfo_pb2_grpc
import Properties_pb2
import Properties_pb2_grpc
import RiaVersionInfo

class ResInfo:
    def __init__(self, channel):
        self.resInfo      = ResInfo_pb2_grpc.ResInfoStub(channel)
    def versionMessage(self):
        return self.resInfo.GetVersion(Empty())
    def majorVersion(self):
        return self.versionMessage().major_version
    def minorVersion(self):
        return self.versionMessage().minor_version
    def patchVersion(self):
        return self.versionMessage().patch_version
    def versionString(self):
        return str(self.majorVersion()) + "." + str(self.minorVersion()) + "." + str(self.patchVersion())

class CommandExecutor:
    def __init__(self, channel):
        self.commands      = CmdRpc.CommandsStub(channel)
        
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

class GridInfo:
    def __init__(self, channel):
        self.gridInfo = GridInfo_pb2_grpc.GridInfoStub(channel)		
    
    def gridCount(self, caseId=0):
        return self.gridInfo.GetGridCount(CaseInfo_pb2.Case(id=caseId)).count
        
    def gridDimensions(self, caseId=0):
        return self.gridInfo.GetGridDimensions(CaseInfo_pb2.Case(id=caseId)).dimensions
        
    def cellCount(self, caseId=0, porosityModel='MATRIX_MODEL'):
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request =  GridInfo_pb2.CellInfoRequest(case_id=caseId,
                                                porosity_model=porosityModel)
        return self.gridInfo.GetCellCount(request)

    def cellInfoForActiveCells(self, caseId=0, porosityModel='MATRIX_MODEL'):
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request =  GridInfo_pb2.CellInfoRequest(case_id=caseId,
                                                porosity_model=porosityModel)
        return self.gridInfo.GetCellInfoForActiveCells(request)

    def timeSteps(self, caseId=0):
        return self.gridInfo.GetTimeSteps(CaseInfo_pb2.Case(id=caseId))
       
class ProjectInfo:
    def __init__(self, channel):
        self.projectInfo = ProjectInfo_pb2_grpc.ProjectInfoStub(channel)
    def selectedCases(self):
        selected = self.projectInfo.SelectedCases(Empty())
        if selected is not None:
            return selected.case_info
        else:
            return None
    def allCases(self):
        cases = self.projectInfo.AllCases(Empty())
        if cases is not None:
            return cases.case_info
        else:
            return None

class Properties:
    def __init__(self, channel):
        self.properties = Properties_pb2_grpc.PropertiesStub(channel)
    
    def generateResultRequestArrayIterator(self, values_iterator, parameters):
        chunk = Properties_pb2.ResultRequestChunk()
        chunk.params.CopyFrom(parameters)
        yield chunk

        for values in values_iterator:
            valmsg = Properties_pb2.ResultArray(values = values)
            chunk.values.CopyFrom(valmsg)
            yield chunk

    def generateResultRequestChunks(self, array, parameters):
         # Each double is 8 bytes. A good chunk size is 64KiB = 65536B
         # Meaning ideal number of doubles would be 8192.
         # However we need overhead space, so if we choose 8160 in chunk size
         # We have 256B left for overhead which should be plenty
        chunkSize = 8000
        index = -1
        while index < len(array):
            chunk = Properties_pb2.ResultRequestChunk()
            if index is -1:
                chunk.params.CopyFrom(parameters)
                index += 1;
            else:
                actualChunkSize = min(len(array) - index + 1, chunkSize)
                chunk.values.CopyFrom(Properties_pb2.ResultArray(values = array[index:index+actualChunkSize]))
                index += actualChunkSize

            yield chunk
        # Final empty message to signal completion
        chunk = Properties_pb2.ResultRequestChunk()
        yield chunk

    def availableProperties(self, caseId, propertyType, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertiesRequest (request_case = CaseInfo_pb2.Case(id=caseId),
                                                    property_type = propertyTypeEnum,
                                                    porosity_model = porosityModelEnum)
        return self.properties.GetAvailableProperties(request).property_names
    def activeCellResults(self, caseId, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.ResultRequest(request_case   = CaseInfo_pb2.Case(id=caseId),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               porosity_model = porosityModelEnum)
        for chunk in self.properties.GetActiveCellResults(request):
            yield chunk

    def gridCellResults(self, caseId, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.ResultRequest(request_case   = CaseInfo_pb2.Case(id=caseId),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               grid_index     = gridIndex,
                                               porosity_model = porosityModelEnum)
        return self.properties.GetGridResults(request)

    def setActiveCellResultsAsync(self, values_iterator, caseId, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.ResultRequest(request_case   = CaseInfo_pb2.Case(id=caseId),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               grid_index     = gridIndex,
                                               porosity_model = porosityModelEnum)
        try:
            reply_iterator = self.generateResultRequestArrayIterator(values_iterator, request)
            self.properties.SetActiveCellResults(reply_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)

    def setActiveCellResults(self, values, caseId, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.ResultRequest(request_case   = CaseInfo_pb2.Case(id=caseId),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               grid_index     = gridIndex,
                                               porosity_model = porosityModelEnum)
        try:
            request_iterator = self.generateResultRequestChunks(values, request)
            self.properties.SetActiveCellResults(request_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)
    def setGridResults(self, values, caseId, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = GridInfo_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.ResultRequest(request_case   = CaseInfo_pb2.Case(id=caseId),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               grid_index     = gridIndex,
                                               porosity_model = porosityModelEnum)
        try:
            request_iterator = self.generateResultRequestArrays(values, request)
            self.properties.SetGridResults(request_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)

class Instance:
    @staticmethod
    def is_port_in_use(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.2)
            return s.connect_ex(('localhost', port)) == 0
    
    @staticmethod
    def launch():
        port = 50051
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            port = int(portEnv)
        
        resInsightExecutable = os.environ.get('RESINSIGHT_EXECUTABLE')
        if resInsightExecutable is None:
            print('Error: Could not launch any ResInsight instances because RESINSIGHT_EXECUTABLE is not set')
            return None
        
        while Instance.is_port_in_use(port):
            port += 1

        print('Port ' + str(port))
        print('Trying to launch', resInsightExecutable)
        pid = os.spawnl(os.P_NOWAIT, resInsightExecutable, " --grpcserver " + str(port))
        print(pid)
        return Instance(port)
    
    @staticmethod
    def find(startPort = 50051, endPort = 50071):
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            startPort = int(portEnv)		
            endPort   = startPort + 20 
        
        for tryPort in range(startPort, endPort):
            if Instance.is_port_in_use(tryPort):
                return Instance(tryPort)
                
        print('Error: Could not find any ResInsight instances responding between ports ' + str(startPort) + ' and ' + str(endPort))
        return None

    def __init__(self, port = 50051):
        logging.basicConfig()
        location = "localhost:" + str(port)
        self.channel = grpc.insecure_channel(location)

        # Main version check package
        self.resInfo     = ResInfo(self.channel)
        try:
            majorVersionOk = self.resInfo.majorVersion() == int(RiaVersionInfo.RESINSIGHT_MAJOR_VERSION)
            minorVersionOk = self.resInfo.minorVersion() == int(RiaVersionInfo.RESINSIGHT_MINOR_VERSION)
            if not (majorVersionOk and minorVersionOk):
                raise Exception('Version of ResInsight does not match version of Python API')
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                print('Info: Could not find any instances at port ' + str(port))
        except Exception as e:
            print('Error:', e)
                
        # Service packages
        self.commands    = CommandExecutor(self.channel)
        self.gridInfo    = GridInfo(self.channel)
        self.projectInfo = ProjectInfo(self.channel)
        self.properties  = Properties(self.channel)
    