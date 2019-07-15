import grpc
import os
import sys
from .Grid import Grid
from .Properties import Properties
from .PdmObject import PdmObject
from .View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Case_pb2
import Case_pb2_grpc
import PdmObject_pb2

class Case (PdmObject):
    """ResInsight case class
    
    Operate on a ResInsight case specified by a Case Id integer.
    Not meant to be constructed separately but created by one of the following
    methods in Project: loadCase, case, allCases, selectedCases

    Attributes:
        id (int): Case Id corresponding to case Id in ResInsight project.
        name (str): Case name
        groupId (int): Case Group id        
    """
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
        PdmObject.__init__(self, self.stub.GetPdmObject(self.request), self.channel)
  
    def gridCount(self):
        """Get number of grids in the case"""
        try:
            return self.stub.GetGridCount(self.request).count          
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return 0
            print("ERROR: ", e)
            return 0
    
    def grid(self, index):
        """Get Grid of a given index. Returns a rips Grid object
		
		Arguments:
			index (int): The grid index
		
		Returns: Grid object
		"""
        return Grid(index, self)
    
    def grids(self):
        """Get a list of all rips Grid objects in the case"""
        gridList = []
        for i in range(0, self.gridCount()):
            gridList.append(Grid(i, self))
        return gridList

    def cellCount(self, porosityModel='MATRIX_MODEL'):
        """Get a cell count object containing number of active cells and
        total number of cells
        
        Arguments:
            porosityModel (str): String representing an enum.
                must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
        Returns:
            Cell Count object with the following integer attributes:
                active_cell_count: number of active cells
                reservoir_cell_count: total number of reservoir cells
        """
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosityModel)
        return self.stub.GetCellCount(request)

    def cellInfoForActiveCells(self, porosityModel='MATRIX_MODEL'):
        """Get Stream of cell info objects for current case
		
        Arguments:
            porosityModel(str): String representing an enum.
                must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
		
        Returns:
            Stream of cell info objects with the following attributes:
                grid_index(int): grid the cell belongs to
                parent_grid_index(int): parent of the grid the cell belongs to
                coarsening_box_index(int): the coarsening box index
                local_ijk(Vec3i: i(int), j(int), k(int)): local cell index in i, j, k directions.
                parent_ijk(Vec3i: i(int), j(int), k(int)): cell index in parent grid in i, j, k.
		
        """
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosityModel)
        return self.stub.GetCellInfoForActiveCells(request)

    def timeSteps(self):
        """Get a list containing time step strings for all time steps"""
        return self.stub.GetTimeSteps(self.request).dates

    def daysSinceStart(self):
        """Get a list of decimal values representing days since the start of the simulation"""
        return self.stub.GetDaysSinceStart(self.request).day_decimals

    def views(self):
        """Get a list of views belonging to a case"""
        pbmObjects = self.children("ReservoirView")
        viewList = []
        for pbmObject in pbmObjects:
            viewList.append(View(pbmObject))
        return viewList

    def view(self, id):
        """Get a particular view belonging to a case by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for viewObject in views:
            if viewObject.id == id:
                return viewObject
        return None

