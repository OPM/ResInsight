import grpc
import os
import sys
from .Grid import Grid
from .Properties import Properties

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../generated'))

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
  
    # Get number of grids in the case
    def gridCount(self):
        try:
            return self.stub.GetGridCount(self.request).count          
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return 0
            print("ERROR: ", e)
            return 0
    
    # Get Grid of a given index. Returns a rips Grid object
    def grid(self, index):
        return Grid(index, self)

    # Get a list of all rips Grid objects in the case
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
        return self.stub.GetTimeSteps(self.request).dates

    def daysSinceStart(self):
        return self.stub.GetDaysSinceStart(self.request).day_decimals
        