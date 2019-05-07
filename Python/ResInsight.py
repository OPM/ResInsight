from __future__ import print_function

import grpc

import CaseInfo_pb2
import CaseInfo_pb2_grpc
import GridInfo_pb2
import GridInfo_pb2_grpc
import ProjectInfo_pb2
import ProjectInfo_pb2_grpc

MAX_MESSAGE_LENGTH = 32 * 1024 * 1024

class ResInsight:
	def __init__(self, location):	    
		self.channel = grpc.insecure_channel(location, options=[('grpc.max_receive_message_length', MAX_MESSAGE_LENGTH)])
		self.GridInfo = GridInfo_pb2_grpc.GridInfoStub(self.channel)		
		self.ProjectInfo = ProjectInfo_pb2_grpc.ProjectInfoStub(self.channel)
		
	def Case(id):
		return CaseInfo_pb2.Case(id=id)
	
	def Empty():
		return CaseInfo_pb2.Empty();
	