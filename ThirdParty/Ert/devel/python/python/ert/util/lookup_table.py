#  Copyright (C) 2011  Statoil ASA, Norway. 
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


from ert.util import UTIL_LIB
from ert.cwrap import BaseCClass, CWrapper, CWrapperNameSpace


class LookupTable(BaseCClass):

    def __init__(self , lower_limit = None , upper_limit = None):
        super(LookupTable, self).__init__(LookupTable.cNamespace().alloc())

        if not lower_limit is None:
            self.setLowerLimit( lower_limit )

        if not upper_limit is None:
            self.setUpperLimit( upper_limit )



    def getMaxValue(self):
        self.assertSize( 1 )
        return LookupTable.cNamespace().max(self)

    def getMinValue(self):
        self.assertSize( 1 )
        return LookupTable.cNamespace().min(self)

    def getMinArg(self):
        self.assertSize( 1 )
        return LookupTable.cNamespace().arg_min(self)

    def getMaxArg(self):
        self.assertSize( 1 )
        return LookupTable.cNamespace().arg_max(self)

    def assertSize(self , N):
        if len(self) < N:
            raise ValueError("Lookup table is too small")

    def __len__(self):
        return LookupTable.cNamespace().size(self)

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

    def setLowerLimit(self , value):
        LookupTable.cNamespace().set_low_limit(self, value)

    def hasLowerLimit(self):
        return LookupTable.cNamespace().has_low_limit(self)

    def setUpperLimit(self , value):
        LookupTable.cNamespace().set_high_limit(self, value)

    def hasUpperLimit(self):
        return LookupTable.cNamespace().has_high_limit(self)

    def interp(self, x):
        self.assertSize( 2 )
        if x < self.getMinArg():
            if not self.hasLowerLimit():
                raise ValueError("Interpolate argument:%g is outside valid interval: [%g,%g]" % (x,self.getMinArg() , self.getMaxArg()))
        elif x > self.getMaxArg():
            if not self.hasUpperLimit():
                raise ValueError("Interpolate argument:%g is outside valid interval: [%g,%g]" % (x,self.getMinArg() , self.getMaxArg()))
                
        return LookupTable.cNamespace().interp(self, x)

            
            
    def append(self, x, y):
        LookupTable.cNamespace().append(self, x, y)


    def __del__(self):
        LookupTable.cNamespace().free(self)

    def free(self):
        LookupTable.cNamespace().free(self)


cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerObjectType("lookup_table", LookupTable)

LookupTable.cNamespace().alloc = cwrapper.prototype("c_void_p lookup_table_alloc_empty()")
LookupTable.cNamespace().max = cwrapper.prototype("double lookup_table_get_max_value( lookup_table )")
LookupTable.cNamespace().min = cwrapper.prototype("double lookup_table_get_min_value( lookup_table )")
LookupTable.cNamespace().arg_max = cwrapper.prototype("double lookup_table_get_max_arg( lookup_table )")
LookupTable.cNamespace().arg_min = cwrapper.prototype("double lookup_table_get_min_arg( lookup_table )")
LookupTable.cNamespace().append = cwrapper.prototype("void lookup_table_append( lookup_table , double , double )")
LookupTable.cNamespace().size = cwrapper.prototype("int lookup_table_get_size( lookup_table )")
LookupTable.cNamespace().interp = cwrapper.prototype("double lookup_table_interp( lookup_table , double)")
LookupTable.cNamespace().free = cwrapper.prototype("void lookup_table_free( lookup_table )")
LookupTable.cNamespace().set_low_limit = cwrapper.prototype("void lookup_table_set_low_limit( lookup_table , double)")
LookupTable.cNamespace().set_high_limit = cwrapper.prototype("void lookup_table_set_high_limit( lookup_table , double)")
LookupTable.cNamespace().has_low_limit = cwrapper.prototype("bool lookup_table_has_low_limit( lookup_table)")
LookupTable.cNamespace().has_high_limit = cwrapper.prototype("bool lookup_table_has_high_limit( lookup_table)")
