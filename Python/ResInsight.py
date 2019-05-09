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

MAX_MESSAGE_LENGTH = 32 * 1024 * 1024

class CommandExecutor:
	def __init__(self, channel, commandParams):
		self.commands      = Commands_pb2_grpc.CommandsStub(channel)
		self.commandParams = commandParams
	def execute(self):
		try:
			self.commands.Execute(self.commandParams)	
		except grpc.RpcError as e:
			if e.code() == grpc.StatusCode.NOT_FOUND:
				print("Command not found")
			else:
				print("Other error")


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
		self.GridCount = GridInfo_pb2_grpc.GridCountStub(self.channel)		
		self.AllGridDimensions = GridInfo_pb2_grpc.AllGridDimensionsStub(self.channel)		
		self.ProjectInfo = ProjectInfo_pb2_grpc.ProjectInfoStub(self.channel)
		self.Commands = Commands_pb2_grpc.CommandsStub(self.channel)
		
	def Case(id):
		return CaseInfo_pb2.Case(id=id)
	
	def Empty():
		return CaseInfo_pb2.Empty();
	
	def setTimeStep(self, caseId, timeStep):
		return CommandExecutor(self.channel, Commands_pb2.CommandParams(setTimeStep=Commands_pb2.SetTimeStepParams(caseId=caseId, timeStep=timeStep))).execute()
		
	def setMainWindowSize(self, width, height):
		return CommandExecutor(self.channel, Commands_pb2.CommandParams(setMainWindowSize=Commands_pb2.SetMainWindowSizeParams(width=width, height=height))).execute()