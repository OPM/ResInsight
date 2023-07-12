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

import PdmObject_pb2
import PdmObject_pb2_grpc
import Commands_pb2
import Commands_pb2_grpc


from typing import Any, Callable, TypeVar, Tuple, cast, Union, List, Optional, Type
from typing_extensions import ParamSpec, Self


def camel_to_snake(name: str) -> str:
    s1 = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub("([a-z0-9])([A-Z])", r"\1_\2", s1).lower()


def snake_to_camel(name: str) -> str:
    return "".join(word.title() for word in name.split("_"))


F = TypeVar("F", bound=Callable[..., Any])
C = TypeVar("C")


def add_method(cls: C) -> Callable[[F], F]:
    def decorator(func: F) -> F:
        setattr(cls, func.__name__, func)
        return func  # returning func means func can still be used normally

    return decorator


P = ParamSpec("P")
T = TypeVar("T")


def add_static_method(cls: C) -> Callable[[Callable[P, T]], Callable[P, T]]:
    def decorator(func: Callable[P, T]) -> Callable[P, T]:
        @wraps(func)
        def wrapper(*args: P.args, **kwargs: P.kwargs) -> T:
            return func(*args, **kwargs)

        setattr(cls, func.__name__, wrapper)
        # Note we are not binding func, but wrapper which accepts self but does exactly the same as func
        return func  # returning func means func can still be used normally

    return decorator


