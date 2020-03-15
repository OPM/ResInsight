# pylint: disable=no-self-use
"""
ResInsight caf::PdmObject connection module
"""

from functools import partial, wraps
import grpc
import re
import builtins
import importlib
import inspect
import sys

import rips.generated.PdmObject_pb2 as PdmObject_pb2
import rips.generated.PdmObject_pb2_grpc as PdmObject_pb2_grpc
import rips.generated.Commands_pb2 as Cmd
import rips.generated.Commands_pb2_grpc as CmdRpc
from rips.generated.pdm_objects import PdmObject, class_from_keyword

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def snake_to_camel(name):
    return ''.join(word.title() for word in name.split('_'))

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
    self.__chunk_size = 8160

    self._channel = channel
    
    # Create stubs
    if self._channel:
        self._pdm_object_stub = PdmObject_pb2_grpc.PdmObjectServiceStub(self._channel)
        self._commands = CmdRpc.CommandsStub(self._channel)

    if pb2_object is not None:
        # Copy parameters from ResInsight
        assert(not isinstance(pb2_object, PdmObject))
        self._pb2_object = pb2_object        
        for camel_keyword in self._pb2_object.parameters:
            snake_keyword = camel_to_snake(camel_keyword)
            setattr(self, snake_keyword, self.__get_grpc_value(camel_keyword))
    else:
        # Copy parameters from PdmObject defaults
        self._pb2_object = PdmObject_pb2.PdmObject(class_keyword=self.__class__.__name__)
        self.__copy_to_pb2()

@add_method(PdmObject)
def copy_from(self, object):
    """Copy attribute values from object to self
    """
    for attribute in dir(object):
        if not attribute.startswith('__'):
            value = getattr(object, attribute)
            # This is crucial to avoid overwriting methods
            if not callable(value):
                setattr(self, attribute, value)  
    if self.__custom_init__ is not None:
        self.__custom_init__(self._pb2_object, self._channel)

@add_method(PdmObject)
def warnings(self):
    return self.__warnings

@add_method(PdmObject)       
def has_warnings(self):
    return len(self.__warnings) > 0

@add_method(PdmObject)
def __copy_to_pb2(self):
    if self._pb2_object is not None:
        for snake_kw in dir(self):
            if not snake_kw.startswith('_'):
                value = getattr(self, snake_kw)
                # This is crucial to avoid overwriting methods
                if not callable(value):
                    camel_kw = snake_to_camel(snake_kw)
                    self.__set_grpc_value(camel_kw, value)

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
    print("=========== " + self.__class__.__name__ + " =================")
    print("Object Attributes: ")
    for snake_kw in dir(self):
        if not snake_kw.startswith("_") and not callable(getattr(self, snake_kw)):
            camel_kw = snake_to_camel(snake_kw)
            print("   " + snake_kw + " [" + type(getattr(self, snake_kw)).__name__ +
                    "]: " + str(getattr(self, snake_kw)))
    print("Object Methods:")
    for snake_kw in dir(self):
        if not snake_kw.startswith("_") and callable(getattr(self, snake_kw)):
            print ("   " + snake_kw)

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
def __from_pb2_to_pdm_objects(self, pb2_object_list, super_class_definition):
    pdm_object_list = []
    for pb2_object in pb2_object_list:
        child_class_definition = class_from_keyword(pb2_object.class_keyword)
        if child_class_definition is None:
            child_class_definition = super_class_definition

        pdm_object = child_class_definition(pb2_object=pb2_object, channel=self.channel())
        pdm_object_list.append(pdm_object)    
    return pdm_object_list

@add_method(PdmObject)
def descendants(self, class_definition):
    """Get a list of all project tree descendants matching the class keyword
    Arguments:
        class_definition[class]: A class definition matching the type of class wanted

    Returns:
        A list of PdmObjects matching the class_definition
    """
    assert(inspect.isclass(class_definition))

    class_keyword = class_definition.__name__
    try:
        request = PdmObject_pb2.PdmDescendantObjectRequest(
            object=self._pb2_object, child_keyword=class_keyword)
        object_list = self._pdm_object_stub.GetDescendantPdmObjects(
            request).objects
        return self.__from_pb2_to_pdm_objects(object_list, class_definition)
    except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.NOT_FOUND:
            return [] # Valid empty result
        raise e   

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
        return self.__from_pb2_to_pdm_objects(object_list, class_definition)
    except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.NOT_FOUND:
            return []
        raise e

