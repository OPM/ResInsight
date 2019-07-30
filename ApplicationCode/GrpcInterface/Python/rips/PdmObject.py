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
        self.pdmObjectStub = PdmObject_pb2_grpc.PdmObjectServiceStub(self.channel)
    
    def address(self):
        return self.pb2Object.address

    def classKeyword(self):
        return self.pb2Object.class_keyword

    def keywords(self):
        listOfKeywords = []
        for keyword in self.pb2Object.parameters:
            listOfKeywords.append(keyword)
        return listOfKeywords
    
    def printObjectInfo(self):
        print ("Class Keyword: " + self.classKeyword())
        for keyword in self.keywords():
            print(keyword + " [" + type(self.getValue(keyword)).__name__ + "]: " + str(self.getValue(keyword)))

    def __toValue(self, value):
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
                    # We may have a string. Strip internal start and end quotes
                    value = value.strip('\"')
                    if self.__islist(value):
                        return self.__makelist(value)
                    return value
    def __fromValue(self, value):
        if isinstance(value, bool):
            if value:
                return "true"
            else:
                return "false"
        elif isinstance(value, list):
            listofstrings = []
            for val in value:
                listofstrings.append(self.__fromValue(val))
            return "[" + ", ".join(listofstrings) + "]"
        else:
            return str(value)

    def getValue(self, keyword):
        value = self.pb2Object.parameters[keyword]
        return self.__toValue(value)

    def __islist(self, value):
        return value.startswith("[") and value.endswith("]")

    def __makelist(self, liststring):
        liststring = liststring.lstrip("[")
        liststring = liststring.rstrip("]")
        strings = liststring.split(", ")
        values = []
        for string in strings:
            values.append(self.__toValue(string))
        return values

    def setValue(self, keyword, value):
        self.pb2Object.parameters[keyword] = self.__fromValue(value)

    def descendants(self, classKeyword):
        request = PdmObject_pb2.PdmDescendantObjectRequest(object=self.pb2Object, child_keyword=classKeyword)
        objectList = self.pdmObjectStub.GetDescendantPdmObjects(request).objects
        childList = []
        for object in objectList:
            childList.append(PdmObject(object, self.channel))
        return childList

    def children(self, childField):
        request = PdmObject_pb2.PdmChildObjectRequest(object=self.pb2Object, child_field=childField)
        objectList = self.pdmObjectStub.GetChildPdmObjects(request).objects
        childList = []
        for object in objectList:
            childList.append(PdmObject(object, self.channel))
        return childList

    def ancestor(self, classKeyword):
        request = PdmObject_pb2.PdmParentObjectRequest(object=self.pb2Object, parent_keyword=classKeyword)
        return PdmObject(self.pdmObjectStub.GetAncestorPdmObject(request), self.channel)

    def update(self):
        self.pdmObjectStub.UpdateExistingPdmObject(self.pb2Object)

#    def createChild(self, childField, childClassKeyword):
#        childRequest = PdmObject_pb2.CreatePdmChildObjectRequest(object=self.pb2Object, child_field=childField, child_class=childClassKeyword)
#        return PdmObject(self.pdmObjectStub.CreateChildPdmObject(childRequest), self.channel)
