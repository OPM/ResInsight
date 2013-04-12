#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'erttypes.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import datetime
import time
import ctypes
#from   ert.util.tvector import DoubleVector


class time_t(ctypes.c_long):
    """A convenience class for working with time_t objects."""

    def time(self):
        """Return this time_t as a time.localtime() object"""
        return time.localtime(self.value)

    def datetime(self):
        """Return this time_t as a datetime.date([year, month, day])"""
        return datetime.date(*self.time()[0:3])

    def __str__(self):
        return "%d %s" % (self.value, str(self.datetime()))

    def __ge__(self, other):
        return self.value >= other.value

    def __lt__(self, other):
        return not self >= other

        
class VectorIterator:
    """A simple iterator"""
    def __init__(self, data, size):
        self.index = 0
        self.data  = data
        self.size  = size
        
    def next(self):
        if self.index == self.size:
            raise StopIteration
        result = self.data[self.index]
        self.index += 1
        return result


class time_vector(ctypes.c_long):
    """Represents a vector of time_t objects"""
    initialized = False
    lib = None

    def __getitem__(self, item):
        """Indexing support"""
        return self.__class__.lib.time_t_vector_iget(self, item)

    def __iter__(self):
        """Iterator support"""
        return VectorIterator(self, self.size())

    def __del__(self):
        """Garbage collection"""
        self._free()

    def size(self):
        """The size of this vector"""
        return self.__class__.lib.time_t_vector_size(self)

    def _free(self):
        self.__class__.lib.time_t_vector_free(self)

    def getPointer(self):
        """Returns the internal pointer for this instance."""
        return self.__class__.lib.time_t_vector_get_ptr(self)

    @classmethod
    def initialize(cls, ert):
        if not cls.initialized:
            cls.lib = ert.util
            ert.registerType("time_vector", time_vector)
            ert.prototype("int time_t_vector_size(time_vector)", lib=ert.util)
            ert.prototype("time_t time_t_vector_iget(time_vector, int)", lib=ert.util)
            ert.prototype("long time_t_vector_get_ptr(time_vector)", lib=ert.util)
            ert.prototype("void time_t_vector_free(time_vector)", lib=ert.util)

            cls.initialized = True


class double_vector(ctypes.c_long):
    """Represents a vector of double objects"""
    initialized = False
    lib = None

    def __getitem__(self, item):
        """Indexing support"""
        return self.__class__.lib.double_vector_iget(self, item)

    def __iter__(self):
        """Iterator support"""
        return VectorIterator(self, self.size())

    def __del__(self):
        """Garbage collection"""
        self._free()

    def size(self):
        """The size of this vector"""
        return self.__class__.lib.double_vector_size(self)

    def _free(self):
        self.__class__.lib.double_vector_free(self)

    def getPointer(self):
        """Returns the internal pointer for this instance."""
        return self.__class__.lib.double_vector_get_ptr(self)

    @classmethod
    def initialize(cls, ert):
        if not cls.initialized:
            cls.lib = ert.util
            ert.registerType("double_vector", double_vector)
            ert.prototype("int double_vector_size(double_vector)", lib=ert.util)
            ert.prototype("double double_vector_iget(double_vector, int)", lib=ert.util)
            ert.prototype("long double_vector_get_ptr(double_vector)", lib=ert.util)
            ert.prototype("void double_vector_free(double_vector)", lib=ert.util)

            cls.initialized = True
