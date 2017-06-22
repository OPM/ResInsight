#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

import ctypes
import inspect
import re

import sys

class TypeDefinition(object):
    def __init__(self, type_class_or_function, is_return_type, storage_type):
        self.storage_type = storage_type
        self.is_return_type = is_return_type
        self.type_class_or_function = type_class_or_function


REGISTERED_TYPES = {}
""":type: dict[str,TypeDefinition]"""


def _registerType(type_name, type_class_or_function, is_return_type=True, storage_type=None):
    if type_name in REGISTERED_TYPES:
        raise PrototypeError("Type: '%s' already registered!" % type_name)

    REGISTERED_TYPES[type_name] = TypeDefinition(type_class_or_function, is_return_type, storage_type)

    # print("Registered: %s for class: %s" % (type_name, repr(type_class_or_function)))

_registerType("void", None)
_registerType("void*", ctypes.c_void_p)
_registerType("uint", ctypes.c_uint)
_registerType("uint*", ctypes.POINTER(ctypes.c_uint))
_registerType("int", ctypes.c_int)
_registerType("int*", ctypes.POINTER(ctypes.c_int))
_registerType("int64", ctypes.c_int64)
_registerType("int64*", ctypes.POINTER(ctypes.c_int64))
_registerType("size_t", ctypes.c_size_t)
_registerType("size_t*", ctypes.POINTER(ctypes.c_size_t))
_registerType("bool", ctypes.c_bool)
_registerType("bool*", ctypes.POINTER(ctypes.c_bool))
_registerType("long", ctypes.c_long)
_registerType("long*", ctypes.POINTER(ctypes.c_long))
_registerType("char", ctypes.c_char)
_registerType("char*", ctypes.c_char_p)
_registerType("char**", ctypes.POINTER(ctypes.c_char_p))
_registerType("float", ctypes.c_float)
_registerType("float*", ctypes.POINTER(ctypes.c_float))
_registerType("double", ctypes.c_double)
_registerType("double*", ctypes.POINTER(ctypes.c_double))
_registerType("py_object", ctypes.py_object)

PROTOTYPE_PATTERN = "(?P<return>[a-zA-Z][a-zA-Z0-9_*]*) +(?P<function>[a-zA-Z]\w*) *[(](?P<arguments>[a-zA-Z0-9_*, ]*)[)]"

class PrototypeError(Exception):
    pass


class Prototype(object):
    pattern = re.compile(PROTOTYPE_PATTERN)

    def __init__(self, lib, prototype, bind=False):
        super(Prototype, self).__init__()
        self._lib = lib
        self._prototype = prototype
        self._bind = bind
        self._func = None
        self.__name__ = prototype
        self._resolved = False


    def _parseType(self, type_name):
        """Convert a prototype definition type from string to a ctypes legal type."""
        type_name = type_name.strip()

        if type_name in REGISTERED_TYPES:
            type_definition = REGISTERED_TYPES[type_name]
            return type_definition.type_class_or_function, type_definition.storage_type
        raise ValueError("Unknown type: %s" % type_name)


    def shouldBeBound(self):
        return self._bind

    def resolve(self):
        match = re.match(Prototype.pattern, self._prototype)
        if not match:
            raise PrototypeError("Illegal prototype definition: %s\n" % self._prototype)
        else:
            restype = match.groupdict()["return"]
            function_name = match.groupdict()["function"]
            self.__name__ = function_name
            arguments = match.groupdict()["arguments"].split(",")

            try:
                func = getattr(self._lib, function_name)
            except AttributeError:
                raise PrototypeError("Can not find function: %s in library: %s" % (function_name , self._lib))

            if not restype in REGISTERED_TYPES or not REGISTERED_TYPES[restype].is_return_type:
                sys.stderr.write("The type used as return type: %s is not registered as a return type.\n" % restype)

                return_type = self._parseType(restype)

                if inspect.isclass(return_type):
                    sys.stderr.write("  Correct type may be: %s_ref or %s_obj.\n" % (restype, restype))

                return None

            return_type, storage_type = self._parseType(restype)

            func.restype = return_type

            if storage_type is not None:
                func.restype = storage_type

                def returnFunction(result, func, arguments):
                    return return_type(result)

                func.errcheck = returnFunction

            if len(arguments) == 1 and arguments[0].strip() == "":
                func.argtypes = []
            else:
                argtypes = [self._parseType(arg)[0] for arg in arguments]
                if len(argtypes) == 1 and argtypes[0] is None:
                    argtypes = []
                func.argtypes = argtypes

            self._func = func


    def __call__(self, *args):
        if not self._resolved:
            self.resolve()
            self._resolved = True

        if self._func is None:
            raise PrototypeError("Prototype has not been properly resolved!")
        return self._func(*args)

    def __repr__(self):
        bound = ""
        if self.shouldBeBound():
            bound = ", bind=True"

        return 'Prototype("%s"%s)' % (self._prototype, bound)

    @classmethod
    def registerType(cls, type_name, type_class_or_function, is_return_type=True, storage_type=None):
        if storage_type is None and (inspect.isfunction(type_class_or_function)):
          storage_type = ctypes.c_void_p

        _registerType(type_name,
                      type_class_or_function,
                      is_return_type = is_return_type,
                      storage_type   = storage_type)
