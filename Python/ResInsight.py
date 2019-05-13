from __future__ import print_function

import grpc
import os

import CaseInfo_pb2
import CaseInfo_pb2_grpc
import Commands_pb2
import Commands_pb2_grpc
import GridInfo_pb2
import GridInfo_pb2_grpc
import ProjectInfo_pb2
import ProjectInfo_pb2_grpc

MAX_MESSAGE_LENGTH = 128 * 1024 * 1024

class CommandExecutor:
	def __init__(self, channel):
		self.commands      = Commands_pb2_grpc.CommandsStub(channel)
		
	def execute(self, commandParams):
		try:
			self.commands.Execute(commandParams)	
		except grpc.RpcError as e:
			if e.code() == grpc.StatusCode.NOT_FOUND:
				print("Command not found")
			else:
				print("Other error")

	def setTimeStep(self, caseId, timeStep):
		return self.execute(Commands_pb2.CommandParams(setTimeStep=Commands_pb2.SetTimeStepParams(caseId=caseId, timeStep=timeStep)))
		
	def setMainWindowSize(self, width, height):
		return self.execute(Commands_pb2.CommandParams(setMainWindowSize=Commands_pb2.SetMainWindowSizeParams(width=width, height=height)))

	def openProject(self, path):
		return self.execute(Commands_pb2.CommandParams(openProject=Commands_pb2.FilePathRequest(path=path)))
		
	def closeProject(self):
		return self.execute(Commands_pb2.CommandParams(closeProject=CaseInfo_pb2.Empty()))

class GridInfo:
	def __init__(self, channel):
		self.gridInfo = GridInfo_pb2_grpc.GridInfoStub(channel)		
	
	def getGridCount(self, caseId=0):
		return self.gridInfo.GetGridCount(CaseInfo_pb2.Case(id=caseId)).count
		
	def getAllGridDimensions(self, caseId=0):
		return self.gridInfo.GetAllGridDimensions(CaseInfo_pb2.Case(id=caseId)).grid_dimensions
		
	def getActiveCellInfoArray(self, caseId=0):
		return self.gridInfo.GetActiveCellInfoArray(CaseInfo_pb2.Case(id=caseId)).data

	def streamActiveCellInfo(self, caseId=0):
		return self.gridInfo.StreamActiveCellInfo(CaseInfo_pb2.Case(id=caseId))

	def streamActiveCellInfoChunks(self, caseId=0):
		return self.gridInfo.StreamActiveCellInfoChunks(CaseInfo_pb2.Case(id=caseId))
		
class ProjectInfo:
	def __init__(self, channel):
		self.projectInfo = ProjectInfo_pb2_grpc.ProjectInfoStub(channel)
		
class Instance:
	def __init__(self, port=None):
		if port is None:
			port = os.environ.get('RESINSIGHT_GRPC_PORT')
		else:
			port = str(port)
		if port is None:
			port = str(50051)
		location = "localhost:" + port
		self.channel = grpc.insecure_channel(location, options=[('grpc.max_receive_message_length', MAX_MESSAGE_LENGTH)])
		
		# Service packages
		self.commands    = CommandExecutor(self.channel)
		self.gridInfo    = GridInfo(self.channel)
		self.projectInfo = ProjectInfo(self.channel)

