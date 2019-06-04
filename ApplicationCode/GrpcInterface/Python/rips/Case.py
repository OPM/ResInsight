import grpc
import os
import sys
from .Grid import Grid
from .Properties import Properties

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

import Case_pb2
import Case_pb2_grpc

class Case:
    def __init__(self, channel, id):
        self.channel = channel
        self.stub = Case_pb2_grpc.CaseStub(channel)		
        self.id = id
        info = self.stub.GetCaseInfo(Case_pb2.CaseRequest(id=self.id))
        self.name    = info.name
        self.groupId = info.group_id
        self.type    = info.type
        self.properties = Properties(self)
        self.request = Case_pb2.CaseRequest(id=self.id)
  
    def gridCount(self):
        try:
            return self.stub.GetGridCount(self.request).count          
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return 0
            print("ERROR: ", e)
            return 0
    
    def grid(self, index):
        return Grid(index, self)

    def grids(self):
        gridList = []
        for i in range(0, self.gridCount()):
            gridList.append(Grid(i, self))
        return gridList

    def cellCount(self, porosityModel='MATRIX_MODEL'):
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosityModel)
        return self.stub.GetCellCount(request)

    def cellInfoForActiveCells(self, porosityModel='MATRIX_MODEL'):
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosityModel)
        return self.stub.GetCellInfoForActiveCells(request)

    def timeSteps(self):
        return self.stub.GetTimeSteps(self.request)
        