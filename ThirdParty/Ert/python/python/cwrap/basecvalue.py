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

from __future__ import (absolute_import, division,
                        print_function, unicode_literals)

import six

from ctypes import (pointer, c_long, c_int, c_bool, c_float, c_double, c_byte,
                    c_short, c_char, c_ubyte, c_ushort, c_uint, c_ulong)

from .metacwrap import MetaCWrap

@six.add_metaclass(MetaCWrap)
class BaseCValue(object):
    DATA_TYPE = None
    LEGAL_TYPES = [c_byte, c_ubyte, c_short, c_ushort, c_int, c_uint, c_long, c_ulong, c_bool, c_char, c_float, c_double]

    def __init__(self, value):
        super(BaseCValue, self).__init__()

        if not self.DATA_TYPE in self.LEGAL_TYPES:
            raise ValueError("DATA_TYPE must be one of these CTypes classes: %s" % BaseCValue.LEGAL_TYPES)

        self.__value = self.cast(value)


    def value(self):
        return self.__value.value

    @classmethod
    def storageType(cls):
        return cls.type()

    @classmethod
    def type(cls):
        return cls.DATA_TYPE

    @classmethod
    def cast(cls, value):
        return cls.DATA_TYPE(value)

    def setValue(self, value):
        self.__value = self.cast(value)

    def asPointer(self):
        return pointer(self.__value)

    @classmethod
    def from_param(cls, c_value_object):
        if c_value_object is not None and not isinstance(c_value_object, BaseCValue):
            raise ValueError("c_class_object must be a BaseCValue instance!")

        return c_value_object.__value
