import grpc
import os
import sys
from rips.Commands import Commands
from rips.Grid import Grid
from rips.Properties import Properties
from rips.PdmObject import PdmObject
from rips.View import View

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
        self.group_id = info.group_id
        self.type    = info.type
        self.properties = Properties(self)
        self.request = Case_pb2.CaseRequest(id=self.id)
        PdmObject.__init__(self, self.stub.GetPdmObject(self.request), self.channel)
  
    def grid_count(self):
        """Get number of grids in the case"""
        try:
            return self.stub.GetGridCount(self.request).count          
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return 0
            print("ERROR: ", e)
            return 0
    
    def grid_path(self):
        return self.get_value("CaseFileName")

    def grid(self, index):
        """Get Grid of a given index. Returns a rips Grid object
		
		Arguments:
			index (int): The grid index
		
		Returns: Grid object
		"""
        return Grid(index, self)
    
    def grids(self):
        """Get a list of all rips Grid objects in the case"""
        grid_list = []
        for i in range(0, self.grid_count()):
            grid_list.append(Grid(i, self))
        return grid_list

    def cell_count(self, porosity_model='MATRIX_MODEL'):
        """Get a cell count object containing number of active cells and
        total number of cells
        
        Arguments:
            porosity_model (str): String representing an enum.
                must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
        Returns:
            Cell Count object with the following integer attributes:
                active_cell_count: number of active cells
                reservoir_cell_count: total number of reservoir cells
        """
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosity_model_enum)
        return self.stub.GetCellCount(request)

    def cell_info_for_active_cells_async(self, porosity_model='MATRIX_MODEL'):
        """Get Stream of cell info objects for current case
		
        Arguments:
            porosity_model(str): String representing an enum.
                must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
		
        Returns:
            Stream of **CellInfo** objects
        
        See cell_info_for_active_cells() for detalis on the **CellInfo** class.
        """
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request =  Case_pb2.CellInfoRequest(case_request=self.request,
                                            porosity_model=porosity_model_enum)
        return self.stub.GetCellInfoForActiveCells(request)

    def cell_info_for_active_cells(self, porosity_model='MATRIX_MODEL'):
        """Get list of cell info objects for current case
		
        Arguments:
            porosity_model(str): String representing an enum.
                must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
		
        Returns:
            List of **CellInfo** objects

        ### CellInfo class description

        Parameter                 | Description                                   | Type
        ------------------------- | --------------------------------------------- | -----
        grid_index                | Index to grid                                 | Integer          
        parent_grid_index         | Index to parent grid                          | Integer          
        coarsening_box_index      | Index to coarsening box                       | Integer          
        local_ijk                 | Cell index in IJK directions of local grid    | Vec3i
        parent_ijk                | Cell index in IJK directions of parent grid   | Vec3i
		
        ### Vec3i class description

        Parameter        | Description                                  | Type
        ---------------- | -------------------------------------------- | -----
        i                | I grid index                                 | Integer          
        j                | J grid index                                 | Integer          
        k                | K grid index                                 | Integer          

        """
        active_cell_info_chunks = self.cell_info_for_active_cells_async()
        received_active_cells = []
        for active_cell_chunk in active_cell_info_chunks:
            for active_cell in active_cell_chunk.data:
                received_active_cells.append(active_cell)
        return received_active_cells

    def time_steps(self):
        """Get a list containing all time steps

        The time steps are defined by the class **TimeStepDate** : 
        
        Type      | Name      
        --------- | ----------
        int       | year      
        int       | month     
        int       | day       
        int       | hour      
        int       | minute    
        int       | second    

        
        """
        return self.stub.GetTimeSteps(self.request).dates

    def days_since_start(self):
        """Get a list of decimal values representing days since the start of the simulation"""
        return self.stub.GetDaysSinceStart(self.request).day_decimals

    def views(self):
        """Get a list of views belonging to a case"""
        pdm_objects = self.children("ReservoirViews")
        view_list = []
        for pdm_object in pdm_objects:
            view_list.append(View(pdm_object))
        return view_list

    def view(self, id):
        """Get a particular view belonging to a case by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for view_object in views:
            if view_object.id == id:
                return view_object
        return None

    def create_view(self):
        """Create a new view in the current case"""
        view_id =  Commands(self.channel).create_view(self.id)
        return self.view(view_id)

