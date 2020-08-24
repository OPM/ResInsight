# pylint: disable=no-member
# pylint: disable=too-many-arguments
# pylint: disable=too-many-public-methods
# pylint: disable=no-self-use

"""
Module containing the Case class

ResInsight Case class

Operate on a ResInsight case specified by a Case Id integer.
Not meant to be constructed separately but created by one of the following
methods in Project: loadCase, case, allCases, selectedCases

.. _result-definition-label:

Result Definition
-----------------
When working with grid case results, the following two argumenst are used in many functions to identify a
result

**Result Definition enums**::
    
    property_type           |       | porosity_model
    (str enum)              |       | (str enum)
    ----------------------- | ----- | --------------
    DYNAMIC_NATIVE          |       | MATRIX_MODEL
    STATIC_NATIVE           |       | FRACTURE_MODEL
    SOURSIMRL               |       |
    GENERATED               |       |
    INPUT_PROPERTY          |       |
    FORMATION_NAMES         |       |
    FLOW_DIAGNOSTICS        |       |
    INJECTION_FLOODING      |       |



Attributes:
    id (int): Case Id corresponding to case Id in ResInsight project.
    name (str): Case name
    group_id (int): Case Group id
    chunkSize(int): The size of each chunk during value streaming.
                    A good chunk size is 64KiB = 65536B.
                    Meaning the ideal number of doubles would be 8192.
                    However we need overhead space, so the default is 8160.
                    This leaves 256B for overhead.
"""

import builtins
import grpc

import Case_pb2
import Case_pb2_grpc
import Commands_pb2 as Cmd
import PdmObject_pb2 as PdmObject_pb2

import Properties_pb2
import Properties_pb2_grpc
import NNCProperties_pb2
import NNCProperties_pb2_grpc
from resinsight_classes import Case, EclipseCase, GeoMechCase, WellBoreStabilityPlot, WbsParameters

from .grid import Grid
from .pdmobject import add_method
from .view import View
from .simulation_well import SimulationWell
import rips.project  # full name import due to circular dependency


@add_method(Case)
def __custom_init__(self, pb2_object, channel):
    self.__case_stub = Case_pb2_grpc.CaseStub(self._channel)

    self.__properties_stub = Properties_pb2_grpc.PropertiesStub(self._channel)
    self.__nnc_properties_stub = NNCProperties_pb2_grpc.NNCPropertiesStub(self._channel)

    # Public properties
    self.chunk_size = 8160


@add_method(Case)
def __grid_count(self):
    """Get number of grids in the case"""
    try:
        return self.__case_stub.GetGridCount(self.__request()).count
    except grpc.RpcError as exception:
        if exception.code() == grpc.StatusCode.NOT_FOUND:
            return 0
        return 0


@add_method(Case)
def __request(self):
    return Case_pb2.CaseRequest(id=self.id)


@add_method(Case)
def __generate_property_input_iterator(self, values_iterator, parameters):
    chunk = Properties_pb2.PropertyInputChunk()
    chunk.params.CopyFrom(parameters)
    yield chunk

    for values in values_iterator:
        valmsg = Properties_pb2.PropertyChunk(values=values)
        chunk.values.CopyFrom(valmsg)
        yield chunk


@add_method(Case)
def __generate_property_input_chunks(self, array, parameters):
    index = -1
    while index < len(array):
        chunk = Properties_pb2.PropertyInputChunk()
        if index is -1:
            chunk.params.CopyFrom(parameters)
            index += 1
        else:
            actual_chunk_size = min(len(array) - index + 1, self.chunk_size)
            chunk.values.CopyFrom(
                Properties_pb2.PropertyChunk(values=array[index:index +
                                                          actual_chunk_size]))
            index += actual_chunk_size

        yield chunk
    # Final empty message to signal completion
    chunk = Properties_pb2.PropertyInputChunk()
    yield chunk


@add_method(Case)
def grid(self, index):
    """Get Grid of a given index

    Arguments:
        index (int): The grid index

    Returns: 
        :class:`rips.grid.Grid`
    """
    return Grid(index, self, self.channel())


