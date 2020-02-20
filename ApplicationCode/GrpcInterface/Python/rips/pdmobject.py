# pylint: disable=no-self-use
"""
ResInsight caf::PdmObject connection module
"""

from functools import partial, wraps
import grpc
import re
import builtins
import inspect

import rips.generated.PdmObject_pb2 as PdmObject_pb2
import rips.generated.PdmObject_pb2_grpc as PdmObject_pb2_grpc
import rips.generated.Commands_pb2 as Cmd
import rips.generated.Commands_pb2_grpc as CmdRpc
from rips.generated.pdm_objects import PdmObject

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def add_method(cls):
    def decorator(func):
        setattr(cls, func.__name__, func)
        return func # returning func means func can still be used normally
    return decorator

def add_static_method(cls):
    def decorator(func):
        @wraps(func) 
        def wrapper(*args, **kwargs): 
            return func(*args, **kwargs)
        setattr(cls, func.__name__, wrapper)
        # Note we are not binding func, but wrapper which accepts self but does exactly the same as func
        return func # returning func means func can still be used normally
    return decorator

@add_method(PdmObject)
def _execute_command(self, **command_params):
    self.__warnings = []
    response, call = self._commands.Execute.with_call(Cmd.CommandParams(**command_params))
    for key, value in call.trailing_metadata():
        value = value.replace(';;', '\n')
        if key == 'warning':
            self.__warnings.append(value)

    return response

@add_method(PdmObject)
def __custom_init__(self, pb2_object, channel):
    self.__warnings = []
    self.__keyword_translation = {}

    if pb2_object is not None:
        self._pb2_object = pb2_object
    else:
        self._pb2_object = PdmObject_pb2.PdmObject(class_keyword=self.__class__.__name__)

    self._channel = channel
        
    if self.pb2_object() is not None and self.channel() is not None:        
        if self.channel() is not None:
            self._pdm_object_stub = PdmObject_pb2_grpc.PdmObjectServiceStub(self.channel())
            self._commands = CmdRpc.CommandsStub(self.channel())
    
        for camel_keyword in self._pb2_object.parameters:
            snake_keyword = camel_to_snake(camel_keyword)
            setattr(self, snake_keyword, self.__get_grpc_value(camel_keyword))
            self.__keyword_translation[snake_keyword] = camel_keyword   

@add_method(PdmObject)
def copy_from(self, object):
    """Copy attribute values from object to self and requires that the attribute already exists           
    """
    for attribute in dir(object):
        if not attribute.startswith('__') and hasattr(self, attribute):
            value = getattr(object, attribute)
            if not callable(value):
                setattr(self, attribute, value)

@add_method(PdmObject)
def cast(self, class_definition):
    new_object = class_definition(self.pb2_object(), self.channel())
    new_object.copy_from(self)
    return new_object

@add_method(PdmObject)
def warnings(self):
    return self.__warnings

@add_method(PdmObject)
       
def has_warnings(self):
    return len(self.__warnings) > 0

@add_method(PdmObject)
def pb2_object(self):
    """ Private method"""
    return self._pb2_object

@add_method(PdmObject)
def channel(self):
    """ Private method"""
    return self._channel

@add_method(PdmObject)
def address(self):
    """Get the unique address of the PdmObject

    Returns:
        A 64-bit unsigned integer address
    """

    return self._pb2_object.address

@add_method(PdmObject)
def class_keyword(self):
    """Get the class keyword in the ResInsight Data Model for the given PdmObject"""
    return self._pb2_object.class_keyword

@add_method(PdmObject)
def set_visible(self, visible):
    """Set the visibility of the object in the ResInsight project tree"""
    self._pb2_object.visible = visible

@add_method(PdmObject)
def visible(self):
    """Get the visibility of the object in the ResInsight project tree"""
    return self._pb2_object.visible        

@add_method(PdmObject)
def print_object_info(self):
    """Print the structure and data content of the PdmObject"""
    print("=========== " + self.class_keyword() + " =================")
    print("Object Attributes: ")
    for snake_kw, camel_kw in self.__keyword_translation.items():
        print("   " + snake_kw + " [" + type(getattr(self, snake_kw)).__name__ +
                "]: " + str(getattr(self, snake_kw)))
    print("Object Methods:")
    for method in dir(self):
        if not method.startswith("_") and callable(getattr(self, method)):
            print ("   " + method)

