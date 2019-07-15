import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Empty_pb2 import Empty
import PdmObject_pb2
import PdmObject_pb2_grpc

class PdmObject:
    def __init__(self, pb2Object, channel):
        self.pb2Object = pb2Object
        self.channel = channel
        self.stub = PdmObject_pb2_grpc.PdmObjectServiceStub(self.channel)
    
    def address(self):
        return self.pb2Object.address

    def classKeyword(self):
        return self.pb2Object.class_keyword

    def keywords(self):
        listOfKeywords = []
        for keyword in self.pb2Object.parameters:
            listOfKeywords.append(keyword)
        return listOfKeywords
    
    def getValue(self, keyword):
        value = self.pb2Object.parameters[keyword]
        if value.lower() == 'false':
            return False
        elif value.lower() == 'true':
            return True
        else:
            try:
                intVal = int(value)
                return intVal
            except ValueError:
                try:
                    floatVal = float(value)
                    return floatVal
                except ValueError:
                    return value

    def setValue(self, keyword, value):
        if isinstance(value, bool):
            if value:
                self.pb2Object.parameters[keyword] = "true"
            else:
                self.pb2Object.parameters[keyword] = "false"
        elif isinstance(value, str):
            self.pb2Object.parameters[keyword] = "\"" + str(value) + "\""
        else:
            self.pb2Object.parameters[keyword] = str(value)

    def descendants(self, classKeyword):
        request = PdmObject_pb2.PdmChildObjectRequest(object=self.pb2Object, child_keyword=classKeyword)
        objectList = self.stub.GetDescendantPdmObjects(request).objects
        childList = []
        for object in objectList:
            childList.append(PdmObject(object, self.channel))
        return childList

    def children(self, classKeyword):
        request = PdmObject_pb2.PdmChildObjectRequest(object=self.pb2Object, child_keyword=classKeyword)
        objectList = self.stub.GetChildPdmObjects(request).objects
        childList = []
        for object in objectList:
            childList.append(PdmObject(object, self.channel))
        return childList

    def ancestor(self, classKeyword):
        request = PdmObject_pb2.PdmParentObjectRequest(object=self.pb2Object, parent_keyword=classKeyword)
        return PdmObject(self.stub.GetAncestorPdmObject(request), self.channel)

    def update(self):
        self.stub.UpdateExistingPdmObject(self.pb2Object)