class PdmObjectBase:
    """
    The ResInsight base class for the Project Data Model
    """

    __custom_init__ = None

    def _execute_command(self, **command_params) -> Any:
        self.__warnings = []
        response, call = self._commands.Execute.with_call(
            Commands_pb2.CommandParams(**command_params)
        )
        for key, value in call.trailing_metadata():
            value = value.replace(";;", "\n")
            if key == "warning":
                self.__warnings.append(value)

        return response

    def __init__(
        self,
        pb2_object: Optional[PdmObject_pb2.PdmObject],
        channel: Optional[grpc.Channel],
    ) -> None:
        self.__warnings = []
        self.__chunk_size = 8160

        self._channel = channel

        # Create stubs
        if self._channel:
            self._pdm_object_stub = PdmObject_pb2_grpc.PdmObjectServiceStub(
                self._channel
            )
            self._commands = Commands_pb2_grpc.CommandsStub(self._channel)

        if pb2_object is not None:
            # Copy parameters from ResInsight
            assert isinstance(pb2_object, PdmObject_pb2.PdmObject)
            self._pb2_object = pb2_object
            for camel_keyword in self._pb2_object.parameters:
                snake_keyword = camel_to_snake(camel_keyword)
                setattr(self, snake_keyword, self.__get_grpc_value(camel_keyword))
        else:
            # Copy parameters from PdmObject defaults
            self._pb2_object = PdmObject_pb2.PdmObject(
                class_keyword=self.__class__.__name__
            )
            self.__copy_to_pb2()

    def copy_from(self, obj: object) -> None:
        """Copy attribute values from object to self"""
        for attribute in dir(obj):
            if not attribute.startswith("__"):
                value = getattr(obj, attribute)
                # This is crucial to avoid overwriting methods
                if not callable(value):
                    setattr(self, attribute, value)
        if self.__custom_init__ is not None:
            self.__custom_init__(self._pb2_object, self._channel)
        self.update()

    def warnings(self) -> List[str]:
        return self.__warnings

    def has_warnings(self) -> bool:
        return len(self.__warnings) > 0

    def __copy_to_pb2(self) -> None:
        if self._pb2_object is not None:
            for snake_kw in dir(self):
                if not snake_kw.startswith("_"):
                    value = getattr(self, snake_kw)
                    # This is crucial to avoid overwriting methods
                    if not callable(value):
                        camel_kw = snake_to_camel(snake_kw)
                        self.__set_grpc_value(camel_kw, value)

    def pb2_object(self) -> PdmObject_pb2.PdmObject:
        """Private method"""
        return self._pb2_object

    def channel(self) -> grpc.Channel:
        """Private method"""
        return self._channel

    def address(self) -> Any:
        """Get the unique address of the PdmObject

        Returns:
            A 64-bit unsigned integer address
        """

        return self._pb2_object.address

    def set_visible(self, visible: bool) -> None:
        """Set the visibility of the object in the ResInsight project tree"""
        self._pb2_object.visible = visible

    def visible(self) -> bool:
        """Get the visibility of the object in the ResInsight project tree"""
        return bool(self._pb2_object.visible)

    def print_object_info(self) -> None:
        """Print the structure and data content of the PdmObject"""
        print("=========== " + self.__class__.__name__ + " =================")
        print("Object Attributes: ")
        for snake_kw in dir(self):
            if not snake_kw.startswith("_") and not callable(getattr(self, snake_kw)):
                camel_kw = snake_to_camel(snake_kw)
                print(
                    "   "
                    + snake_kw
                    + " ["
                    + type(getattr(self, snake_kw)).__name__
                    + "]: "
                    + str(getattr(self, snake_kw))
                )
        print("Object Methods:")
        for snake_kw in dir(self):
            if not snake_kw.startswith("_") and callable(getattr(self, snake_kw)):
                print("   " + snake_kw)

    Value = Union[bool, str, float, int, "ValueArray"]
    ValueArray = List[Value]

    def __convert_from_grpc_value(self, value: str) -> Value:
        if value.lower() == "false":
            return False
        if value.lower() == "true":
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
                value = value.strip('"')
                if self.__islist(value):
                    return self.__makelist(value)
                return value

    def __convert_to_grpc_value(self, value: Any) -> str:
        if isinstance(value, bool):
            if value:
                return "true"
            return "false"
        if isinstance(value, PdmObjectBase):
            return value.__class__.__name__ + ":" + str(value.address())
        if isinstance(value, list):
            list_of_values = []
            for val in value:
                list_of_values.append(self.__convert_to_grpc_value(val))
            return "[" + ", ".join(list_of_values) + "]"
        return str(value)

    def __get_grpc_value(self, camel_keyword: str) -> Value:
        return self.__convert_from_grpc_value(
            self._pb2_object.parameters[camel_keyword]
        )

    def __set_grpc_value(self, camel_keyword: str, value: str) -> None:
        self._pb2_object.parameters[camel_keyword] = self.__convert_to_grpc_value(value)

    def set_value(self, snake_keyword: str, value: object) -> None:
        """Set the value associated with the provided keyword and updates ResInsight

        Arguments:
            keyword(str): A string containing the parameter keyword
            value(varying): A value matching the type of the parameter.
                See keyword documentation and/or print_object_info() to find
                the correct data type.
        """
        setattr(self, snake_keyword, value)
        self.update()

    def __islist(self, value: str) -> bool:
        return value.startswith("[") and value.endswith("]")

    def __makelist(self, list_string: str) -> Value:
        list_string = list_string.lstrip("[")
        list_string = list_string.rstrip("]")
        if not list_string:
            # Return empty list if empty string. Otherwise, the split function will return ['']
            return []
        strings = list_string.split(", ")
        values = []
        for string in strings:
            values.append(self.__convert_from_grpc_value(string))
        return values

    D = TypeVar("D")

    def __from_pb2_to_resinsight_classes(
        self,
        pb2_object_list: List[PdmObject_pb2.PdmObject],
        super_class_definition: Type[D],
    ) -> List[D]:
        pdm_object_list = []
        from .generated.generated_classes import class_from_keyword

        for pb2_object in pb2_object_list:
            child_class_definition = class_from_keyword(pb2_object.class_keyword)
            if child_class_definition is None:
                child_class_definition = super_class_definition

            assert child_class_definition is not None
            pdm_object = child_class_definition(
                pb2_object=pb2_object, channel=self.channel()
            )

            pdm_object_list.append(pdm_object)
        return pdm_object_list

    def descendants(self, class_definition: Type[D]) -> List[D]:
        """Get a list of all project tree descendants matching the class keyword
        Arguments:
            class_definition[class]: A class definition matching the type of class wanted

        Returns:
            A list of PdmObjects matching the class_definition
        """
        assert inspect.isclass(class_definition)

        class_keyword = class_definition.__name__
        try:
            request = PdmObject_pb2.PdmDescendantObjectRequest(
                object=self._pb2_object, child_keyword=class_keyword
            )
            object_list = self._pdm_object_stub.GetDescendantPdmObjects(request).objects
            return self.__from_pb2_to_resinsight_classes(object_list, class_definition)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return []  # Valid empty result
            raise e

    def children(self, child_field: str, class_definition: Type[D]) -> List[D]:
        """Get a list of all direct project tree children inside the provided child_field
        Arguments:
            child_field[str]: A field name
        Returns:
            A list of PdmObjects inside the child_field
        """
        request = PdmObject_pb2.PdmChildObjectRequest(
            object=self._pb2_object, child_field=child_field
        )
        try:
            object_list = self._pdm_object_stub.GetChildPdmObjects(request).objects
            return self.__from_pb2_to_resinsight_classes(object_list, class_definition)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return []
            raise e

    def add_new_object(
        self, class_definition: Type[D], child_field: str = ""
    ) -> Optional[D]:
        """Create and add an object to the specified child field
        Arguments:
            class_definition[class]: Class definition of the object to create
            child_field[str]: The keyword for the field to create a new object in. If empty, the first child array field is used.
        Returns:
            The created PdmObject inside the child_field
        """
        from .generated.generated_classes import class_from_keyword

        assert inspect.isclass(class_definition)

        class_keyword = class_definition.__name__

        request = PdmObject_pb2.CreatePdmChildObjectRequest(
            object=self._pb2_object,
            child_field=child_field,
            class_keyword=class_keyword,
        )
        try:
            pb2_object = self._pdm_object_stub.CreateChildPdmObject(request)
            child_class_definition = class_from_keyword(pb2_object.class_keyword)

            if child_class_definition is None:
                child_class_definition = class_definition

            assert child_class_definition.__name__ == class_definition.__name__
            pdm_object = class_definition(pb2_object=pb2_object, channel=self.channel())

            return pdm_object
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return None
            raise e

    def ancestor(self, class_definition: Type[D]) -> Optional[D]:
        """Find the first ancestor that matches the provided class_keyword
        Arguments:
            class_definition[class]: A class definition matching the type of class wanted
        """
        assert inspect.isclass(class_definition)

        class_keyword = class_definition.__name__

        request = PdmObject_pb2.PdmParentObjectRequest(
            object=self._pb2_object, parent_keyword=class_keyword
        )
        try:
            pb2_object = self._pdm_object_stub.GetAncestorPdmObject(request)
            pdm_object = class_definition(pb2_object=pb2_object, channel=self.channel())
            return pdm_object
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return None
            raise e

    def _call_get_method_async(self, method_name: str):
        request = PdmObject_pb2.PdmObjectGetterRequest(
            object=self._pb2_object, method=method_name
        )
        for chunk in self._pdm_object_stub.CallPdmObjectGetter(request):
            yield chunk

    def _call_get_method(self, method_name: str):
        all_values = []
        generator = self._call_get_method_async(method_name)
        for chunk in generator:
            data = getattr(chunk, chunk.WhichOneof("data"))
            for value in data.data:
                all_values.append(value)
        return all_values

    def __generate_set_method_chunks(self, array, method_request):
        index = -1

        while index < len(array):
            chunk = PdmObject_pb2.PdmObjectSetterChunk()
            if index == -1:
                chunk.set_request.CopyFrom(
                    PdmObject_pb2.PdmObjectSetterRequest(
                        request=method_request, data_count=len(array)
                    )
                )
                index += 1
            else:
                actual_chunk_size = min(len(array) - index + 1, self.__chunk_size)
                if isinstance(array[0], float):
                    chunk.CopyFrom(
                        PdmObject_pb2.PdmObjectSetterChunk(
                            doubles=PdmObject_pb2.DoubleArray(
                                data=array[index : index + actual_chunk_size]
                            )
                        )
                    )
                elif isinstance(array[0], int):
                    chunk.CopyFrom(
                        PdmObject_pb2.PdmObjectSetterChunk(
                            ints=PdmObject_pb2.IntArray(
                                data=array[index : index + actual_chunk_size]
                            )
                        )
                    )
                elif isinstance(array[0], str):
                    chunk.CopyFrom(
                        PdmObject_pb2.PdmObjectSetterChunk(
                            strings=PdmObject_pb2.StringArray(
                                data=array[index : index + actual_chunk_size]
                            )
                        )
                    )
                else:
                    raise Exception("Wrong data type for set method")
                index += actual_chunk_size
            yield chunk
        # Final empty message to signal completion
        chunk = PdmObject_pb2.PdmObjectSetterChunk()
        yield chunk

    def _call_set_method(self, method_name: str, values) -> None:
        method_request = PdmObject_pb2.PdmObjectGetterRequest(
            object=self._pb2_object, method=method_name
        )
        request_iterator = self.__generate_set_method_chunks(values, method_request)
        reply = self._pdm_object_stub.CallPdmObjectSetter(request_iterator)
        if reply.accepted_value_count < len(values):
            raise IndexError

    def _call_pdm_method_void(self, method_name: str, **kwargs: Any) -> None:
        pb2_params = PdmObject_pb2.PdmObject(class_keyword=method_name)
        for key, value in kwargs.items():
            pb2_params.parameters[snake_to_camel(key)] = self.__convert_to_grpc_value(
                value
            )
        request = PdmObject_pb2.PdmObjectMethodRequest(
            object=self._pb2_object, method=method_name, params=pb2_params
        )

        self._pdm_object_stub.CallPdmObjectMethod(request)

    X = TypeVar("X")

    def _call_pdm_method_return_value(
        self, method_name: str, class_definition: Type[X], **kwargs: Any
    ) -> X:
        pb2_params = PdmObject_pb2.PdmObject(class_keyword=method_name)
        for key, value in kwargs.items():
            pb2_params.parameters[snake_to_camel(key)] = self.__convert_to_grpc_value(
                value
            )
        request = PdmObject_pb2.PdmObjectMethodRequest(
            object=self._pb2_object, method=method_name, params=pb2_params
        )

        pb2_object = self._pdm_object_stub.CallPdmObjectMethod(request)

        pdm_object = class_definition(pb2_object=pb2_object, channel=self.channel())
        return pdm_object

    O = TypeVar("O")

    def _call_pdm_method_return_optional_value(
        self, method_name: str, class_definition: Type[O], **kwargs: Any
    ) -> Optional[O]:
        pb2_params = PdmObject_pb2.PdmObject(class_keyword=method_name)
        for key, value in kwargs.items():
            pb2_params.parameters[snake_to_camel(key)] = self.__convert_to_grpc_value(
                value
            )
        request = PdmObject_pb2.PdmObjectMethodRequest(
            object=self._pb2_object, method=method_name, params=pb2_params
        )

        pb2_object = self._pdm_object_stub.CallPdmObjectMethod(request)

        from .generated.generated_classes import class_from_keyword

        child_class_definition = class_from_keyword(pb2_object.class_keyword)
        if child_class_definition is None:
            return None

        assert class_definition.__name__ == child_class_definition.__name__
        pdm_object = class_definition(pb2_object=pb2_object, channel=self.channel())
        return pdm_object

    def update(self) -> None:
        """Sync all fields from the Python Object to ResInsight"""
        self.__copy_to_pb2()
        if self._pdm_object_stub is not None:
            self._pdm_object_stub.UpdateExistingPdmObject(self._pb2_object)
        else:
            raise Exception(
                "Object is not connected to GRPC service so cannot update ResInsight"
            )
