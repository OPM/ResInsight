import grpc
import os
import sys
from rips.Grid import Grid
from rips.Properties import Properties
from rips.PdmObject import PdmObject
from rips.View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Case_pb2
import Case_pb2_grpc
import Commands_pb2 as Cmd
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
    
    def replace(self, new_egrid_file):
        """Replace the current case grid with a new grid loaded from file
        
        Arguments:
            new_egrid_file (str): path to EGRID file            
        """
        self._execute_command(replaceCase=Cmd.ReplaceCaseRequest(newGridFile=new_egrid_file,
                                                             caseId=self.id))
        self.__init__(self.channel, self.id)

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
        return self.view(self._execute_command(createView=Cmd.CreateViewRequest(caseId=self.id)).createViewResult.viewId)

    def export_snapshots_of_all_views(self, prefix=''):
        """ Export snapshots for all views in the case
        
        Arguments:
            prefix (str): Exported file name prefix
        
        """
        return self._execute_command(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
                                                                         prefix='VIEWS',
                                                                         caseId=self.id))

    def export_well_path_completions(self, time_step, well_path_names, file_split,
                                  compdat_export, include_perforations, include_fishbones,
                                  exclude_main_bore_for_fishbones, combination_mode):
        if (isinstance(well_path_names, str)):
            well_path_names = [well_path_names]
        return self._execute_command(exportWellPathCompletions=Cmd.ExportWellPathCompRequest(caseId=self.id,
                                                                                    timeStep=time_step,
                                                                                    wellPathNames=well_path_names,
                                                                                    fileSplit=file_split,
                                                                                    compdatExport=compdat_export,
                                                                                    includePerforations=include_perforations,
                                                                                    includeFishbones=include_fishbones,
                                                                                    excludeMainBoreForFishbones=exclude_main_bore_for_fishbones,
                                                                                    combinationMode=combination_mode))

    def export_msw(self, well_path):
        return self._execute_command(exportMsw=Cmd.ExportMswRequest(caseId=self.id,
                                                           wellPath=well_path))

    def set_time_step_for_all_views(self, time_step):
        return self._execute_command(setTimeStep=Cmd.SetTimeStepParams(caseId=self.id, viewId=-1, timeStep=time_step))

    def create_multiple_fractures(self, template_id, well_path_names, min_dist_from_well_td,
                                max_fractures_per_well, top_layer, base_layer, spacing, action):
        if isinstance(well_path_names, str):
            well_path_names = [well_path_names]
        return self._execute_command(createMultipleFractures=Cmd.MultipleFracAction(caseId=self.id,
                                                                           templateId=template_id,
                                                                           wellPathNames=well_path_names,
                                                                           minDistFromWellTd=min_dist_from_well_td,
                                                                           maxFracturesPerWell=max_fractures_per_well,
                                                                           topLayer=top_layer,
                                                                           baseLayer=base_layer,
                                                                           spacing=spacing,
                                                                           action=action))
    def create_lgr_for_completion(self, time_step, well_path_names, refinement_i, refinement_j, refinement_k, split_type):
        if isinstance(well_path_names, str):
            well_path_names = [well_path_names]
        return self._execute_command(createLgrForCompletions=Cmd.CreateLgrForCompRequest(caseId=self.id,
                                                                                timeStep=time_step,
                                                                                wellPathNames=well_path_names,
                                                                                refinementI=refinement_i,
                                                                                refinementJ=refinement_j,
                                                                                refinementK=refinement_k,
                                                                                splitType=split_type))
    
    def create_saturation_pressure_plots(self):
        case_ids = [self.id]
        return self._execute_command(createSaturationPressurePlots=Cmd.CreateSatPressPlotRequest(caseIds=case_ids))

    
    def export_flow_characteristics(self, time_steps, injectors, producers, file_name, minimum_communication=0.0, aquifer_cell_threshold=0.1):
        """ Export Flow Characteristics data to text file in CSV format

        Parameter                 | Description                                   | Type
        ------------------------- | --------------------------------------------- | -----
        time_steps                | Time step indices                             | List of Integer  
        injectors                 | Injector names                                | List of Strings  
        producers                 | Producer names                                | List of Strings  
        file_name                 | Export file name                              | Integer          
        minimum_communication     | Minimum Communication, defaults to 0.0        | Integer          
        aquifer_cell_threshold    | Aquifer Cell Threshold, defaults to 0.1       | Integer          

        """
        if isinstance(time_steps, int):
            time_steps = [time_steps]
        if isinstance(injectors, str):
            injectors = [injectors]
        if isinstance(producers, str):
            producers = [producers]
        return self._execute_command(exportFlowCharacteristics=Cmd.ExportFlowInfoRequest(caseId=self.id,
                                                                                timeSteps=time_steps,
                                                                                injectors=injectors,
                                                                                producers=producers,
                                                                                fileName=file_name,
                                                                                minimumCommunication = minimum_communication,
                                                                                aquiferCellThreshold = aquifer_cell_threshold))
