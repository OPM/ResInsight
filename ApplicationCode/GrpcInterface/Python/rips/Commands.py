import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Definitions_pb2 import Empty
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc
import rips.Case

class Commands:
    """Command executor which can run ResInsight Command File commands nearly verbatim. See    
    [ Command File Interface ]({{< ref "commandfile.md" >}})

    The differences are:

    - Enum values have to be provided as strings. I.e. "ALL" instead of ALL.
    - Booleans have to be specified as correct Python. True instead of true.
    
    """
    def __init__(self, channel):
        self.channel = channel
        self.commands = CmdRpc.CommandsStub(channel)

    def __execute(self, **command_params):
        return self.commands.Execute(Cmd.CommandParams(**command_params))
    
    ########################
    # Case Control Commands
    ########################

    def open_project(self, path):
        """Open a project
        
        Arguments:
            path (str): path to project file
        
        
        """
        return self.__execute(openProject=Cmd.FilePathRequest(path=path))

    def close_project(self):
        """Close the current project (and reopen empty one)"""
        return self.__execute(closeProject=Empty())

    def set_start_dir(self, path):
        """Set current start directory
        
        Arguments:
            path (str): path to directory
        
        """
        return self.__execute(setStartDir=Cmd.FilePathRequest(path=path))

    def load_case(self, path):
        """Load a case
        
        Arguments:
            path (str): path to EGRID file
        
        Returns:
            A Case object
        
        """
        command_reply = self.__execute(loadCase=Cmd.FilePathRequest(path=path))
        return rips.Case(self.channel, command_reply.loadCaseResult.id)

    def replace_case(self, new_grid_file, case_id=0):
        """Replace the given case with a new case loaded from file
        
        Arguments:
            new_grid_file (str): path to EGRID file
            case_id (int): case Id to replace
            
        """
        return self.__execute(replaceCase=Cmd.ReplaceCaseRequest(newGridFile=new_grid_file,
                                                                 caseId=case_id))
    
    def replace_source_cases(self, grid_list_file, case_group_id=0):
        """Replace all source cases within a case group
        
        Arguments:
            grid_list_file (str): path to file containing a list of cases
            case_group_id (int): id of the case group to replace
        
        """
        return self.__execute(replaceSourceCases=Cmd.ReplaceSourceCasesRequest(gridListFile=grid_list_file,
                                                                               caseGroupId=case_group_id))

    def create_grid_case_group(self, case_paths):
        """Create a Grid Case Group from a list of cases

        Arguments:
            case_paths (list): list of file path strings

        Returns:
            A case group id and name
        """
        commandReply = self.__execute(createGridCaseGroup=Cmd.CreateGridCaseGroupRequest(casePaths=case_paths))
        return (commandReply.createGridCaseGroupResult.groupId, commandReply.createGridCaseGroupResult.groupName)

    def create_statistics_case(self, case_group_id):
        """Create a Statistics case in a Grid Case Group

        Arguments:
            case_group_id (int): id of the case group

        Returns:
            A case id for the new case
        """
        commandReply = self.__execute(createStatisticsCase=Cmd.CreateStatisticsCaseRequest(caseGroupId=case_group_id))
        return commandReply.createStatisticsCaseResult.caseId

    ##################
    # Export Commands
    ##################

    def export_multi_case_snapshots(self, grid_list_file):
        """Export snapshots for a set of cases
        
        Arguments:
            grid_list_file (str): Path to a file containing a list of grids to export snapshot for
        
        """
        return self.__execute(exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(gridListFile=grid_list_file))

    def export_snapshots(self, type = 'ALL', prefix='', case_id = -1):
        """ Export snapshots of a given type
        
        Arguments:
            type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
            prefix (str): Exported file name prefix
            case_id (int): the case Id to export for. The default of -1 will export all cases
        
        """
        return self.__execute(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
                                                                         prefix=prefix,
                                                                         caseId=case_id))

    def export_property(self, case_id, time_step, property, eclipse_keyword=property, undefined_value=0.0, export_file=property):
        """ Export an Eclipse property

        Arguments:
            case_id (int): case id
            time_step (int): time step index
            property (str): property to export
            eclipse_keyword (str): Eclipse keyword used as text in export header. Defaults to the value of property parameter.
            undefined_value (double):	Value to use for undefined values. Defaults to 0.0
            export_file (str):	File name for export. Defaults to the value of property parameter
        """
        return self.__execute(exportProperty=Cmd.ExportPropertyRequest(caseId=case_id,
                                                                     timeStep=time_step,
                                                                     property=property,
                                                                     eclipseKeyword=eclipse_keyword,
                                                                     undefinedValue=undefined_value,
                                                                     exportFile=export_file))

    def export_property_in_views(self, case_id, view_names, undefined_value):
        """ Export the current Eclipse property from the given views

        Arguments:
            case_id (int): case id
            view_names (list): list of views
            undefined_value (double):	Value to use for undefined values. Defaults to 0.0
        """
        if isinstance(view_names, str):
            view_names = [view_names]

        return self.__execute(exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(caseId=case_id,
                                                                                   viewNames=view_names,
                                                                                   undefinedValue=undefined_value))

    def export_well_path_completions(self, case_id, time_step, well_path_names, file_split,
                                  compdat_export, include_perforations, include_fishbones,
                                  exclude_main_bore_for_fishbones, combination_mode):
        if (isinstance(well_path_names, str)):
            well_path_names = [well_path_names]
        return self.__execute(exportWellPathCompletions=Cmd.ExportWellPathCompRequest(caseId=case_id,
                                                                                    timeStep=time_step,
                                                                                    wellPathNames=well_path_names,
                                                                                    fileSplit=file_split,
                                                                                    compdatExport=compdat_export,
                                                                                    includePerforations=include_perforations,
                                                                                    includeFishbones=include_fishbones,
                                                                                    excludeMainBoreForFishbones=exclude_main_bore_for_fishbones,
                                                                                    combinationMode=combination_mode))

    def export_sim_well_fracture_completions(self, case_id, view_name, time_step, simulation_well_names, file_split, compdat_export):
        if(isinstance(simulation_well_names, str)):
            simulation_well_names = [simulation_well_names]
        return self.__execute(exportSimWellFractureCompletions=Cmd.ExportSimWellPathFraqRequest(caseId=case_id,
                                                                                              viewName=view_name,
                                                                                              timeStep=time_step,
                                                                                              simulationWellNames=simulation_well_names,
                                                                                              fileSplit=file_split,
                                                                                              compdatExport=compdat_export))

    def export_msw(self, case_id, well_path):
        return self.__execute(exportMsw=Cmd.ExportMswRequest(caseId=case_id,
                                                           wellPath=well_path))

    def export_well_paths(self, well_paths=[], md_step_size=5.0):
        if isinstance(well_paths, str):
            well_paths = [well_paths]
        return self.__execute(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=well_paths, mdStepSize=md_step_size))

    def export_visible_cells(self, case_id, view_name, export_keyword='FLUXNUM', visible_active_cells_value=1, hidden_active_cells_value=0, inactive_cells_value=0):
        return self.__execute(exportVisibleCells=Cmd.ExportVisibleCellsRequest(caseId=case_id,
                                                                             viewName=view_name,
                                                                             exportKeyword=export_keyword,
                                                                             visibleActiveCellsValue=visible_active_cells_value,
                                                                             hiddenActiveCellsValue=hidden_active_cells_value,
                                                                             inactiveCellsValue=inactive_cells_value))
    def set_export_folder(self, type, path, create_folder=False):
        return self.__execute(setExportFolder=Cmd.SetExportFolderRequest(type=type,
                                                                       path=path,
                                                                       createFolder=create_folder))

    def run_octave_script(self, path, cases):
        caseIds = []
        for case in cases:
            caseIds.append(case.id)
        return self.__execute(runOctaveScript=Cmd.RunOctaveScriptRequest(path=path,
                                                                       caseIds=caseIds))
        
    def set_main_window_size(self, width, height):
        return self.__execute(setMainWindowSize=Cmd.SetMainWindowSizeParams(width=width, height=height))

    def compute_case_group_statistics(self, case_ids = [], case_group_id = -1):
        return self.__execute(computeCaseGroupStatistics=Cmd.ComputeCaseGroupStatRequest(caseIds=case_ids,
                                                                                         caseGroupId=case_group_id))

    def set_time_step(self, case_id, time_step):
        return self.__execute(setTimeStep=Cmd.SetTimeStepParams(caseId=case_id, timeStep=time_step))

    def scale_fracture_template(self, id, half_length, height, dfactor, conductivity):
        return self.__execute(scaleFractureTemplate=Cmd.ScaleFractureTemplateRequest(id=id,
                                                                                   halfLength=half_length,
                                                                                   height=height,
                                                                                   dFactor=dfactor,
                                                                                   conductivity=conductivity))

    def set_fracture_containment(self, id, top_layer, base_layer):
        return self.__execute(setFractureContainment=Cmd.SetFracContainmentRequest(id=id,
                                                                                 topLayer=top_layer,
                                                                                 baseLayer=base_layer))

    def create_multiple_fractures(self, case_id, template_id, well_path_names, min_dist_from_well_td,
                                max_fractures_per_well, top_layer, base_layer, spacing, action):
        if isinstance(well_path_names, str):
            well_path_names = [well_path_names]
        return self.__execute(createMultipleFractures=Cmd.MultipleFracAction(caseId=case_id,
                                                                           templateId=template_id,
                                                                           wellPathNames=well_path_names,
                                                                           minDistFromWellTd=min_dist_from_well_td,
                                                                           maxFracturesPerWell=max_fractures_per_well,
                                                                           topLayer=top_layer,
                                                                           baseLayer=base_layer,
                                                                           spacing=spacing,
                                                                           action=action))

    def create_lgr_for_completion(self, case_id, time_step, well_path_names, refinement_i, refinement_j, refinement_k, split_type):
        if isinstance(well_path_names, str):
            well_path_names = [well_path_names]
        return self.__execute(createLgrForCompletions=Cmd.CreateLgrForCompRequest(caseId=case_id,
                                                                                timeStep=time_step,
                                                                                wellPathNames=well_path_names,
                                                                                refinementI=refinement_i,
                                                                                refinementJ=refinement_j,
                                                                                refinementK=refinement_k,
                                                                                splitType=split_type))

    def create_saturation_pressure_plots(self, case_ids):
        if isinstance(case_ids, int):
            case_ids = [case_ids]
        return self.__execute(createSaturationPressurePlots=Cmd.CreateSatPressPlotRequest(caseIds=case_ids))

    def export_flow_characteristics(self, case_id, time_steps, injectors, producers, file_name, minimum_communication=0.0, aquifer_cell_threshold=0.1):
        """ Export Flow Characteristics data to text file in CSV format

        Parameter                 | Description                                   | Type
        ------------------------- | --------------------------------------------- | -----
        case_id                   | ID of case                                    | Integer          
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
        return self.__execute(exportFlowCharacteristics=Cmd.ExportFlowInfoRequest(caseId=case_id,
                                                                                timeSteps=time_steps,
                                                                                injectors=injectors,
                                                                                producers=producers,
                                                                                fileName=file_name,
                                                                                minimumCommunication = minimum_communication,
                                                                                aquiferCellThreshold = aquifer_cell_threshold))

    def create_view(self, case_id):
        return self.__execute(createView=Cmd.CreateViewRequest(caseId=case_id)).createViewResult.viewId

    def clone_view(self, view_id):
        return self.__execute(cloneView=Cmd.CloneViewRequest(viewId=view_id)).createViewResult.viewId