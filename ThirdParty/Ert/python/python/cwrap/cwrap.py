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

"""
Module implementing type map between C types and Python classes.
The prototype_pattern is a major regexp which is used to set the
correct restype and argtypes attributes of the function objects.
"""

from __future__ import print_function
import ctypes
import re
import sys
import inspect

from .basecclass import BaseCClass
from .basecvalue import BaseCValue
from .prototype import REGISTERED_TYPES



prototype_pattern = "(?P<return>[a-zA-Z][a-zA-Z0-9_*]*) +(?P<function>[a-zA-Z]\w*) *[(](?P<arguments>[a-zA-Z0-9_*, ]*)[)]"

def isBoundMethod(func, bound_to_class=None):
    if bound_to_class is None:
        return hasattr(func, "__self__")
    else:
        return hasattr(func, "__self__") and issubclass(func.__self__, bound_to_class)


class CWrapError(Exception):
    pass

class CWrapper:
    # Observe that registered_types is a class attribute, shared
    # between all CWrapper instances.
    registered_types = {}
    pattern = re.compile(prototype_pattern)

    def __init__(self, lib):
        self.__lib = lib

    @classmethod
    def registerType(cls, type_name, value):
        """Register a type against a legal ctypes type or a callable (or class)"""
        # if type_name in cls.registered_types:
        #     print("Type %s already exists!" % type_name)
        cls.registered_types[type_name] = value

    @classmethod
    def registerObjectType(cls, type_name, base_c_class):
        """
        Automatically registers a class type with object and reference versions.
        For example:
            string_list -> StringList
            string_list_ref -> StringList.createCReference
            string_list_obj -> StringList.createPythonObject
        @type type_name: str
        @type base_c_class: BaseCClass
        """
        assert issubclass(base_c_class, BaseCClass)

        cls.registerType(type_name, base_c_class)
        cls.registerType("%s_ref" % type_name, base_c_class.createCReference)
        cls.registerType("%s_obj" % type_name, base_c_class.createPythonObject)


    @classmethod
    def registerDefaultTypes(cls):
        """Registers the default available types for prototyping."""
        cls.registerType("void", None)
        cls.registerType("void*", ctypes.c_void_p)
        cls.registerType("uint", ctypes.c_uint)
        cls.registerType("uint*", ctypes.POINTER(ctypes.c_uint))
        cls.registerType("int", ctypes.c_int)
        cls.registerType("int*", ctypes.POINTER(ctypes.c_int))
        cls.registerType("int64", ctypes.c_int64)
        cls.registerType("int64*", ctypes.POINTER(ctypes.c_int64))
        cls.registerType("size_t", ctypes.c_size_t)
        cls.registerType("size_t*", ctypes.POINTER(ctypes.c_size_t))
        cls.registerType("bool", ctypes.c_bool)
        cls.registerType("bool*", ctypes.POINTER(ctypes.c_bool))
        cls.registerType("long", ctypes.c_long)
        cls.registerType("long*", ctypes.POINTER(ctypes.c_long))
        cls.registerType("char", ctypes.c_char)
        cls.registerType("char*", ctypes.c_char_p)
        cls.registerType("char**", ctypes.POINTER(ctypes.c_char_p))
        cls.registerType("float", ctypes.c_float)
        cls.registerType("float*", ctypes.POINTER(ctypes.c_float))
        cls.registerType("double", ctypes.c_double)
        cls.registerType("double*", ctypes.POINTER(ctypes.c_double))


    def __parseType(self, type_name):
        """Convert a prototype definition type from string to a ctypes legal type."""
        type_name = type_name.strip()

        if type_name in CWrapper.registered_types:
            return CWrapper.registered_types[type_name]
        elif type_name in REGISTERED_TYPES:
            return REGISTERED_TYPES[type_name].type_class_or_function
        else:
            return getattr(ctypes, type_name)


    def prototype(self, prototype, lib=None):
        """
        Defines the return type and arguments for a C-function
        prototype expects a string formatted like this:
            "type functionName(type, ... ,type)"
        where type is a type available to ctypes
        Some type are automatically converted:
            int  -> c_int
            long -> c_long
            char -> c_char_p
            bool -> c_int
            void -> None
            double -> c_double
            float  -> c_float
        There are also pointer versions of these:
            long* -> POINTER(c_long)
            bool* -> POINTER(c_int)
            double* -> POINTER(c_double)
            char* -> c_char_p
            ...
        In addition, user register types are recognized and any type
        registered as a reference to BaseCClass createCReference and
        createPythonObject are treated as pointers and converted
        automatically.
        """

        match = re.match(CWrapper.pattern, prototype)
        if not match:
            raise CWrapError("Illegal prototype definition: %s\n" % prototype)
        else:
            restype = match.groupdict()["return"]
            function_name = match.groupdict()["function"]
            arguments = match.groupdict()["arguments"].split(",")
            
            try:
                func = getattr(self.__lib, function_name)
            except AttributeError:
                raise CWrapError("Can not find function: %s in library: %s" % (function_name , self.__lib))
                
            return_type = self.__parseType(restype)

            if inspect.isclass(return_type) and issubclass(return_type, BaseCClass):
                sys.stderr.write("BaseCClass can not be used as a return type in prototype definition: %s\n" % prototype)
                sys.stderr.write("  Correct return type may be: %s_ref or %s_obj" % (restype, restype))
                return None

            func.restype = return_type

            if inspect.isclass(return_type):
                if issubclass(return_type, BaseCValue):
                    self.setReturnBehavior(func, return_type)
                else:
                    pass # Use default behavior for BaseCEnum and ctypes classes
            elif callable(return_type):
                if isBoundMethod(return_type, BaseCClass) or not isBoundMethod(return_type):
                    self.setReturnBehavior(func, return_type)
                else:
                    pass #Methods bound to anything else than BaseCClass
            else:
                if return_type is not None:
                    raise CWrapError("Unknown return type: %s" % return_type)

            if len(arguments) == 1 and arguments[0].strip() == "":
                func.argtypes = []
            else:
                argtypes = [self.__parseType(arg) for arg in arguments]
                if len(argtypes) == 1 and argtypes[0] is None:
                    argtypes = []
                func.argtypes = argtypes

            return func

    @staticmethod
    def setReturnBehavior(func, return_type):
        if inspect.isclass(return_type) and issubclass(return_type, BaseCValue):
            func.restype = return_type.type()
        else:
            func.restype = ctypes.c_void_p

        def returnFunction(result, func, arguments):
            return return_type(result)

        func.errcheck = returnFunction

    def printTypes(self):
        for ctype in self.registered_types.keys():
            print('%16s -> %s' % (ctype, self.registered_types[ctype]))


class CWrapperNameSpace:
    def __init__( self, name ):
        self.name = name

    def __str__(self):
        return "%s wrapper" % self.name


CWrapper.registerDefaultTypes()
