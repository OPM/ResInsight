import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Case_pb2
import Case_pb2_grpc
from Definitions_pb2 import ClientToServerStreamReply
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc
import Properties_pb2
import Properties_pb2_grpc

class Properties:
    """ Class for streaming properties to and from ResInsight

    Attributes:
        chunkSize(int): The size of each chunk during value streaming.
                        A good chunk size is 64KiB = 65536B.
                        Meaning the ideal number of doubles would be 8192.
                        However we need overhead space, so the default is 8160.
                        This leaves 256B for overhead.
    """    
    def __init__(self, case):
        """
            Arguments:
                case(Case): A rips case to handle properties for
        """
        self.case = case
        self._properties_stub = Properties_pb2_grpc.PropertiesStub(self.case.channel)
        self.chunk_size = 8160
        self.commands = CmdRpc.CommandsStub(self.case.channel)

    def _execute_command(self, **command_params):
        return self.commands.Execute(Cmd.CommandParams(**command_params))

    def __generate_property_input_iterator(self, values_iterator, parameters):
        chunk = Properties_pb2.PropertyInputChunk()
        chunk.params.CopyFrom(parameters)
        yield chunk

        for values in values_iterator:
            valmsg = Properties_pb2.PropertyChunk(values = values)
            chunk.values.CopyFrom(valmsg)
            yield chunk

    def __generate_property_input_chunks(self, array, parameters):
       
        index = -1
        while index < len(array):
            chunk = Properties_pb2.PropertyInputChunk()
            if index is -1:
                chunk.params.CopyFrom(parameters)
                index += 1
            else:
                actual_chunk_size = min(len(array) - index + 1, self.chunk_size)
                chunk.values.CopyFrom(Properties_pb2.PropertyChunk(values = array[index:index+actual_chunk_size]))
                index += actual_chunk_size

            yield chunk
        # Final empty message to signal completion
        chunk = Properties_pb2.PropertyInputChunk()
        yield chunk

    def available(self, property_type, porosity_model = 'MATRIX_MODEL'):
        """Get a list of available properties
        
        Arguments:
            property_type (str): string corresponding to property_type enum. Can be one of the following:
                - DYNAMIC_NATIVE
                - STATIC_NATIVE
                - SOURSIMRL
                - GENERATED
                - INPUT_PROPERTY
                - FORMATION_NAMES
                - FLOW_DIAGNOSTICS
                - INJECTION_FLOODING

            porosity_model(str): 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
        """

        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.AvailablePropertiesRequest (case_request = Case_pb2.CaseRequest(id=self.case.id),
                                                    property_type = property_type_enum,
                                                    porosity_model = porosity_model_enum)
        return self._properties_stub.GetAvailableProperties(request).property_names

    def active_cell_property_async(self, property_type, property_name, time_step, porosity_model = 'MATRIX_MODEL'):
        """Get a cell property for all active cells. Async, so returns an iterator
            
            Arguments:
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                porosity_model(str): string enum. See available()

            Returns:
                An iterator to a chunk object containing an array of double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.PropertyRequest(case_request   = Case_pb2.CaseRequest(id=self.case.id),
                                               property_type  = property_type_enum,
                                               property_name  = property_name,
                                               time_step      = time_step,
                                               porosity_model = porosity_model_enum)
        for chunk in self._properties_stub.GetActiveCellProperty(request):
            yield chunk

    def active_cell_property(self, property_type, property_name, time_step, porosity_model = 'MATRIX_MODEL'):
        """Get a cell property for all active cells. Sync, so returns a list
            
            Arguments:
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                porosity_model(str): string enum. See available()

            Returns:
                A list containing double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        all_values = []
        generator = self.active_cell_property_async(property_type, property_name, time_step, porosity_model)
        for chunk in generator:
            for value in chunk.values:
                all_values.append(value)
        return all_values

    def grid_property_async(self, property_type, property_name, time_step, gridIndex = 0, porosity_model = 'MATRIX_MODEL'):
        """Get a cell property for all grid cells. Async, so returns an iterator
            
            Arguments:
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                gridIndex(int): index to the grid we're getting values for
                porosity_model(str): string enum. See available()

            Returns:
                An iterator to a chunk object containing an array of double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = property_type_enum,
                                                 property_name  = property_name,
                                                 time_step      = time_step,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosity_model_enum)
        for chunk in self._properties_stub.GetGridProperty(request):
            yield chunk

    def grid_property(self, property_type, property_name, time_step, grid_index = 0, porosity_model = 'MATRIX_MODEL'):
        """Get a cell property for all grid cells. Synchronous, so returns a list
            
            Arguments:
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                grid_index(int): index to the grid we're getting values for
                porosity_model(str): string enum. See available()

            Returns:
                A list of double values
        """
        all_values = []
        generator = self.grid_property_async(property_type, property_name, time_step, grid_index, porosity_model)
        for chunk in generator:
            for value in chunk.values:
                all_values.append(value)
        return all_values

    def set_active_cell_property_async(self, values_iterator, property_type, property_name, time_step, porosity_model = 'MATRIX_MODEL'):
        """Set a cell property for all active cells. Async, and so takes an iterator to the input values
            
            Arguments:
                values_iterator(iterator): an iterator to the properties to be set
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                porosity_model(str): string enum. See available()
        """
        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = property_type_enum,
                                                 property_name  = property_name,
                                                 time_step      = time_step,
                                                 porosity_model = porosity_model_enum)
        
        request_iterator = self.__generate_property_input_iterator(values_iterator, request)
        self._properties_stub.SetActiveCellProperty(request_iterator)
    
    def set_active_cell_property(self, values, property_type, property_name, time_step, porosity_model = 'MATRIX_MODEL'):
        """Set a cell property for all active cells.
            
            Arguments:
                values(list): a list of double precision floating point numbers
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                porosity_model(str): string enum. See available()
        """
        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = property_type_enum,
                                                 property_name  = property_name,
                                                 time_step      = time_step,
                                                 porosity_model = porosity_model_enum)
        request_iterator = self.__generate_property_input_chunks(values, request)
        reply = self._properties_stub.SetActiveCellProperty(request_iterator)
        if reply.accepted_value_count < len(values):
            raise IndexError

    def set_grid_property(self, values, property_type, property_name, time_step, grid_index = 0, porosity_model = 'MATRIX_MODEL'):
        """Set a cell property for all grid cells.
            
            Arguments:
                values(list): a list of double precision floating point numbers
                property_type(str): string enum. See available()
                property_name(str): name of an Eclipse property
                time_step(int): the time step for which to get the property for
                grid_index(int): index to the grid we're setting values for
                porosity_model(str): string enum. See available()
        """
        property_type_enum = Properties_pb2.PropertyType.Value(property_type)
        porosity_model_enum = Case_pb2.PorosityModelType.Value(porosity_model)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = property_type_enum,
                                                 property_name  = property_name,
                                                 time_step      = time_step,
                                                 grid_index     = grid_index,
                                                 porosity_model = porosity_model_enum)
        request_iterator = self.__generate_property_input_chunks(values, request)
        reply = self._properties_stub.SetGridProperty(request_iterator)
        if reply.accepted_value_count < len(values):
            raise IndexError

    def export(self, time_step, property, eclipse_keyword=property, undefined_value=0.0, export_file=property):
        """ Export an Eclipse property

        Arguments:
            time_step (int): time step index
            property (str): property to export
            eclipse_keyword (str): Eclipse keyword used as text in export header. Defaults to the value of property parameter.
            undefined_value (double):	Value to use for undefined values. Defaults to 0.0
            export_file (str):	File name for export. Defaults to the value of property parameter
        """
        return self._execute_command(exportProperty=Cmd.ExportPropertyRequest(caseId=self.case.id,
                                                                     timeStep=time_step,
                                                                     property=property,
                                                                     eclipseKeyword=eclipse_keyword,
                                                                     undefinedValue=undefined_value,
                                                                     exportFile=export_file))

    def export_in_views(self, view_ids, undefined_value=0.0):
        """ Export the current Eclipse property from the given views

        Arguments:
            view_ids (list): list of view ids
            undefined_value (double):	Value to use for undefined values. Defaults to 0.0
        """
        if isinstance(view_ids, int):
            view_ids = [view_ids]

        return self._execute_command(exportPropertyInViews=Cmd.ExportPropertyInViewsRequest(caseId=self.case.id,
                                                                                   viewIds=view_ids,
                                                                                   undefinedValue=undefined_value))
            