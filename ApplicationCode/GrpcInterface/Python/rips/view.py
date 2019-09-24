"""
ResInsight 3d view module
"""
import rips.generated.Commands_pb2 as Cmd

import rips.case  # Circular import of Case, which already imports View. Use full name.
from rips.pdmobject import PdmObject


class View(PdmObject):
    """ResInsight view class

    Attributes:
        id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pdm_object):
        PdmObject.__init__(self, pdm_object.pb2_object(), pdm_object.channel())
        self.view_id = pdm_object.get_value("ViewId")

    def show_grid_box(self):
        """Check if the grid box is meant to be shown in the view"""
        return self.get_value("ShowGridBox")

    def set_show_grid_box(self, value):
        """Set if the grid box is meant to be shown in the view"""
        self.set_value("ShowGridBox", value)

    def background_color(self):
        """Get the current background color in the view"""
        return self.get_value("ViewBackgroundColor")

    def set_background_color(self, bgcolor):
        """Set the background color in the view"""
        self.set_value("ViewBackgroundColor", bgcolor)

    def set_cell_result(self):
        """Retrieve the current cell results"""
        return self.children("GridCellResult")[0]

    def apply_cell_result(self, result_type, result_variable):
        """Apply a regular cell result

        Arguments:
            result_type (str): String representing the result category. The valid values are
                - DYNAMIC_NATIVE
                - STATIC_NATIVE
                - SOURSIMRL
                - GENERATED
                - INPUT_PROPERTY
                - FORMATION_NAMES
                - FLOW_DIAGNOSTICS
                - INJECTION_FLOODING
            result_variable (str): String representing the result variable.
        """
        cell_result = self.set_cell_result()
        cell_result.set_value("ResultType", result_type)
        cell_result.set_value("ResultVariable", result_variable)
        cell_result.update()

    def apply_flow_diagnostics_cell_result(
            self,
            result_variable='TOF',
            selection_mode='FLOW_TR_BY_SELECTION',
            injectors=None,
            producers=None):
        """Apply a flow diagnostics cell result

        Parameter           | Description                                            | Type
        ------------------- | ------------------------------------------------------ | -----
        result_variable     | String representing the result value                   | String
        selection_mode      | String specifying which tracers to select              | String
        injectors           | List of injector names, used by 'FLOW_TR_BY_SELECTION' | String List
        producers           | List of injector names, used by 'FLOW_TR_BY_SELECTION' | String List

        ##### Enum compdat_export 

        Option                  | Description
        ------------------------| ------------
        "TOF"                   | Time of flight
        "Fraction"              | Fraction
        "MaxFractionTracer"     | Max Fraction Tracer
        "Communication"         | Communication

        """
        if injectors is None:
            injectors = []
        if producers is None:
            producers = []
        cell_result = self.set_cell_result()
        cell_result.set_value("ResultType", "FLOW_DIAGNOSTICS")
        cell_result.set_value("ResultVariable", result_variable)
        cell_result.set_value("FlowTracerSelectionMode", selection_mode)
        if selection_mode == 'FLOW_TR_BY_SELECTION':
            cell_result.set_value("SelectedInjectorTracers", injectors)
            cell_result.set_value("SelectedProducerTracers", producers)
        cell_result.update()

    def case(self):
        """Get the case the view belongs to"""
        pdm_case = self.ancestor("EclipseCase")
        if pdm_case is None:
            pdm_case = self.ancestor("ResInsightGeoMechCase")
        if pdm_case is None:
            return None
        return rips.case.Case(self._channel, pdm_case.get_value("CaseId"))

    def clone(self):
        """Clone the current view"""
        view_id = self._execute_command(cloneView=Cmd.CloneViewRequest(
            viewId=self.view_id)).createViewResult.viewId
        return self.case().view(view_id)

    def set_time_step(self, time_step):
        """Set the time step for current view"""
        case_id = self.case().case_id
        return self._execute_command(setTimeStep=Cmd.SetTimeStepParams(
            caseId=case_id, viewId=self.view_id, timeStep=time_step))

    def export_sim_well_fracture_completions(self, time_step,
                                             simulation_well_names, file_split,
                                             compdat_export):
        """Export fracture completions for simulation wells

        Parameter                   | Description                                      | Type
        ----------------------------| ------------------------------------------------ | -----
        time_step                   | Time step to export for                          | Integer
        simulation_well_names       | List of simulation well names                    | List
        file_split                  | Controls how export data is split into files     | String enum
        compdat_export              | Compdat export type                              | String enum

        ##### Enum file_split 

        Option                              | Description
        ----------------------------------- | ------------
        "UNIFIED_FILE" <b>Default Option</b>| A single file with all transmissibilities
        "SPLIT_ON_WELL"                     | One file for each well transmissibilities
        "SPLIT_ON_WELL_AND_COMPLETION_TYPE" | One file for each completion type for each well 

        ##### Enum compdat_export 

        Option                                   | Description
        -----------------------------------------| ------------
        "TRANSMISSIBILITIES"<b>Default Option</b>| Direct export of transmissibilities 
        "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS" | Include export of WPIMULT

        """
        if isinstance(simulation_well_names, str):
            simulation_well_names = [simulation_well_names]

        case_id = self.case().case_id
        return self._execute_command(
            exportSimWellFractureCompletions=Cmd.ExportSimWellPathFracRequest(
                caseId=case_id,
                viewId=self.view_id,
                timeStep=time_step,
                simulationWellNames=simulation_well_names,
                fileSplit=file_split,
                compdatExport=compdat_export))

    def export_visible_cells(self,
                             export_keyword='FLUXNUM',
                             visible_active_cells_value=1,
                             hidden_active_cells_value=0,
                             inactive_cells_value=0):
        """Export special properties for all visible cells.

        Arguments:
            export_keyword (string): The keyword to export.
            Choices: 'FLUXNUM' or 'MULTNUM'. Default: 'FLUXNUM'
            visible_active_cells_value (int): Value to export forvisible active cells. Default: 1
            hidden_active_cells_value (int): Value to export for hidden active cells. Default: 0
            inactive_cells_value (int): Value to export for inactive cells. Default: 0
        """
        case_id = self.case().case_id
        return self._execute_command(
            exportVisibleCells=Cmd.ExportVisibleCellsRequest(
                caseId=case_id,
                viewId=self.view_id,
                exportKeyword=export_keyword,
                visibleActiveCellsValue=visible_active_cells_value,
                hiddenActiveCellsValue=hidden_active_cells_value,
                inactiveCellsValue=inactive_cells_value))

    def export_property(self, undefined_value=0.0):
        """ Export the current Eclipse property from the view

        Arguments:
            undefined_value (double): Value to use for undefined values. Defaults to 0.0
        """
        case_id = self.case().case_id
        return self._execute_command(
            exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(
                caseId=case_id,
                viewIds=[self.view_id],
                undefinedValue=undefined_value))

    def export_snapshot(self, prefix=''):
        """ Export snapshot for the current view
        
        Arguments:
            prefix (str): Exported file name prefix
        """
        case_id = self.case().case_id
        return self._execute_command(
            exportSnapshots=Cmd.ExportSnapshotsRequest(type='VIEWS',
                                                       prefix=prefix,
                                                       caseId=case_id,
                                                       viewId=self.view_id))
