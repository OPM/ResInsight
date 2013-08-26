#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ctime.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import math
import ctypes
import types
import datetime
import time
from ert.cwrap import CWrapper


class ctime(ctypes.c_long):
    def __init__(self, value):
        if isinstance(value, types.IntType):
            self.value = value
        else:
            try:
                # Input value is assumed to be datetime.datetime instance
                self.value = int(math.floor(time.mktime(
                    (value.year, value.month, value.day, value.hour, value.minute, value.second, 0, 0, -1 ))))
            except (OverflowError, ValueError, AttributeError):
                # Input value is assumed to be datetime.date instance
                self.value = int(math.floor(time.mktime((value.year, value.month, value.day, 0, 0, 0, 0, 0, -1 ))))


    def ctime(self):
        return self.value

    def time(self):
        """Return this time_t as a time.localtime() object"""
        return time.localtime(self.value)

    def date(self):
        """Return this time_t as a datetime.date([year, month, day])"""
        return datetime.date(*self.time()[0:3])

    def datetime(self):
        return datetime.datetime(*self.time()[0:6])

    def __str__(self):
        return "%s" % (str(self.datetime()))

    def __ge__(self, other):
        return self.value >= other.value

    def __lt__(self, other):
        return not self >= other

    def __eq__(self, other):
        return self.value == other.value

    @property
    def stripped(self):
        return time.strptime(self, "%Y-%m-%d %H:%M:S%")


cwrapper = CWrapper(None)
cwrapper.registerType("time_t", ctime)
cwrapper.registerType("time_t*", ctypes.POINTER(ctime))

