import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Definitions_pb2 import Empty
import PdmObject_pb2
import PdmObject_pb2_grpc
import Commands_pb2 as Cmd
import Commands_pb2_grpc as CmdRpc

class PdmObject:
    def _execute_command(self, **command_params):
        return self.commands.Execute(Cmd.CommandParams(**command_params))

    def __init__(self, pb2_object, channel):
        self.pb2_object = pb2_object
        self.channel = channel
        self.pdm_object_stub = PdmObject_pb2_grpc.PdmObjectServiceStub(self.channel)
        self.commands = CmdRpc.CommandsStub(channel)
 
    def address(self):
        """Get the unique address of the PdmObject
        
        Returns:
            A 64-bit unsigned integer address
        """

        return self.pb2_object.address

    def class_keyword(self):
        """Get the class keyword in the ResInsight Data Model for the given PdmObject"""
        return self.pb2_object.class_keyword

    def keywords(self):
        """Get a list of all parameter keywords available in the object"""
        list_of_keywords = []
        for keyword in self.pb2_object.parameters:
            list_of_keywords.append(keyword)
        return list_of_keywords
    
    def print_object_info(self):
        """Print the structure and data content of the PdmObject"""
        print ("Class Keyword: " + self.class_keyword())
        for keyword in self.keywords():
            print(keyword + " [" + type(self.get_value(keyword)).__name__ + "]: " + str(self.get_value(keyword)))

    def __to_value(self, value):
        if value.lower() == 'false':
                return False
        elif value.lower() == 'true':
            return True
        else:
            try:
                int_val = int(value)
                return int_val
            except ValueError:
                try:
                    float_val = float(value)
                    return float_val
                except ValueError:
                    # We may have a string. Strip internal start and end quotes
                    value = value.strip('\"')
                    if self.__islist(value):
                        return self.__makelist(value)
                    return value
    def __from_value(self, value):
        if isinstance(value, bool):
            if value:
                return "true"
            else:
                return "false"
        elif isinstance(value, list):
            list_of_strings = []
            for val in value:
                list_of_strings.append(self.__from_value('\"' + val + '\"'))
            return "[" + ", ".join(list_of_strings) + "]"
        else:
            return str(value)

    def get_value(self, keyword):
        """Get the value associated with the provided keyword
        Arguments:
            keyword(str): A string containing the parameter keyword
        
        Returns:
            The value of the parameter. Can be int, str or list.
        """
        value = self.pb2_object.parameters[keyword]
        return self.__to_value(value)

    def __islist(self, value):
        return value.startswith("[") and value.endswith("]")

    def __makelist(self, list_string):
        list_string = list_string.lstrip("[")
        list_string = list_string.rstrip("]")
        strings = list_string.split(", ")
        values = []
        for string in strings:
            values.append(self.__to_value(string))
        return values

    def set_value(self, keyword, value):
        """Set the value associated with the provided keyword
        Arguments:
            keyword(str): A string containing the parameter keyword
            value(varying): A value matching the type of the parameter.
                See keyword documentation and/or print_object_info() to find
                the correct data type.
        """
        self.pb2_object.parameters[keyword] = self.__from_value(value)

    def descendants(self, class_keyword):
        """Get a list of all project tree descendants matching the class keyword
        Arguments:
            class_keyword[str]: A class keyword matching the type of class wanted

        Returns:
            A list of PdmObjects matching the keyword provided
        """
        request = PdmObject_pb2.PdmDescendantObjectRequest(object=self.pb2_object, child_keyword=class_keyword)
        object_list = self.pdm_object_stub.GetDescendantPdmObjects(request).objects
        child_list = []
        for object in object_list:
            child_list.append(PdmObject(object, self.channel))
        return child_list

    def children(self, child_field):
        """Get a list of all direct project tree children inside the provided child_field
        Arguments:
            child_field[str]: A field name
        Returns:
            A list of PdmObjects inside the child_field
        """
        request = PdmObject_pb2.PdmChildObjectRequest(object=self.pb2_object, child_field=child_field)
        object_list = self.pdm_object_stub.GetChildPdmObjects(request).objects
        child_list = []
        for object in object_list:
            child_list.append(PdmObject(object, self.channel))
        return child_list

    def ancestor(self, class_keyword):
        """Find the first ancestor that matches the provided class_keyword
        Arguments:
            class_keyword[str]: A class keyword matching the type of class wanted
        """
        request = PdmObject_pb2.PdmParentObjectRequest(object=self.pb2_object, parent_keyword=class_keyword)
        return PdmObject(self.pdm_object_stub.GetAncestorPdmObject(request), self.channel)

    def update(self):
        """Sync all fields from the Python Object to ResInsight"""
        self.pdm_object_stub.UpdateExistingPdmObject(self.pb2_object)

#    def createChild(self, child_field, childClassKeyword):
#        childRequest = PdmObject_pb2.CreatePdmChildObjectRequest(object=self.pb2Object, child_field=child_field, child_class=childClassKeyword)
#        return PdmObject(self.pdmObjectStub.CreateChildPdmObject(childRequest), self.channel)
