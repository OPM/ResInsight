import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Case_pb2
import Grid_pb2
import Grid_pb2_grpc

class Grid:
    """Grid Information. Not meant to be constructed separately
    
    Create Grid objects using mathods on Case: Grid() and Grids()
    """
    def __init__(self, index, case, channel):
        self.__channel = channel
        self.__stub = Grid_pb2_grpc.GridStub(self.__channel)

        self.case = case
        self.index = index

    def dimensions(self):
        """The dimensions in i, j, k direction
        
        Returns:
            Vec3i: class with integer attributes i, j, k representing the extent in all three dimensions.
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)
        return self.__stub.GetDimensions(Grid_pb2.GridRequest(case_request = case_request, grid_index = self.index)).dimensions

