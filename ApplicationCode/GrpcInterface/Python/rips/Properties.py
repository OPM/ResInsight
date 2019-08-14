import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Properties_pb2
import Properties_pb2_grpc
import Case_pb2
import Case_pb2_grpc
from Definitions_pb2 import ClientToServerStreamReply

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
        self.propertiesStub = Properties_pb2_grpc.PropertiesStub(self.case.channel)
        self.chunkSize = 8160

    
    def __generatePropertyInputIterator(self, values_iterator, parameters):
        chunk = Properties_pb2.PropertyInputChunk()
        chunk.params.CopyFrom(parameters)
        yield chunk

        for values in values_iterator:
            valmsg = Properties_pb2.PropertyChunk(values = values)
            chunk.values.CopyFrom(valmsg)
            yield chunk

    def __generatePropertyInputChunks(self, array, parameters):
       
        index = -1
        while index < len(array):
            chunk = Properties_pb2.PropertyInputChunk()
            if index is -1:
                chunk.params.CopyFrom(parameters)
                index += 1
            else:
                actualChunkSize = min(len(array) - index + 1, self.chunkSize)
                chunk.values.CopyFrom(Properties_pb2.PropertyChunk(values = array[index:index+actualChunkSize]))
                index += actualChunkSize

            yield chunk
        # Final empty message to signal completion
        chunk = Properties_pb2.PropertyInputChunk()
        yield chunk

    def available(self, propertyType, porosityModel = 'MATRIX_MODEL'):
        """Get a list of available properties
        
        Arguments:
            propertyType (str): string corresponding to propertyType enum.
                
                Can be one of the following:
	            
                'DYNAMIC_NATIVE'
	            'STATIC_NATIVE'
	            'SOURSIMRL'
	            'GENERATED'
	            'INPUT_PROPERTY'
	            'FORMATION_NAMES'
	            'FLOW_DIAGNOSTICS'
	            'INJECTION_FLOODING'
            porosityModel(str): 'MATRIX_MODEL' or 'FRACTURE_MODEL'.
        """

        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.AvailablePropertiesRequest (case_request = Case_pb2.CaseRequest(id=self.case.id),
                                                    property_type = propertyTypeEnum,
                                                    porosity_model = porosityModelEnum)
        return self.propertiesStub.GetAvailableProperties(request).property_names

    def activeCellPropertyAsync(self, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        """Get a cell property for all active cells. Async, so returns an iterator
            
            Arguments:
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                porosityModel(str): string enum. See available()

            Returns:
                An iterator to a chunk object containing an array of double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = Case_pb2.CaseRequest(id=self.case.id),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               porosity_model = porosityModelEnum)
        for chunk in self.propertiesStub.GetActiveCellProperty(request):
            yield chunk

    def activeCellProperty(self, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        """Get a cell property for all active cells. Sync, so returns a list
            
            Arguments:
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                porosityModel(str): string enum. See available()

            Returns:
                A list containing double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        allValues = []
        generator = self.activeCellPropertyAsync(propertyType, propertyName, timeStep, porosityModel)
        for chunk in generator:
            for value in chunk.values:
                allValues.append(value)
        return allValues

    def gridPropertyAsync(self, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        """Get a cell property for all grid cells. Async, so returns an iterator
            
            Arguments:
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                gridIndex(int): index to the grid we're getting values for
                porosityModel(str): string enum. See available()

            Returns:
                An iterator to a chunk object containing an array of double values
                You first loop through the chunks and then the values within the chunk to get all values.
        """
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        for chunk in self.propertiesStub.GetGridProperty(request):
            yield chunk

    def gridProperty(self, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        """Get a cell property for all grid cells. Synchronous, so returns a list
            
            Arguments:
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                gridIndex(int): index to the grid we're getting values for
                porosityModel(str): string enum. See available()

            Returns:
                A list of double values
        """
        allValues = []
        generator = self.gridPropertyAsync(propertyType, propertyName, timeStep, gridIndex, porosityModel)
        for chunk in generator:
            for value in chunk.values:
                allValues.append(value)
        return allValues

    def setActiveCellPropertyAsync(self, values_iterator, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        """Set a cell property for all active cells. Async, and so takes an iterator to the input values
            
            Arguments:
                values_iterator(iterator): an iterator to the properties to be set
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                porosityModel(str): string enum. See available()
        """
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 porosity_model = porosityModelEnum)
        
        request_iterator = self.__generatePropertyInputIterator(values_iterator, request)
        self.propertiesStub.SetActiveCellProperty(request_iterator)
    
    def setActiveCellProperty(self, values, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        """Set a cell property for all active cells.
            
            Arguments:
                values(list): a list of double precision floating point numbers
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                porosityModel(str): string enum. See available()
        """
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 porosity_model = porosityModelEnum)
        request_iterator = self.__generatePropertyInputChunks(values, request)
        reply = self.propertiesStub.SetActiveCellProperty(request_iterator)
        if reply.accepted_value_count < len(values):
            raise IndexError

    def setGridProperty(self, values, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        """Set a cell property for all grid cells.
            
            Arguments:
                values(list): a list of double precision floating point numbers
                propertyType(str): string enum. See available()
                propertyName(str): name of an Eclipse property
                timeStep(int): the time step for which to get the property for
                gridIndex(int): index to the grid we're setting values for
                porosityModel(str): string enum. See available()
        """
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        request_iterator = self.__generatePropertyInputChunks(values, request)
        reply = self.propertiesStub.SetGridProperty(request_iterator)
        if reply.accepted_value_count < len(values):
            raise IndexError
            