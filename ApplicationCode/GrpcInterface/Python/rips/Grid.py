import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Grid_pb2
import Grid_pb2_grpc

class Grid:
    """Grid Information. Not meant to be constructed separately
    
    Create Grid objects using mathods on Case: Grid() and Grids()
    """
    def __init__(self, index, case):
        self.case = case
        self.index   = index
        self.stub = Grid_pb2_grpc.GridStub(self.case.channel)

    def dimensions(self):
        """The dimensions in i, j, k direction
        
        Returns:
            Vec3i: class with integer attributes i, j, k representing the extent in all three dimensions.
        """
        return self.stub.GetDimensions(Grid_pb2.GridRequest(case_request = self.case.request, grid_index = self.index)).dimensions