@add_method(Case)
def grids(self):
    """Get a list of all rips Grid objects in the case

    Returns: 
        List of :class:`rips.grid.Grid`
    """
    grid_list = []
    for i in range(0, self.__grid_count()):
        grid_list.append(Grid(i, self, self.channel()))
    return grid_list


@add_method(Case)
def replace(self, new_grid_file):
    """Replace the current case grid with a new grid loaded from file

    Arguments:
        new_egrid_file (str): Path to EGRID file
    """
    project = self.ancestor(rips.project.Project)
    self._execute_command(replaceCase=Cmd.ReplaceCaseRequest(
        newGridFile=new_grid_file, caseId=self.id))
    new_case = project.case(self.id)
    self.copy_from(new_case)


@add_method(Case)
def cell_count(self, porosity_model="MATRIX_MODEL"):
    """Get a cell count object containing number of active cells and total number of cells

    Arguments:
        porosity_model (str): String representing an enum.
            must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
    Returns:
        Cell Count object with the following integer attributes:
            active_cell_count: number of active cells
            reservoir_cell_count: total number of reservoir cells

    **CellCount class description**::

        Parameter               | Description               | Type
        ----------------------- | ------------------------- | -----
        active_cell_count       | Number of active cells    | Integer
        reservoir_cell_count    | Total number of cells     | Integer

    """
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Case_pb2.CellInfoRequest(case_request=self.__request(),
                                       porosity_model=porosity_model_enum)
    return self.__case_stub.GetCellCount(request)


@add_method(Case)
def cell_info_for_active_cells_async(self, porosity_model="MATRIX_MODEL"):
    """Get Stream of cell info objects for current case

    Arguments:
        porosity_model(str): String representing an enum.
            must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.

    Returns:
        Stream of **CellInfo** objects

    See :meth:`rips.case.cell_info_for_active_cells()` for detalis on the **CellInfo** class.
    """
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Case_pb2.CellInfoRequest(case_request=self.__request(),
                                       porosity_model=porosity_model_enum)
    return self.__case_stub.GetCellInfoForActiveCells(request)


@add_method(Case)
def cell_info_for_active_cells(self, porosity_model="MATRIX_MODEL"):
    """Get list of cell info objects for current case

    Arguments:
        porosity_model(str): String representing an enum.
            must be 'MATRIX_MODEL' or 'FRACTURE_MODEL'.

    Returns:
        List of **CellInfo** objects

    **CellInfo class description**::

        Parameter                 | Description                                   | Type
        ------------------------- | --------------------------------------------- | -----
        grid_index                | Index to grid                                 | Integer
        parent_grid_index         | Index to parent grid                          | Integer
        coarsening_box_index      | Index to coarsening box                       | Integer
        local_ijk                 | Cell index in IJK directions of local grid    | Vec3i
        parent_ijk                | Cell index in IJK directions of parent grid   | Vec3i

    **Vec3i class description**::

        Parameter        | Description                                  | Type
        ---------------- | -------------------------------------------- | -----
        i                | I grid index                                 | Integer
        j                | J grid index                                 | Integer
        k                | K grid index                                 | Integer

    """
    active_cell_info_chunks = self.cell_info_for_active_cells_async(
        porosity_model=porosity_model)
    received_active_cells = []
    for active_cell_chunk in active_cell_info_chunks:
        for active_cell in active_cell_chunk.data:
            received_active_cells.append(active_cell)
    return received_active_cells


@add_method(Case)
def time_steps(self):
    """Get a list containing all time steps

    The time steps are defined by the class **TimeStepDate**

    **TimeStepDate class description**::

        Type      | Name
        --------- | ----------
        int       | year
        int       | month
        int       | day
        int       | hour
        int       | minute
        int       | second


    """
    return self.__case_stub.GetTimeSteps(self.__request()).dates


@add_method(Case)
def reservoir_boundingbox(self):
    """Get the reservoir bounding box

    Returns:
        BoundingBox

    **BoundingBox class description**::

        Type      | Name
        --------- | ----------
        int       | min_x
        int       | max_x
        int       | min_y
        int       | max_y
        int       | min_z
        int       | max_z


    """
    return self.__case_stub.GetReservoirBoundingBox(self.__request())


