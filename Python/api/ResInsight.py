from __future__ import print_function

import grpc
import os
import sys
import socket

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

import Empty_pb2
import CaseInfo_pb2
import CaseInfo_pb2_grpc
import Commands_pb2
import Commands_pb2_grpc
import GridInfo_pb2
import GridInfo_pb2_grpc
import ProjectInfo_pb2
import ProjectInfo_pb2_grpc
import ResInfo_pb2
import ResInfo_pb2_grpc
import RiaVersionInfo

MAX_MESSAGE_LENGTH = 128 * 1024 * 1024

class ResInfo:
	def __init__(self, channel):
		self.resInfo      = ResInfo_pb2_grpc.ResInfoStub(channel)
	def versionMessage(self):
		return self.resInfo.GetVersion(Empty_pb2.Empty())
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

	def loadCase(self, path):
		return self.execute(Commands_pb2.CommandParams(loadCase=Commands_pb2.FilePathRequest(path=path)))
		
	def closeProject(self):
		return self.execute(Commands_pb2.CommandParams(closeProject=Empty_pb2.Empty()))

class GridInfo:
	def __init__(self, channel):
		self.gridInfo = GridInfo_pb2_grpc.GridInfoStub(channel)		
	
	def getGridCount(self, caseId=0):
		return self.gridInfo.GetGridCount(CaseInfo_pb2.Case(id=caseId)).count
		
	def getGridDimensions(self, caseId=0):
		return self.gridInfo.GetGridDimensions(CaseInfo_pb2.Case(id=caseId)).dimensions
		
	def streamActiveCellInfo(self, caseId=0):
		return self.gridInfo.StreamActiveCellInfo(CaseInfo_pb2.Case(id=caseId))		
		
class ProjectInfo:
	def __init__(self, channel):
		self.projectInfo = ProjectInfo_pb2_grpc.ProjectInfoStub(channel)
		
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
		location = "localhost:" + str(port)
		self.channel = grpc.insecure_channel(location, options=[('grpc.max_receive_message_length', MAX_MESSAGE_LENGTH)])

		# Main version check package
		self.resInfo     = ResInfo(self.channel)
		try:
			majorVersionOk = self.resInfo.majorVersion() == RiaVersionInfo.RESINSIGHT_MAJOR_VERSION
			minorVersionOk = self.resInfo.minorVersion() == RiaVersionInfo.RESINSIGHT_MINOR_VERSION
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
	