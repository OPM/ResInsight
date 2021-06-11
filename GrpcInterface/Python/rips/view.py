"""
ResInsight 3d view module
"""

import builtins
import Commands_pb2 as Cmd

import rips.case  # Circular import of Case, which already imports View. Use full name.
from .pdmobject import add_method
from .resinsight_classes import View, ViewWindow, EclipseView, GeoMechView


@add_method(View)
def apply_cell_result(self, result_type, result_variable):
    """Apply a regular cell result

    Arguments:
        result_type (str): String representing the result category. The valid values are::
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
    cell_result = self.cell_result()
    cell_result.result_type = result_type
    cell_result.result_variable = result_variable
    cell_result.update()


@add_method(View)
def apply_flow_diagnostics_cell_result(
    self,
    result_variable="TOF",
    selection_mode="FLOW_TR_BY_SELECTION",
    injectors=None,
    producers=None,
):
    """Apply a flow diagnostics cell result

    **Parameters**::

        Parameter           | Description                                            | Type
        ------------------- | ------------------------------------------------------ | -----
        result_variable     | String representing the result value                   | String
        selection_mode      | String specifying which tracers to select              | String
        injectors           | List of injector names, used by 'FLOW_TR_BY_SELECTION' | String List
        producers           | List of injector names, used by 'FLOW_TR_BY_SELECTION' | String List

    **Enum compdat_export**::

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
    cell_result = self.cell_result()
    cell_result.result_type = "FLOW_DIAGNOSTICS"
    cell_result.result_variable = result_variable
    cell_result.flow_tracer_selection_mode = selection_mode
    if selection_mode == "FLOW_TR_BY_SELECTION":
        cell_result.selected_injector_tracers = injectors
        cell_result.selected_producer_tracers = producers
    cell_result.update()


@add_method(View)
def clone(self):
    """Clone the current view"""
    view_id = self._execute_command(
        cloneView=Cmd.CloneViewRequest(viewId=self.id)
    ).createViewResult.viewId
    return self.case().view(view_id)


@add_method(View)
def set_time_step(self, time_step):
    """Set the time step for current view"""
    case_id = self.case().id
    return self._execute_command(
        setTimeStep=Cmd.SetTimeStepParams(
            caseId=case_id, viewId=self.id, timeStep=time_step
        )
    )


@add_method(View)
def export_sim_well_fracture_completions(
    self, time_step, simulation_well_names, file_split, compdat_export
):
    """Export fracture completions for simulation wells

    **Parameters**::

        Parameter                   | Description                                      | Type
        ----------------------------| ------------------------------------------------ | -----
        time_step                   | Time step to export for                          | Integer
        simulation_well_names       | List of simulation well names                    | List
        file_split                  | Controls how export data is split into files     | String enum
        compdat_export              | Compdat export type                              | String enum

    **Enum file_split**::

        Option                              | Description
        ----------------------------------- | ------------
        "UNIFIED_FILE" <b>Default Option</b>| A single file with all transmissibilities
        "SPLIT_ON_WELL"                     | One file for each well transmissibilities
        "SPLIT_ON_WELL_AND_COMPLETION_TYPE" | One file for each completion type for each well

    **Enum compdat_export**::

        Option                                   | Description
        -----------------------------------------| ------------
        "TRANSMISSIBILITIES"<b>Default Option</b>| Direct export of transmissibilities
        "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS" | Include export of WPIMULT

    """
    if isinstance(simulation_well_names, str):
        simulation_well_names = [simulation_well_names]

    case_id = self.case().id
    return self._execute_command(
        exportSimWellFractureCompletions=Cmd.ExportSimWellPathFracRequest(
            caseId=case_id,
            viewId=self.id,
            timeStep=time_step,
            simulationWellNames=simulation_well_names,
            fileSplit=file_split,
            compdatExport=compdat_export,
        )
    )


@add_method(View)
def export_visible_cells(
    self,
    export_keyword="FLUXNUM",
    visible_active_cells_value=1,
    hidden_active_cells_value=0,
    inactive_cells_value=0,
):
    """Export special properties for all visible cells.

    Arguments:
        export_keyword (string): The keyword to export.
        Choices: 'FLUXNUM' or 'MULTNUM'. Default: 'FLUXNUM'
        visible_active_cells_value (int): Value to export forvisible active cells. Default: 1
        hidden_active_cells_value (int): Value to export for hidden active cells. Default: 0
        inactive_cells_value (int): Value to export for inactive cells. Default: 0
    """
    case_id = self.case().id
    return self._execute_command(
        exportVisibleCells=Cmd.ExportVisibleCellsRequest(
            caseId=case_id,
            viewId=self.id,
            exportKeyword=export_keyword,
            visibleActiveCellsValue=visible_active_cells_value,
            hiddenActiveCellsValue=hidden_active_cells_value,
            inactiveCellsValue=inactive_cells_value,
        )
    )


@add_method(View)
def export_property(self, undefined_value=0.0):
    """Export the current Eclipse property from the view

    Arguments:
        undefined_value (double): Value to use for undefined values. Defaults to 0.0
    """
    case_id = self.case().id
    return self._execute_command(
        exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(
            caseId=case_id, viewIds=[self.id], undefinedValue=undefined_value
        )
    )


@add_method(ViewWindow)
def case(self):
    """Get the case the view belongs to"""
    mycase = self.ancestor(rips.case.Case)
    assert mycase is not None
    return mycase


@add_method(ViewWindow)
def export_snapshot(self, prefix="", export_folder=""):
    """Export snapshot for the current view

    Arguments:
        prefix (str): Exported file name prefix
        export_folder(str): The path to export to. By default will use the global export folder
    """
    case_id = self.case().id
    return self._execute_command(
        exportSnapshots=Cmd.ExportSnapshotsRequest(
            type="VIEWS",
            prefix=prefix,
            caseId=case_id,
            viewId=self.id,
            exportFolder=export_folder,
        )
    )