@add_method(Case)
def reservoir_depth_range(self):
    """Get the reservoir depth range

    Returns:
        A tuple with two members. The first is the minimum depth, the second is the maximum depth
    """
    bbox = self.reservoir_boundingbox()
    return -bbox.max_z, -bbox.min_z


@add_method(Case)
def days_since_start(self):
    """Get a list of decimal values representing days since the start of the simulation"""
    return self.__case_stub.GetDaysSinceStart(self.__request()).day_decimals


@add_method(Case)
def view(self, view_id):
    """Get a particular view belonging to a case by providing view id

    Arguments:
        view_id(int): view id

    Returns:
        :class:`rips.generated.resinsight_classes.View`
    """
    views = self.views()
    for view_object in views:
        if view_object.id == view_id:
            return view_object
    return None


@add_method(Case)
def create_view(self):
    """Create a new view in the current case

    Returns: 
        :class:`rips.generated.resinsight_classes.View`
    """
    return self.view(
        self._execute_command(createView=Cmd.CreateViewRequest(
            caseId=self.id)).createViewResult.viewId)


@add_method(Case)
def export_snapshots_of_all_views(self, prefix="", export_folder=""):
    """ Export snapshots for all views in the case

    Arguments:
        prefix (str): Exported file name prefix
        export_folder(str): The path to export to. By default will use the global export folder

    """
    return self._execute_command(
        exportSnapshots=Cmd.ExportSnapshotsRequest(
            type="VIEWS", prefix=prefix, caseId=self.id, viewId=-1, exportFolder=export_folder))


@add_method(Case)
def export_well_path_completions(
        self,
        time_step,
        well_path_names,
        file_split,
        compdat_export="TRANSMISSIBILITIES",
        include_perforations=True,
        include_fishbones=True,
        fishbones_exclude_main_bore=True,
        combination_mode="INDIVIDUALLY",
):
    """
    Export well path completions for the current case to file

    **Parameters**::

        Parameter                   | Description                                      | Type
        ----------------------------| ------------------------------------------------ | -----
        time_step                   | Time step to export for                          | Integer
        well_path_names             | List of well path names                          | List
        file_split                  | Controls how export data is split into files     | String enum
        compdat_export              | Compdat export type                              | String enum
        include_perforations        | Export perforations?                             | bool
        include_fishbones           | Export fishbones?                                | bool
        fishbones_exclude_main_bore | Exclude main bore when exporting fishbones?      | bool
        combination_mode            | Settings for multiple completions in same cell   | String Enum

    **Enum file_split**::

        Option                              | Description
        ----------------------------------- | ------------
        "UNIFIED_FILE"                      | A single file with all combined transmissibilities
        "SPLIT_ON_WELL"                     | One file for each well with combined transmissibilities
        "SPLIT_ON_WELL_AND_COMPLETION_TYPE" | One file for each completion type for each well 

    **Enum compdat_export**::

        Option                                      | Description
        ------------------------------------------- | ------------
        "TRANSMISSIBILITIES"                        | Direct export of transmissibilities
        "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS"    | Include WPIMULT in addition to transmissibilities

    **Enum combination_mode**::

        Option              | Description
        ------------------- | ------------
        "INDIVIDUALLY"      | Exports the different completion types into separate sections
        "COMBINED"          | Export one combined transmissibility for each cell

    """
    if isinstance(well_path_names, str):
        well_path_names = [well_path_names]
    return self._execute_command(
        exportWellPathCompletions=Cmd.ExportWellPathCompRequest(
            caseId=self.id,
            timeStep=time_step,
            wellPathNames=well_path_names,
            fileSplit=file_split,
            compdatExport=compdat_export,
            includePerforations=include_perforations,
            includeFishbones=include_fishbones,
            excludeMainBoreForFishbones=fishbones_exclude_main_bore,
            combinationMode=combination_mode,
        ))


@add_method(Case)
def export_msw(self, well_path):
    """
    Export Eclipse Multi-segment-well model to file

    Arguments:
        well_path(str): Well path name
    """
    return self._execute_command(exportMsw=Cmd.ExportMswRequest(
        caseId=self.id, wellPath=well_path))


