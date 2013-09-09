#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'c_enums.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ctypes import *

class EnumerationType(type(c_uint)):
    def __new__(metacls, name, bases, dict):
        if not "_members_" in dict:
            _members_ = {}
            for key,value in dict.items():
                if not key.startswith("_"):
                    _members_[key] = value
            dict["_members_"] = _members_
        cls = type(c_uint).__new__(metacls, name, bases, dict)
        for key,value in cls._members_.items():
            globals()[key] = value
        return cls

    def __contains__(self, value):
        return value in self._members_.values()

    def __repr__(self):
        return "<Enumeration %s>" % self.__name__

class Enumeration(c_uint):
    __metaclass__ = EnumerationType
    _members_ = {}
    def __init__(self, value):
        for k,v in self._members_.items():
            if v == value:
                self.name = k
                break
        else:
            raise ValueError("No enumeration member with value %r" % value)
        c_uint.__init__(self, value)
        

    @classmethod
    def from_param(cls, param):
        if isinstance(param, Enumeration):
            if param.__class__ != cls:
                raise ValueError("Cannot mix enumeration members")
            else:
                return param
        else:
            return cls(param)

    def __repr__(self):
        return "<member %s=%d of %r>" % (self.name, self.value, self.__class__)
#-------------------------------------------------------------------
#    enum implementations
#-------------------------------------------------------------------

class state_enum(Enumeration):
    UNDEFINED  = 0
    SERIALIZED = 1
    FORECAST   = 2
    ANALYZED   = 4
    BOTH       = 6

class var_type(Enumeration):
    INVALID_VAR      =  0
    PARAMETER        =  1
    DYNAMIC_STATE    =  2
    DYNAMIC_RESULT   =  4
    STATIC_STATE     =  8
    INDEX_STATE      = 16
