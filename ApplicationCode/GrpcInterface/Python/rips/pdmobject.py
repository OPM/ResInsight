# pylint: disable=no-self-use
"""
ResInsight caf::PdmObject connection module
"""

from functools import partial
import grpc
import re
import builtins

import rips.generated.PdmObject_pb2 as PdmObject_pb2
import rips.generated.PdmObject_pb2_grpc as PdmObject_pb2_grpc
import rips.generated.Commands_pb2 as Cmd
import rips.generated.Commands_pb2_grpc as CmdRpc
from rips.generated.pdm_objects import PdmObject

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

class PdmObjectCustomization:
    """
    Generic ResInsight object. Corresponds to items in the Project Tree
    """

    def _execute_command(self, **command_params):
        self.__warnings = []
        response, call = self._commands.Execute.with_call(Cmd.CommandParams(**command_params))
        for key, value in call.trailing_metadata():
            value = value.replace(';;', '\n')
            if key == 'warning':
                self.__warnings.append(value)

        return response
        
    def __custom_init__(self, pb2_object, channel):
        self._pb2_object = pb2_object
        self._channel = channel
        self._pdm_object_stub = PdmObject_pb2_grpc.PdmObjectServiceStub(self._channel)
        self._commands = CmdRpc.CommandsStub(channel)
        self.__warnings = []

        self.__keyword_translation = {}
        for camel_keyword in self.keywords():
            snake_keyword = camel_to_snake(camel_keyword)
            setter = 'set_' + snake_keyword            
            setattr(self, snake_keyword, self.__get_grpc_value(camel_keyword))
            setattr(self, setter, partial(self.set_value, snake_keyword))
            self.__keyword_translation[snake_keyword] = camel_keyword            

    @classmethod
    def create(cls, class_keyword, channel):
        pb2_object = PdmObject_pb2.PdmObject(class_keyword=class_keyword)
        return cls(pb2_object, channel)

    def warnings(self):
        return self.__warnings

    def has_warnings(self):
        return len(self.__warnings) > 0

    def pb2_object(self):
        """ Private method"""
        return self._pb2_object

    def channel(self):
        """ Private method"""
        return self._channel

    def address(self):
        """Get the unique address of the PdmObject

        Returns:
            A 64-bit unsigned integer address
        """

        return self._pb2_object.address

    def class_keyword(self):
        """Get the class keyword in the ResInsight Data Model for the given PdmObject"""
        return self._pb2_object.class_keyword

    def set_visible(self, visible):
        """Set the visibility of the object in the ResInsight project tree"""
        self._pb2_object.visible = visible

    def visible(self):
        """Get the visibility of the object in the ResInsight project tree"""
        return self._pb2_object.visible
        
    def keywords(self):
        """Get a list of all parameter keywords available in the object"""
        list_of_keywords = []
        for keyword in self._pb2_object.parameters:
            list_of_keywords.append(keyword)
        return list_of_keywords

    def print_object_info(self):
        """Print the structure and data content of the PdmObject"""
        print("=========== " + self.class_keyword() + " =================")
        print("Object Attributes: ")
        for snake_kw, camel_kw in self.__keyword_translation:
            print("   " + snake_kw + " [" + type(getattr(self, snake_kw)).__name__ +
                  "]: " + str(getattr(self, snake_kw)))
        print("Object Methods:")
        for method in dir(self):
            if callable(getattr(self, method)) and not method.startswith("_"):
                print ("   " + method)

    def __convert_from_grpc_value(self, value):
        if value.lower() == 'false':
            return False
        if value.lower() == 'true':
            return True
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

    def __convert_to_grpc_value(self, value):
        if isinstance(value, bool):
            if value:
                return "true"
            return "false"
        if isinstance(value, list):
            list_of_strings = []
            for val in value:
                list_of_strings.append(self.__convert_to_grpc_value('\"' + val + '\"'))
            return "[" + ", ".join(list_of_strings) + "]"
        return str(value)

    def __get_grpc_value(self, camel_keyword):
        return self.__convert_from_grpc_value(self._pb2_object.parameters[camel_keyword])

    def __set_grpc_value(self, camel_keyword, value):
        self._pb2_object.parameters[camel_keyword] = self.__convert_to_grpc_value(value)

    def set_value(self, snake_keyword, value):
        """Set the value associated with the provided keyword and updates ResInsight
        Arguments:
            keyword(str): A string containing the parameter keyword
            value(varying): A value matching the type of the parameter.
                See keyword documentation and/or print_object_info() to find
                the correct data type.
        """
        setattr(self, snake_keyword, value)
        self.update()

    def __islist(self, value):
        return value.startswith("[") and value.endswith("]")

    def __makelist(self, list_string):
        list_string = list_string.lstrip("[")
        list_string = list_string.rstrip("]")
        strings = list_string.split(", ")
        values = []
        for string in strings:
            values.append(self.__convert_from_grpc_value(string))
        return values

    def descendants(self, class_keyword):
        """Get a list of all project tree descendants matching the class keyword
        Arguments:
            class_keyword[str]: A class keyword matching the type of class wanted

        Returns:
            A list of PdmObjects matching the keyword provided
        """
        request = PdmObject_pb2.PdmDescendantObjectRequest(
            object=self._pb2_object, child_keyword=class_keyword)
        object_list = self._pdm_object_stub.GetDescendantPdmObjects(
            request).objects
        child_list = []
        for pdm_object in object_list:
            child_list.append(PdmObject(pdm_object, self._channel))
        return child_list

    def children(self, child_field):
        """Get a list of all direct project tree children inside the provided child_field
        Arguments:
            child_field[str]: A field name
        Returns:
            A list of PdmObjects inside the child_field
        """
        request = PdmObject_pb2.PdmChildObjectRequest(object=self._pb2_object,
                                                      child_field=child_field)
        try:
            object_list = self._pdm_object_stub.GetChildPdmObjects(request).objects
            child_list = []
            for pdm_object in object_list:
                child_list.append(PdmObject(pdm_object, self._channel))
            return child_list
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return []
            raise e

    def ancestor(self, class_keyword):
        """Find the first ancestor that matches the provided class_keyword
        Arguments:
            class_keyword[str]: A class keyword matching the type of class wanted
        """
        request = PdmObject_pb2.PdmParentObjectRequest(
            object=self._pb2_object, parent_keyword=class_keyword)
        try:
            return PdmObject(self._pdm_object_stub.GetAncestorPdmObject(request),
                             self._channel)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return None
            raise e

    def copy_from(self, object):
        """Copy attribute values from object to self and requires that the attribute already exists           
        """
        for attribute in dir(object):
            if not attribute.startswith('__') and hasattr(self, attribute):
                value = getattr(object, attribute)
                if not callable(value):
                    setattr(self, attribute, value)

    def update(self):
        """Sync all fields from the Python Object to ResInsight"""

        for snake_kw, camel_kw in self.__keyword_translation.items():
            self.__set_grpc_value(camel_kw, getattr(self, snake_kw))

        self._pdm_object_stub.UpdateExistingPdmObject(self._pb2_object)

for attr in dir(PdmObjectCustomization):
    if not hasattr(builtins.object, attr):
        setattr(PdmObject, attr, getattr(PdmObjectCustomization, attr))