@add_method(Case)
def create_multiple_fractures(
        self,
        template_id,
        well_path_names,
        min_dist_from_well_td,
        max_fractures_per_well,
        top_layer,
        base_layer,
        spacing,
        action,
):
    """
    Create Multiple Fractures in one go

    **Parameters**::

        Parameter              | Description                               | Type
        -----------------------| ----------------------------------------- | -----
        template_id            | Id of the template                        | Integer
        well_path_names        | List of well path names                   | List of Strings
        min_dist_from_well_td  | Minimum distance from well TD             | Double
        max_fractures_per_well | Max number of fractures per well          | Integer
        top_layer              | Top grid k-level for fractures            | Integer
        base_layer             | Base grid k-level for fractures           | Integer
        spacing                | Spacing between fractures                 | Double
        action                 | 'APPEND_FRACTURES' or 'REPLACE_FRACTURES' | String enum

    """
    if isinstance(well_path_names, str):
        well_path_names = [well_path_names]
    return self._execute_command(
        createMultipleFractures=Cmd.MultipleFracRequest(
            caseId=self.id,
            templateId=template_id,
            wellPathNames=well_path_names,
            minDistFromWellTd=min_dist_from_well_td,
            maxFracturesPerWell=max_fractures_per_well,
            topLayer=top_layer,
            baseLayer=base_layer,
            spacing=spacing,
            action=action,
        ))


@add_method(Case)
def create_lgr_for_completion(
        self,
        time_step,
        well_path_names,
        refinement_i,
        refinement_j,
        refinement_k,
        split_type,
):
    """
    Create a local grid refinement for the completions on the given well paths

    **Parameters**::

        Parameter       | Description                            | Type
        --------------- | -------------------------------------- | -----
        time_steps      | Time step index                        | Integer
        well_path_names | List of well path names                | List of Strings
        refinement_i    | Refinment in x-direction               | Integer
        refinement_j    | Refinment in y-direction               | Integer
        refinement_k    | Refinment in z-direction               | Integer
        split_type      | Defines how to split LGRS              | String enum

    **Enum split_type**::

        Option                  | Description
        ------------------------| ------------
        "LGR_PER_CELL"          | One LGR for each completed cell 
        "LGR_PER_COMPLETION"    | One LGR for each completion (fracture, perforation, ...)
        "LGR_PER_WELL"          | One LGR for each well

    """
    if isinstance(well_path_names, str):
        well_path_names = [well_path_names]
    return self._execute_command(
        createLgrForCompletions=Cmd.CreateLgrForCompRequest(
            caseId=self.id,
            timeStep=time_step,
            wellPathNames=well_path_names,
            refinementI=refinement_i,
            refinementJ=refinement_j,
            refinementK=refinement_k,
            splitType=split_type,
        ))


@add_method(Case)
def create_saturation_pressure_plots(self):
    """
    Create saturation pressure plots for the current case
    """
    case_ids = [self.id]
    return self._execute_command(
        createSaturationPressurePlots=Cmd.CreateSatPressPlotRequest(
            caseIds=case_ids))


@add_method(Case)
def export_flow_characteristics(
        self,
        time_steps,
        injectors,
        producers,
        file_name,
        minimum_communication=0.0,
        aquifer_cell_threshold=0.1,
):
    """ Export Flow Characteristics data to text file in CSV format

    **Parameters**::

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
    return self._execute_command(
        exportFlowCharacteristics=Cmd.ExportFlowInfoRequest(
            caseId=self.id,
            timeSteps=time_steps,
            injectors=injectors,
            producers=producers,
            fileName=file_name,
            minimumCommunication=minimum_communication,
            aquiferCellThreshold=aquifer_cell_threshold,
        ))


@add_method(Case)
def available_properties(self,
                         property_type,
                         porosity_model="MATRIX_MODEL"):
    """Get a list of available properties

    For argument details, see :ref:`result-definition-label`

    Arguments:
        property_type (str): string corresponding to property_type enum.
        porosity_model(str): 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
    """

    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.AvailablePropertiesRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        porosity_model=porosity_model_enum,
    )
    return self.__properties_stub.GetAvailableProperties(
        request).property_names


@add_method(Case)
def active_cell_property_async(self,
                               property_type,
                               property_name,
                               time_step,
                               porosity_model="MATRIX_MODEL"):
    """Get a cell property for all active cells. Async, so returns an iterator. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum

        Returns:
            An iterator to a chunk object containing an array of double values
            Loop through the chunks and then the values within the chunk to get all values.
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        porosity_model=porosity_model_enum,
    )
    for chunk in self.__properties_stub.GetActiveCellProperty(request):
        yield chunk


