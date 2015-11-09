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


import ctypes
import datetime
import time
from ert.cwrap import CWrapper, BaseCValue
from ert.util import UTIL_LIB


class CTime(BaseCValue):
    DATA_TYPE = ctypes.c_long

    def __init__(self, value):
        if isinstance(value, int):
            value = value
        elif isinstance(value, CTime):
            value = value.value()
        elif isinstance(value, datetime.datetime):
            value = CTime._mktime(value.second, value.minute, value.hour, value.day, value.month, value.year)
        elif isinstance(value, datetime.date):
            value = CTime._mktime(0, 0, 0, value.day, value.month, value.year)
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
        return self.datetime().strftime("%Y-%m-%d %H:%M:%S%z")

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

    def __repr__(self):
        return "time_t value: %d [%s]" % (self.value(), str(self))


    @property
    def stripped(self):
        return time.strptime(self, "%Y-%m-%d %H:%M:S%")

    @classmethod
    def timezone(cls):
        """
         Returns the current timezone "in" C
         @rtype: str
        """
        return CTime._timezone()


cwrapper = CWrapper(UTIL_LIB)
cwrapper.registerType("time_t", CTime)

CTime._timezone = cwrapper.prototype("char* util_get_timezone()")
CTime._mktime = cwrapper.prototype("long util_make_datetime(int, int, int, int, int, int)")

