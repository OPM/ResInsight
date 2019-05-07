#  Copyright (C) 2011  Equinor ASA, Norway. 
#   
#  The file 'lookup_table.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from cwrap import BaseCClass
from ecl import EclPrototype


class LookupTable(BaseCClass):
    _alloc =          EclPrototype("void* lookup_table_alloc_empty()" , bind = False)
    _max =            EclPrototype("double lookup_table_get_max_value( lookup_table )")
    _min =            EclPrototype("double lookup_table_get_min_value( lookup_table )")
    _arg_max =        EclPrototype("double lookup_table_get_max_arg( lookup_table )")
    _arg_min =        EclPrototype("double lookup_table_get_min_arg( lookup_table )")
    _append =         EclPrototype("void lookup_table_append( lookup_table , double , double )")
    _size =           EclPrototype("int lookup_table_get_size( lookup_table )")
    _interp =         EclPrototype("double lookup_table_interp( lookup_table , double)")
    _free =           EclPrototype("void lookup_table_free( lookup_table )")
    _set_low_limit =  EclPrototype("void lookup_table_set_low_limit( lookup_table , double)")
    _set_high_limit = EclPrototype("void lookup_table_set_high_limit( lookup_table , double)")
    _has_low_limit =  EclPrototype("bool lookup_table_has_low_limit( lookup_table)")
    _has_high_limit = EclPrototype("bool lookup_table_has_high_limit( lookup_table)")

    def __init__(self, lower_limit=None, upper_limit=None):
        super(LookupTable, self).__init__(self._alloc())

        if not lower_limit is None:
            self.setLowerLimit(lower_limit)

        if not upper_limit is None:
            self.setUpperLimit(upper_limit)

    def getMaxValue(self):
        self.assertSize(1)
        return self._max()

    def getMinValue(self):
        self.assertSize(1)
        return self._min()

    def getMinArg(self):
        self.assertSize(1)
        return self._arg_min()

    def getMaxArg(self):
        self.assertSize(1)
        return self._arg_max()

    def assertSize(self, N):
        if len(self) < N:
            raise ValueError("Lookup table is too small")

    def __len__(self):
        return self._size()

    @property
    def size(self):
        return len(self)

    # Deprecated properties
    @property
    def max(self):
        return self.getMaxValue()

    @property
    def min(self):
        return self.getMinValue()

    @property
    def arg_max(self):
        return self.getMaxArg()

    @property
    def arg_min(self):
        return self.getMinArg()

    def setLowerLimit(self, value):
        self._set_low_limit(value)

    def hasLowerLimit(self):
        return self._has_low_limit()

    def setUpperLimit(self, value):
        self._set_high_limit(value)

    def hasUpperLimit(self):
        return self._has_high_limit()

    def interp(self, x):
        self.assertSize(2)
        if x < self.getMinArg():
            if not self.hasLowerLimit():
                raise ValueError("Interpolate argument:%g is outside valid interval: [%g,%g]" % (x, self.getMinArg(), self.getMaxArg()))
        elif x > self.getMaxArg():
            if not self.hasUpperLimit():
                raise ValueError("Interpolate argument:%g is outside valid interval: [%g,%g]" % (x, self.getMinArg(), self.getMaxArg()))

        return self._interp(x)

    def append(self, x, y):
        self._append( x, y)

    #todo: necessary???
    def __del__(self):
        self._free()

    def free(self):
        self._free( )