@add_method(Case)
def active_cell_property(self,
                         property_type,
                         property_name,
                         time_step,
                         porosity_model="MATRIX_MODEL"):
    """Get a cell property for all active cells. Sync, so returns a list. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum

        Returns:
            A list containing double values
            Loop through the chunks and then the values within the chunk to get all values.
    """
    all_values = []
    generator = self.active_cell_property_async(property_type,
                                                property_name, time_step,
                                                porosity_model)
    for chunk in generator:
        for value in chunk.values:
            all_values.append(value)
    return all_values


@add_method(Case)
def selected_cell_property_async(self,
                                 property_type,
                                 property_name,
                                 time_step,
                                 porosity_model="MATRIX_MODEL"):
    """Get a cell property for all selected cells. Async, so returns an iterator. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum

        Returns:
            An iterator to a chunk object containing an array of double values
            Loop through the chunks and then the values within the chunk to get all values.
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        porosity_model=porosity_model_enum,
    )
    for chunk in self.__properties_stub.GetSelectedCellProperty(request):
        yield chunk


@add_method(Case)
def selected_cell_property(self,
                           property_type,
                           property_name,
                           time_step,
                           porosity_model="MATRIX_MODEL"):
    """Get a cell property for all selected cells. Sync, so returns a list. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum

        Returns:
            A list containing double values
            Loop through the chunks and then the values within the chunk to get all values.
    """
    all_values = []
    generator = self.selected_cell_property_async(property_type,
                                                  property_name, time_step,
                                                  porosity_model)
    for chunk in generator:
        for value in chunk.values:
            all_values.append(value)
    return all_values


@add_method(Case)
def grid_property_async(
        self,
        property_type,
        property_name,
        time_step,
        grid_index=0,
        porosity_model="MATRIX_MODEL"):
    """Get a cell property for all grid cells. Async, so returns an iterator. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            gridIndex(int): index to the grid we're getting values for
            porosity_model(str): string enum

        Returns:
            An iterator to a chunk object containing an array of double values
            Loop through the chunks and then the values within the chunk to get all values.
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        grid_index=grid_index,
        porosity_model=porosity_model_enum,
    )
    for chunk in self.__properties_stub.GetGridProperty(request):
        yield chunk


@add_method(Case)
def grid_property(
        self,
        property_type,
        property_name,
        time_step,
        grid_index=0,
        porosity_model="MATRIX_MODEL"):
    """Get a cell property for all grid cells. Synchronous, so returns a list. For argument details, see :ref:`result-definition-label`

        Arguments:
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            grid_index(int): index to the grid we're getting values for
            porosity_model(str): string enum

        Returns:
            A list of double values
    """
    all_values = []
    generator = self.grid_property_async(property_type, property_name,
                                         time_step, grid_index,
                                         porosity_model)
    for chunk in generator:
        for value in chunk.values:
            all_values.append(value)
    return all_values


@add_method(Case)
def set_active_cell_property_async(
        self,
        values_iterator,
        property_type,
        property_name,
        time_step,
        porosity_model="MATRIX_MODEL"):
    """Set cell property for all active cells Async. Takes an iterator to the input values. For argument details, see :ref:`result-definition-label`

        Arguments:
            values_iterator(iterator): an iterator to the properties to be set
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        porosity_model=porosity_model_enum,
    )

    request_iterator = self.__generate_property_input_iterator(
        values_iterator, request)
    self.__properties_stub.SetActiveCellProperty(request_iterator)


@add_method(Case)
def set_active_cell_property(
        self,
        values,
        property_type,
        property_name,
        time_step,
        porosity_model="MATRIX_MODEL"):
    """Set a cell property for all active cells. For argument details, see :ref:`result-definition-label`

        Arguments:
            values(list): a list of double precision floating point numbers
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        porosity_model=porosity_model_enum,
    )
    request_iterator = self.__generate_property_input_chunks(
        values, request)
    reply = self.__properties_stub.SetActiveCellProperty(request_iterator)
    if reply.accepted_value_count < len(values):
        raise IndexError