@add_method(PdmObject)
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

@add_method(PdmObject)
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

@add_method(PdmObject)
def __get_grpc_value(self, camel_keyword):
    return self.__convert_from_grpc_value(self._pb2_object.parameters[camel_keyword])

@add_method(PdmObject)
def __set_grpc_value(self, camel_keyword, value):
    self._pb2_object.parameters[camel_keyword] = self.__convert_to_grpc_value(value)

@add_method(PdmObject)
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

@add_method(PdmObject)
def __islist(self, value):
    return value.startswith("[") and value.endswith("]")

@add_method(PdmObject)
def __makelist(self, list_string):
    list_string = list_string.lstrip("[")
    list_string = list_string.rstrip("]")
    strings = list_string.split(", ")
    values = []
    for string in strings:
        values.append(self.__convert_from_grpc_value(string))
    return values

@add_method(PdmObject)
def descendants(self, class_keyword_or_class):
    """Get a list of all project tree descendants matching the class keyword
    Arguments:
        class_keyword_or_class[str/Class]: A class keyword matching the type of class wanted or a Class definition

    Returns:
        A list of PdmObjects matching the keyword provided
    """
    class_definition = PdmObject
    class_keyword = ""
    if isinstance(class_keyword_or_class, str):
        class_keyword = class_keyword_or_class
    else:
        assert(inspect.isclass(class_keyword_or_class))
        class_keyword = class_keyword_or_class.__name__
        class_definition = class_keyword_or_class

    request = PdmObject_pb2.PdmDescendantObjectRequest(
        object=self._pb2_object, child_keyword=class_keyword)
    object_list = self._pdm_object_stub.GetDescendantPdmObjects(
        request).objects
    child_list = []
    for pb2_object in object_list:
        pdm_object = PdmObject(pb2_object=pb2_object, channel=self.channel())
        if class_definition.__name__ == PdmObject.__name__:
            child_list.append(pdm_object)    
        else:
            child_list.append(pdm_object.cast(class_definition))
    return child_list

@add_method(PdmObject)
def children(self, child_field, class_definition=PdmObject):
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
        for pb2_object in object_list:
           pdm_object = PdmObject(pb2_object=pb2_object, channel=self.channel())
        if class_definition.__name__ == PdmObject.__name__:
            child_list.append(pdm_object)    
        else:
            child_list.append(pdm_object.cast(class_definition))
        return child_list
    except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.NOT_FOUND:
            return []
        raise e

@add_method(PdmObject)
def ancestor(self, class_keyword_or_class):
    """Find the first ancestor that matches the provided class_keyword
    Arguments:
        class_keyword_or_class[str/Class]: A class keyword matching the type of class wanted or a Class definition
    """
    class_definition = PdmObject
    class_keyword = ""
    if isinstance(class_keyword_or_class, str):
        class_keyword = class_keyword_or_class
    else:
        assert(inspect.isclass(class_keyword_or_class))
        class_keyword = class_keyword_or_class.__name__
        class_definition = class_keyword_or_class

    request = PdmObject_pb2.PdmParentObjectRequest(
        object=self._pb2_object, parent_keyword=class_keyword)
    try:
        pb2_object = self._pdm_object_stub.GetAncestorPdmObject(request)
        pdm_object = PdmObject(pb2_object=pb2_object,
                               channel=self._channel)
        if class_definition.__name__ == PdmObject.__name__:
            return pdm_object
        else:
            return pdm_object.cast(class_definition)
    except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.NOT_FOUND:
            return None
        raise e

@add_method(PdmObject)
def update(self):
    """Sync all fields from the Python Object to ResInsight"""
    if self._pdm_object_stub is not None and self._pb2_object is not None:
        for snake_kw, camel_kw in self.__keyword_translation.items():
            self.__set_grpc_value(camel_kw, getattr(self, snake_kw))

        self._pdm_object_stub.UpdateExistingPdmObject(self._pb2_object)
    else:
        raise Exception("Object is not connected to GRPC service so cannot update ResInsight")
