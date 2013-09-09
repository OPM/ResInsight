#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'cwrap.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import ctypes
import re
import sys


prototype_pattern = "(?P<return>[a-zA-Z][a-zA-Z0-9_*]*) +(?P<function>[a-zA-Z]\w*) *[(](?P<arguments>[a-zA-Z0-9_*, ]*)[)]"


class CWrapper:
    # Observe that registered_types is a class attribute, shared
    # between all CWrapper instances.
    registered_types = {}
    pattern = re.compile(prototype_pattern)

    def __init__( self, lib ):
        self.lib = lib

    @classmethod
    def registerType(cls, type_name, value):
        """Register a type against a legal ctypes type"""
        cls.registered_types[type_name] = value

    @classmethod
    def registerDefaultTypes(cls):
        """Registers the default available types for prototyping."""
        cls.registerType("void", None)
        cls.registerType("c_ptr", ctypes.c_void_p)
        cls.registerType("int", ctypes.c_int)
        cls.registerType("int*", ctypes.POINTER(ctypes.c_int))
        cls.registerType("size_t", ctypes.c_size_t)
        cls.registerType("size_t*", ctypes.POINTER(ctypes.c_size_t))
        cls.registerType("bool", ctypes.c_int)
        cls.registerType("bool*", ctypes.POINTER(ctypes.c_int))
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

        if CWrapper.registered_types.has_key(type_name):
            return CWrapper.registered_types[type_name]
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
        """

        match = re.match(CWrapper.pattern, prototype)
        if not match:
            sys.stderr.write("Illegal prototype definition: %s\n" % (prototype))
            return None
        else:
            restype = match.groupdict()["return"]
            functioname = match.groupdict()["function"]
            arguments = match.groupdict()["arguments"].split(",")

            func = getattr(self.lib, functioname)
            func.restype = self.__parseType(restype)

            if len(arguments) == 1 and arguments[0].strip() == "":
                func.argtypes = []
            else:
                argtypes = [self.__parseType(arg) for arg in arguments]
                if len(argtypes) == 1 and argtypes[0] is None:
                    argtypes = []
                func.argtypes = argtypes

            return func

    def safe_prototype(self, pattern, lib=None):
        try:
            func = self.prototype(pattern, lib)
        except AttributeError:
            func = None
            sys.stderr.write("****Defunct function call: %s\n" % pattern)
        return func


    def print_types(self):
        for ctype in self.registered_types.keys():
            print "%16s -> %s" % (ctype, self.registered_types[ctype])


class CWrapperNameSpace:
    def __init__( self, name ):
        self.name = name

    def __str__(self):
        return "%s wrapper" % self.name


CWrapper.registerDefaultTypes()