@add_method(Case)
def set_grid_property(
        self,
        values,
        property_type,
        property_name,
        time_step,
        grid_index=0,
        porosity_model="MATRIX_MODEL"):
    """Set a cell property for all grid cells. For argument details, see :ref:`result-definition-label`

        Arguments:
            values(list): a list of double precision floating point numbers
            property_type(str): string enum
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            grid_index(int): index to the grid we're setting values for
            porosity_model(str): string enum
    """
    property_type_enum = Properties_pb2.PropertyType.Value(property_type)
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Properties_pb2.PropertyRequest(
        case_request=self.__request(),
        property_type=property_type_enum,
        property_name=property_name,
        time_step=time_step,
        grid_index=grid_index,
        porosity_model=porosity_model_enum,
    )
    request_iterator = self.__generate_property_input_chunks(
        values, request)
    reply = self.__properties_stub.SetGridProperty(request_iterator)
    if reply.accepted_value_count < len(values):
        raise IndexError


@add_method(Case)
def export_property(
        self,
        time_step,
        property_name,
        eclipse_keyword=property,
        undefined_value=0.0,
        export_file=property):
    """ Export an Eclipse property

    Arguments:
        time_step (int): time step index
        property_name (str): property to export
        eclipse_keyword (str): Keyword used in export header. Defaults: value of property
        undefined_value (double):	Value to use for undefined values. Defaults to 0.0
        export_file (str):	File name for export. Defaults to the value of property parameter
    """
    return self._execute_command(exportProperty=Cmd.ExportPropertyRequest(
        caseId=self.id,
        timeStep=time_step,
        property=property_name,
        eclipseKeyword=eclipse_keyword,
        undefinedValue=undefined_value,
        exportFile=export_file,
    ))


@add_method(Case)
def create_well_bore_stability_plot(self, well_path, time_step, parameters=None):
    """ Create a new well bore stability plot

    Arguments:
        well_path(str): well path name
        time_step(int): time step

    Returns:
        :class:`rips.generated.resinsight_classes.WellBoreStabilityPlot`
    """
    pb2_parameters = None
    if parameters is not None:
        assert(isinstance(parameters, WbsParameters))
        pb2_parameters = parameters.pb2_object()

    plot_result = self._execute_command(createWellBoreStabilityPlot=Cmd.CreateWbsPlotRequest(caseId=self.id,
                                                                                             wellPath=well_path,
                                                                                             timeStep=time_step,
                                                                                             wbsParameters=pb2_parameters))
    project = self.ancestor(rips.project.Project)
    plot = project.plot(view_id=plot_result.createWbsPlotResult.viewId)
    return plot


@add_method(Case)
def import_formation_names(self, formation_files=None):
    """ Import formation names into project and apply it to the current case

    Arguments:
        formation_files(list): list of files to import

    """
    if formation_files is None:
        formation_files = []
    elif isinstance(formation_files, str):
        formation_files = [formation_files]

    self._execute_command(importFormationNames=Cmd.ImportFormationNamesRequest(formationFiles=formation_files,
                                                                               applyToCaseId=self.id))


@add_method(Case)
def simulation_wells(self):
    """Get a list of all simulation wells for a case

    Returns:
        :class:`rips.generated.resinsight_classes.SimulationWell`

    """
    wells = self.descendants(SimulationWell)
    return wells


@add_method(Case)
def active_cell_centers_async(
        self,
        porosity_model="MATRIX_MODEL"):
    """Get a cell centers for all active cells. Async, so returns an iterator

        Arguments:
            porosity_model(str): string enum. See available()

        Returns:
            An iterator to a chunk object containing an array of Vec3d values.
            Loop through the chunks and then the values within the chunk to get all values.
    """
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Case_pb2.CellInfoRequest(case_request=self.__request(),
                                       porosity_model=porosity_model_enum)
    return self.__case_stub.GetCellCenterForActiveCells(request)


