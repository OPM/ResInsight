from __future__ import print_function

import grpc

import ResInsightGrid_pb2
import ResInsightGrid_pb2_grpc

MAX_MESSAGE_LENGTH = 32 * 1024 * 1024

class ResInsight:
	def __init__(self, location):	    
		self.channel = grpc.insecure_channel(location, options=[('grpc.max_receive_message_length', MAX_MESSAGE_LENGTH)])
		self.grid = ResInsightGrid_pb2_grpc.GridStub(self.channel)		
		
	def Case(id):
		return ResInsightGrid_pb2.Case(id=id)

	def ResultAddress(result_cat_type, result_name):
		return ResInsightGrid_pb2.EclipseResultAddress(result_cat_type=result_cat_type, result_name=result_name)
		
	def ResultRequest(result_case, result_address, time_step):
		return ResInsightGrid_pb2.EclipseResultRequest(result_case=result_case, result_address=result_address, time_step=time_step)
	
