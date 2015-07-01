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
import datetime
import time
from types import NoneType
from ert.cwrap import CWrapper, BaseCValue


class CTime(BaseCValue):
    DATA_TYPE = ctypes.c_long

    def __init__(self, value):
        if isinstance(value, int):
            value = value
        elif isinstance(value, CTime):
            value = value.value()
        elif isinstance(value, datetime.datetime):
            value = int(math.floor(time.mktime((value.year, value.month, value.day, value.hour, value.minute, value.second, 0, 0, -1 ))))
        elif isinstance(value, datetime.date):
            value = int(math.floor(time.mktime((value.year, value.month, value.day, 0, 0, 0, 0, 0, -1 ))))
        else:
            raise NotImplementedError("Can not convert class %s to CTime" % value.__class__)

        super(CTime, self).__init__(value)


    def ctime(self):
        """ @rtype: int """
        return self.value()

    def time(self):
        """Return this time_t as a time.localtime() object"""
        return time.localtime(self.value())

    def date(self):
        """Return this time_t as a datetime.date([year, month, day])"""
        return datetime.date(*self.time()[0:3])

    def datetime(self):
        return datetime.datetime(*self.time()[0:6])

    def __str__(self):
        return "%s" % (str(self.datetime()))

    def __ge__(self, other):
        return self > other or self == other

    def __le__(self, other):
        return self < other or self == other

    def __gt__(self, other):
        if isinstance(other, CTime):
            return self.value() > other.value()
        elif isinstance(other, (int, datetime.datetime, datetime.date)):
            return self > CTime(other)
        else:
            raise TypeError("CTime does not support type: %s" % other.__class__)

    def __lt__(self, other):
        if isinstance(other, CTime):
            return self.value() < other.value()
        elif isinstance(other, (int, datetime.datetime, datetime.date)):
            return self < CTime(other)
        else:
            raise TypeError("CTime does not support type: %s" % other.__class__)

    def __ne__(self, other):
        return not self == other

    def __eq__(self, other):
        if isinstance(other, CTime):
            return self.value() == other.value()
        elif isinstance(other, (int, datetime.datetime, datetime.date)):
            return self == CTime(other)
        elif isinstance(other, type(None)):
            return False
        else:
            raise TypeError("CTime does not support type: %s" % other.__class__)
            
    def __imul__(self, other):
        value = int(self.value() * other)
        self.setValue(value)
        return self

    def __hash__(self):
        return hash(self.value())

    def __iadd__(self , other):
        if isinstance(other, CTime):
            self.setValue(self.value() + other.value())
            return self
        else:
            self.setValue(self.value() + CTime(other).value())
            return self

    def __add__(self, other):
        copy = CTime( self )
        copy += other
        return copy

    def __radd__(self, other):
        return self + other


    def __mul__(self , other):
        copy = CTime( self )
        copy *= other
        return copy

    def __rmul__(self , other):
        return self * other

    def timetuple(self):
        # this function is a requirement for comparing against datetime objects where the CTime is on the right side
        pass

    @property
    def stripped(self):
        return time.strptime(self, "%Y-%m-%d %H:%M:S%")


cwrapper = CWrapper(None)
cwrapper.registerType("time_t", CTime)

