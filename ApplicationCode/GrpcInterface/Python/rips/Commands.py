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

    def __executeCmd(self, **command_params):
        return self.commands.Execute(Cmd.CommandParams(**command_params))
     
    ##################
    # Export Commands
    ##################

    def export_multi_case_snapshots(self, grid_list_file):
        """Export snapshots for a set of cases
        
        Arguments:
            grid_list_file (str): Path to a file containing a list of grids to export snapshot for
        
        """
        return self.__executeCmd(exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(gridListFile=grid_list_file))

    def export_snapshots(self, type = 'ALL', prefix='', case_id = -1):
        """ Export snapshots of a given type
        
        Arguments:
            type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
            prefix (str): Exported file name prefix
            case_id (int): the case Id to export for. The default of -1 will export all cases
        
        """
        return self.__executeCmd(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
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
        return self.__executeCmd(exportProperty=Cmd.ExportPropertyRequest(caseId=case_id,
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

        return self.__executeCmd(exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(caseId=case_id,
                                                                                   viewNames=view_names,
                                                                                   undefinedValue=undefined_value))

    def export_well_path_completions(self, case_id, time_step, well_path_names, file_split,
                                  compdat_export, include_perforations, include_fishbones,
                                  exclude_main_bore_for_fishbones, combination_mode):
        if (isinstance(well_path_names, str)):
            well_path_names = [well_path_names]
        return self.__executeCmd(exportWellPathCompletions=Cmd.ExportWellPathCompRequest(caseId=case_id,
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
        return self.__executeCmd(exportSimWellFractureCompletions=Cmd.ExportSimWellPathFraqRequest(caseId=case_id,
                                                                                              viewName=view_name,
                                                                                              timeStep=time_step,
                                                                                              simulationWellNames=simulation_well_names,
                                                                                              fileSplit=file_split,
                                                                                              compdatExport=compdat_export))

    def export_msw(self, case_id, well_path):
        return self.__executeCmd(exportMsw=Cmd.ExportMswRequest(caseId=case_id,
                                                           wellPath=well_path))

    def export_well_paths(self, well_paths=[], md_step_size=5.0):
        if isinstance(well_paths, str):
            well_paths = [well_paths]
        return self.__executeCmd(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=well_paths, mdStepSize=md_step_size))

    def export_visible_cells(self, case_id, view_name, export_keyword='FLUXNUM', visible_active_cells_value=1, hidden_active_cells_value=0, inactive_cells_value=0):
        return self.__executeCmd(exportVisibleCells=Cmd.ExportVisibleCellsRequest(caseId=case_id,
                                                                             viewName=view_name,
                                                                             exportKeyword=export_keyword,
                                                                             visibleActiveCellsValue=visible_active_cells_value,
                                                                             hiddenActiveCellsValue=hidden_active_cells_value,
                                                                             inactiveCellsValue=inactive_cells_value))
    def set_export_folder(self, type, path, create_folder=False):
        return self.__executeCmd(setExportFolder=Cmd.SetExportFolderRequest(type=type,
                                                                       path=path,
                                                                       createFolder=create_folder))

    def run_octave_script(self, path, cases):
        caseIds = []
        for case in cases:
            caseIds.append(case.id)
        return self.__executeCmd(runOctaveScript=Cmd.RunOctaveScriptRequest(path=path,
                                                                       caseIds=caseIds))
        
    def set_main_window_size(self, width, height):
        return self.__executeCmd(setMainWindowSize=Cmd.SetMainWindowSizeParams(width=width, height=height))

    def compute_case_group_statistics(self, case_ids = [], case_group_id = -1):
        return self.__executeCmd(computeCaseGroupStatistics=Cmd.ComputeCaseGroupStatRequest(caseIds=case_ids,
                                                                                         caseGroupId=case_group_id))

    def set_time_step(self, case_id, time_step):
        return self.__executeCmd(setTimeStep=Cmd.SetTimeStepParams(caseId=case_id, timeStep=time_step))

    def scale_fracture_template(self, id, half_length, height, dfactor, conductivity):
        return self.__executeCmd(scaleFractureTemplate=Cmd.ScaleFractureTemplateRequest(id=id,
                                                                                   halfLength=half_length,
                                                                                   height=height,
                                                                                   dFactor=dfactor,
                                                                                   conductivity=conductivity))

    def set_fracture_containment(self, id, top_layer, base_layer):
        return self.__executeCmd(setFractureContainment=Cmd.SetFracContainmentRequest(id=id,
                                                                                 topLayer=top_layer,
                                                                                 baseLayer=base_layer))

    def create_multiple_fractures(self, case_id, template_id, well_path_names, min_dist_from_well_td,
                                max_fractures_per_well, top_layer, base_layer, spacing, action):
        if isinstance(well_path_names, str):
            well_path_names = [well_path_names]
        return self.__executeCmd(createMultipleFractures=Cmd.MultipleFracAction(caseId=case_id,
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
        return self.__executeCmd(createLgrForCompletions=Cmd.CreateLgrForCompRequest(caseId=case_id,
                                                                                timeStep=time_step,
                                                                                wellPathNames=well_path_names,
                                                                                refinementI=refinement_i,
                                                                                refinementJ=refinement_j,
                                                                                refinementK=refinement_k,
                                                                                splitType=split_type))

    def create_saturation_pressure_plots(self, case_ids):
        if isinstance(case_ids, int):
            case_ids = [case_ids]
        return self.__executeCmd(createSaturationPressurePlots=Cmd.CreateSatPressPlotRequest(caseIds=case_ids))

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
        return self.__executeCmd(exportFlowCharacteristics=Cmd.ExportFlowInfoRequest(caseId=case_id,
                                                                                timeSteps=time_steps,
                                                                                injectors=injectors,
                                                                                producers=producers,
                                                                                fileName=file_name,
                                                                                minimumCommunication = minimum_communication,
                                                                                aquiferCellThreshold = aquifer_cell_threshold))

    def create_view(self, case_id):
        return self.__executeCmd(createView=Cmd.CreateViewRequest(caseId=case_id)).createViewResult.viewId

    def clone_view(self, view_id):
        return self.__executeCmd(cloneView=Cmd.CloneViewRequest(viewId=view_id)).createViewResult.viewId