@add_method(Case)
def active_cell_centers(
        self,
        porosity_model="MATRIX_MODEL"):
    """Get a cell centers for all active cells. Synchronous, so returns a list.

        Arguments:
            porosity_model(str): string enum. See available()

        Returns:
            A list of Vec3d
    """
    cell_centers = []
    generator = self.active_cell_centers_async(porosity_model)
    for chunk in generator:
        for value in chunk.centers:
            cell_centers.append(value)
    return cell_centers


@add_method(Case)
def active_cell_corners_async(
        self,
        porosity_model="MATRIX_MODEL"):
    """Get a cell corners for all active cells. Async, so returns an iterator

        Arguments:
            porosity_model(str): string enum. See available()

        Returns:
            An iterator to a chunk object containing an array of CellCorners (which is eight Vec3d values).
            Loop through the chunks and then the values within the chunk to get all values.
    """
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = Case_pb2.CellInfoRequest(case_request=self.__request(),
                                       porosity_model=porosity_model_enum)
    return self.__case_stub.GetCellCornersForActiveCells(request)


@add_method(Case)
def active_cell_corners(
        self,
        porosity_model="MATRIX_MODEL"):
    """Get a cell corners for all active cells. Synchronous, so returns a list.

        Arguments:
            porosity_model(str): string enum. See available()

    **CellCorner class description**::

        Parameter   | Description   | Type
        ----------- | ------------  | -----
        c0          |               | Vec3d
        c1          |               | Vec3d
        c2          |               | Vec3d
        c3          |               | Vec3d
        c4          |               | Vec3d
        c5          |               | Vec3d
        c6          |               | Vec3d
        c7          |               | Vec3d


    """
    cell_corners = []
    generator = self.active_cell_corners_async(porosity_model)
    for chunk in generator:
        for value in chunk.cells:
            cell_corners.append(value)
    return cell_corners


@add_method(Case)
def selected_cells_async(self):
    """Get the selected cells. Async, so returns an iterator.

        Returns:
            An iterator to a chunk object containing an array of cells.
            Loop through the chunks and then the cells within the chunk to get all cells.
    """
    return self.__case_stub.GetSelectedCells(self.__request())


@add_method(Case)
def selected_cells(self):
    """Get the selected cells. Synchronous, so returns a list.

        Returns:
            A list of Cells.
    """
    cells = []
    generator = self.selected_cells_async()
    for chunk in generator:
        for value in chunk.cells:
            cells.append(value)
    return cells


@add_method(Case)
def coarsening_info(self):
    """Get a coarsening information for all grids in the case.

        Returns:
            A list of CoarseningInfo objects with two Vec3i min and max objects
            for each entry.
    """
    return self.__case_stub.GetCoarseningInfoArray(self.__request()).data


@add_method(Case)
def available_nnc_properties(self):
    """Get a list of available NNC properties

  **NNCConnection class description**::

        Parameter               | Description                                   | Type
        ------------------------| --------------------------------------------- | -----
        cell_grid_index1        | Reservoir Cell Index to cell 1                | int32
        cell_grid_index2        | Reservoir Cell Index to cell 2                | int32
        cell1                   | Reservoir Cell IJK to cell 1                  | Vec3i
        cell2                   | Reservoir Cell IJK to cell 1                  | Vec3i

    """
    return self.__nnc_properties_stub.GetAvailableNNCProperties(self.__request()).properties


@add_method(Case)
def nnc_connections_async(self):
    """Get the NNC connections. Async, so returns an iterator.

        Returns:
            An iterator to a chunk object containing an array NNCConnection objects.
            Loop through the chunks and then the connection within the chunk to get all connections.
    """
    return self.__nnc_properties_stub.GetNNCConnections(self.__request())


@add_method(Case)
def nnc_connections(self):
    """Get the NNC connection. Synchronous, so returns a list.

        Returns:
            A list of NNCConnection objects.
    """
    connections = []
    generator = self.nnc_connections_async()
    for chunk in generator:
        for value in chunk.connections:
            connections.append(value)
    return connections


