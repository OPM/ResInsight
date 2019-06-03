import grpc
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

import Properties_pb2
import Properties_pb2_grpc
import Case_pb2
import Case_pb2_grpc

class Properties:
    def __init__(self, case):
        self.case = case
        self.propertiesStub = Properties_pb2_grpc.PropertiesStub(self.case.channel)
    
    def generatePropertyInputIterator(self, values_iterator, parameters):
        chunk = Properties_pb2.PropertyInputChunk()
        chunk.params.CopyFrom(parameters)
        yield chunk

        for values in values_iterator:
            valmsg = Properties_pb2.PropertyChunk(values = values)
            chunk.values.CopyFrom(valmsg)
            yield chunk

    def generatePropertyInputChunks(self, array, parameters):
         # Each double is 8 bytes. A good chunk size is 64KiB = 65536B
         # Meaning ideal number of doubles would be 8192.
         # However we need overhead space, so if we choose 8160 in chunk size
         # We have 256B left for overhead which should be plenty
        chunkSize = 8000
        index = -1
        while index < len(array):
            chunk = Properties_pb2.PropertyInputChunk()
            if index is -1:
                chunk.params.CopyFrom(parameters)
                index += 1;
            else:
                actualChunkSize = min(len(array) - index + 1, chunkSize)
                chunk.values.CopyFrom(Properties_pb2.PropertyChunk(values = array[index:index+actualChunkSize]))
                index += actualChunkSize

            yield chunk
        # Final empty message to signal completion
        chunk = Properties_pb2.PropertyInputChunk()
        yield chunk

    def available(self, propertyType, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.AvailablePropertiesRequest (case_request = Case_pb2.CaseRequest(id=self.case.id),
                                                    property_type = propertyTypeEnum,
                                                    porosity_model = porosityModelEnum)
        return self.propertiesStub.GetAvailableProperties(request).property_names

    def activeCellProperty(self, propertyType, propertyName, timeStep, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = Case_pb2.CaseRequest(id=self.case.id),
                                               property_type  = propertyTypeEnum,
                                               property_name  = propertyName,
                                               time_step      = timeStep,
                                               porosity_model = porosityModelEnum)
        for chunk in self.propertiesStub.GetActiveCellProperty(request):
            yield chunk

    def gridProperty(self, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        return self.propertiesStub.GetGridProperty(request)

    def setActiveCellPropertyAsync(self, values_iterator, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        try:
            reply_iterator = self.generatePropertyInputIterator(values_iterator, request)
            self.propertiesStub.SetActiveCellProperty(reply_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)

    def setActiveCellProperty(self, values, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        try:
            request_iterator = self.generatePropertyInputChunks(values, request)
            self.propertiesStub.SetActiveCellProperty(request_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)
    def setGridProperty(self, values, propertyType, propertyName, timeStep, gridIndex = 0, porosityModel = 'MATRIX_MODEL'):
        propertyTypeEnum = Properties_pb2.PropertyType.Value(propertyType)
        porosityModelEnum = Case_pb2.PorosityModelType.Value(porosityModel)
        request = Properties_pb2.PropertyRequest(case_request   = self.case.request,
                                                 property_type  = propertyTypeEnum,
                                                 property_name  = propertyName,
                                                 time_step      = timeStep,
                                                 grid_index     = gridIndex,
                                                 porosity_model = porosityModelEnum)
        try:
            request_iterator = self.generatePropertyInputChunks(values, request)
            self.propertiesStub.SetGridProperty(request_iterator)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                print("Command not found")
            else:
                print("Other error", e)
