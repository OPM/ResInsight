"""
ResInsight 3d view module
"""

import Commands_pb2 as Cmd

import uuid
import warnings
import rips.project

import rips.case  # Circular import of Case, which already imports View. Use full name.
from .pdmobject import add_method
from .resinsight_classes import (
    View as View,
    ViewWindow as ViewWindow,
    EclipseView as EclipseView,
    GeoMechView as GeoMechView,
)


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
def export_property(self, undefined_value=0.0):
    """Export the current Eclipse property from the view

    Deprecated: use EclipseView.export_current_property(export_file=...) instead, which requires
    an explicit file. This method writes to the PROPERTIES folder set by Instance.set_export_folder().

    Arguments:
        undefined_value (double): Value to use for undefined values. Defaults to 0.0
    """
    warnings.warn(
        "View.export_property() is deprecated, use EclipseView.export_current_property() with an explicit export_file instead",
        DeprecationWarning,
        stacklevel=3,
    )
    case_id = self.case().id
    return self._execute_command(
        exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(
            caseId=case_id, viewIds=[self.id], undefinedValue=undefined_value
        )
    )


def extract_address(address) -> int:
    # Address form: "RimReservoir:123345345345435"
    parts = address.split(":")
    return int(parts[1])


@add_method(View)
def case(self):
    """Get the case the view belongs to"""
    project = self.ancestor(rips.project.Project)

    eclipse_case_addr = extract_address(self.eclipse_case)

    cases = project.cases()
    for c in cases:
        if c.address() == eclipse_case_addr:
            return c
    return None


@add_method(View)
def visible_cells(self, time_step=0):
    """Get the visibility status of all cells in the view

    Arguments:
        time_step (int): The time step to get visibility for. Defaults to 0.

    Returns:
        List[int]: A list where 1 represents a visible cell and 0 represents an invisible cell
    """

    # Generate temporary key for key-value store
    visibility_key = f"{uuid.uuid4()}_visibility"

    # Get the project
    project = self.ancestor(rips.project.Project)
    try:
        # Call internal method to store visibility data
        self.visible_cells_internal(visibility_key=visibility_key, time_step=time_step)

        # Retrieve visibility data from key-value store
        visibility_values = project.key_values(visibility_key)

        # Convert floats to integers (1 for visible, 0 for invisible)
        return [int(v) for v in visibility_values]

    finally:
        # Clean up temporary key from key-value store
        project.remove_key_values(visibility_key)