@add_method(Case)
def __nnc_connections_values_async(self, property_name, property_type, time_step):
    request = NNCProperties_pb2.NNCValuesRequest(case_id=self.id,
                                                 property_name=property_name,
                                                 property_type=property_type,
                                                 time_step=time_step)
    return self.__nnc_properties_stub.GetNNCValues(request)


@add_method(Case)
def __nnc_values_generator_to_list(self, generator):
    """Converts a NNC values generator to a list."""
    vals = []
    for chunk in generator:
        for value in chunk.values:
            vals.append(value)
    return vals


@add_method(Case)
def nnc_connections_static_values_async(self, property_name):
    """Get the static NNC values. Async, so returns an iterator.

        Returns:
            An iterator to a chunk object containing an list of doubles.
            Loop through the chunks and then the values within the chunk to get values
            for all the connections. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    return self.__nnc_connections_values_async(property_name, NNCProperties_pb2.NNC_STATIC, 0)


@add_method(Case)
def nnc_connections_static_values(self, property_name):
    """Get the static NNC values.

        Returns:
            A list of doubles. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    generator = self.nnc_connections_static_values_async(property_name)
    return self.__nnc_values_generator_to_list(generator)


@add_method(Case)
def nnc_connections_dynamic_values_async(self, property_name, time_step):
    """Get the dynamic NNC values. Async, so returns an iterator.

        Returns:
            An iterator to a chunk object containing an list of doubles.
            Loop through the chunks and then the values within the chunk to get values
            for all the connections. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    return self.__nnc_connections_values_async(property_name, NNCProperties_pb2.NNC_DYNAMIC, time_step)


@add_method(Case)
def nnc_connections_dynamic_values(self, property_name, time_step):
    """Get the dynamic NNC values.

        Returns:
            A list of doubles. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    generator = self.nnc_connections_dynamic_values_async(property_name, time_step)
    return self.__nnc_values_generator_to_list(generator)


@add_method(Case)
def nnc_connections_generated_values_async(self, property_name, time_step):
    """Get the generated NNC values. Async, so returns an iterator.

        Returns:
            An iterator to a chunk object containing an list of doubles.
            Loop through the chunks and then the values within the chunk to get values
            for all the connections. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    return self.__nnc_connections_values_async(property_name, NNCProperties_pb2.NNC_GENERATED, time_step)


@add_method(Case)
def nnc_connections_generated_values(self, property_name, time_step):
    """Get the generated NNC values.

        Returns:
            A list of doubles. The order of the list matches the list from
            nnc_connections, i.e. the nth object of nnc_connections() refers to nth
            value in this list.
    """
    generator = self.nnc_connections_generated_values_async(property_name, time_step)
    return self.__nnc_values_generator_to_list(generator)


@add_method(Case)
def __generate_nnc_property_input_chunks(self, array, parameters):
    index = -1
    while index < len(array):
        chunk = NNCProperties_pb2.NNCValuesChunk()
        if index is -1:
            chunk.params.CopyFrom(parameters)
            index += 1
        else:
            actual_chunk_size = min(len(array) - index + 1, self.chunk_size)
            chunk.values.CopyFrom(
                NNCProperties_pb2.NNCValues(values=array[index:index + actual_chunk_size]))
            index += actual_chunk_size

        yield chunk
    # Final empty message to signal completion
    chunk = NNCProperties_pb2.NNCValuesChunk()
    yield chunk


@add_method(Case)
def set_nnc_connections_values(
        self,
        values,
        property_name,
        time_step,
        porosity_model="MATRIX_MODEL"):
    """Set nnc connection values for all connections..

        Arguments:
            values(list): a list of double precision floating point numbers
            property_name(str): name of an Eclipse property
            time_step(int): the time step for which to get the property for
            porosity_model(str): string enum. See available()
    """
    porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
    request = NNCProperties_pb2.NNCValuesInputRequest(
        case_id=self.id,
        property_name=property_name,
        time_step=time_step,
        porosity_model=porosity_model_enum,
    )
    request_iterator = self.__generate_nnc_property_input_chunks(values, request)
    reply = self.__nnc_properties_stub.SetNNCValues(request_iterator)
    if reply.accepted_value_count < len(values):
        raise IndexError