@add_method(PdmObject)
def ancestor(self, class_definition):
    """Find the first ancestor that matches the provided class_keyword
    Arguments:
        class_definition[class]: A class definition matching the type of class wanted
    """
    assert(inspect.isclass(class_definition))

    class_keyword = class_definition.__name__

    request = PdmObject_pb2.PdmParentObjectRequest(
        object=self._pb2_object, parent_keyword=class_keyword)
    try:
        pb2_object = self._pdm_object_stub.GetAncestorPdmObject(request)
        child_class_definition = class_from_keyword(pb2_object.class_keyword)

        if child_class_definition is None:
            child_class_definition = class_definition

        pdm_object = child_class_definition(pb2_object=pb2_object, channel=self.channel())
        return pdm_object
    except grpc.RpcError as e:
        if e.code() == grpc.StatusCode.NOT_FOUND:
            return None
        raise e

@add_method(PdmObject)
def _call_get_method_async(self, method_name):
    request = PdmObject_pb2.PdmObjectMethodRequest(object=self._pb2_object, method=method_name)
    for chunk in self._pdm_object_stub.CallPdmObjectGetMethod(request):
        yield chunk

@add_method(PdmObject)
def _call_get_method(self, method_name):
    all_values = []
    generator = self._call_get_method_async(method_name)
    for chunk in generator:
        data = getattr(chunk, chunk.WhichOneof('data'))
        for value in data.data:
            all_values.append(value)
    return all_values

@add_method(PdmObject)
def __generate_set_method_chunks(self, array, method_request):
    index = -1

    while index < len(array):
        chunk = PdmObject_pb2.PdmObjectSetMethodChunk()
        if index is -1:
            chunk.set_request.CopyFrom(PdmObject_pb2.PdmObjectSetMethodRequest(request=method_request, data_count=len(array)))
            index += 1
        else:
            actual_chunk_size = min(len(array) - index + 1, self.__chunk_size)
            if isinstance(array[0], float):
                chunk.CopyFrom(
                    PdmObject_pb2.PdmObjectSetMethodChunk(doubles=PdmObject_pb2.DoubleArray(data=array[index:index +
                                                            actual_chunk_size])))
            elif isinstance(array[0], int):
                chunk.CopyFrom(
                    PdmObject_pb2.PdmObjectSetMethodChunk(ints=PdmObject_pb2.IntArray(data=array[index:index +
                                                            actual_chunk_size])))
            elif isinstance(array[0], str):
                chunk.CopyFrom(
                    PdmObject_pb2.PdmObjectSetMethodChunk(strings=PdmObject_pb2.StringArray(data=array[index:index +
                                                            actual_chunk_size])))
            else:
                raise Exception("Wrong data type for set method")
            index += actual_chunk_size
        yield chunk
    # Final empty message to signal completion
    chunk = PdmObject_pb2.PdmObjectSetMethodChunk()
    yield chunk

@add_method(PdmObject)
def _call_set_method(self, method_name, values):
    method_request = PdmObject_pb2.PdmObjectMethodRequest(object=self._pb2_object, method=method_name)
    request_iterator = self.__generate_set_method_chunks(values, method_request)
    reply = self._pdm_object_stub.CallPdmObjectSetMethod(request_iterator)
    if reply.accepted_value_count < len(values):
        raise IndexError

@add_method(PdmObject)
def update(self):
    """Sync all fields from the Python Object to ResInsight"""
    self.__copy_to_pb2()
    if self._pdm_object_stub is not None:
        self._pdm_object_stub.UpdateExistingPdmObject(self._pb2_object)
    else:
        raise Exception("Object is not connected to GRPC service so cannot update ResInsight")
