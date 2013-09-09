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
    def __init__(self):
        super(LookupTable, self).__init__(LookupTable.cNamespace().alloc())


    @property
    def max(self):
        return LookupTable.cNamespace().max(self)

    @property
    def min(self):
        return LookupTable.cNamespace().min(self)

    @property
    def arg_max(self):
        return LookupTable.cNamespace().arg_max(self)

    @property
    def arg_min(self):
        return LookupTable.cNamespace().arg_min(self)

    def interp(self, x):
        return LookupTable.cNamespace().interp(self, x)

    def append(self, x, y):
        LookupTable.cNamespace().append(self, x, y)

    @property
    def size(self):
        return LookupTable.cNamespace().size(self)


    def __del__(self):
        LookupTable.cNamespace().free(self)

    def __len__(self):
        return self.size

    def free(self):
        LookupTable.cNamespace().free(self)


cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("lookup_table", LookupTable)
CWrapper.registerType("lookup_table_obj", LookupTable.createPythonObject)
CWrapper.registerType("lookup_table_ref", LookupTable.createCReference)

LookupTable.cNamespace().alloc = cwrapper.prototype("c_void_p lookup_table_alloc_empty()")
LookupTable.cNamespace().max = cwrapper.prototype("double lookup_table_get_max_value( lookup_table )")
LookupTable.cNamespace().min = cwrapper.prototype("double lookup_table_get_min_value( lookup_table )")
LookupTable.cNamespace().arg_max = cwrapper.prototype("double lookup_table_get_max_arg( lookup_table )")
LookupTable.cNamespace().arg_min = cwrapper.prototype("double lookup_table_get_min_arg( lookup_table )")
LookupTable.cNamespace().append = cwrapper.prototype("void lookup_table_append( lookup_table , double , double )")
LookupTable.cNamespace().size = cwrapper.prototype("int lookup_table_get_size( lookup_table )")
LookupTable.cNamespace().interp = cwrapper.prototype("double lookup_table_interp( lookup_table , double)")
LookupTable.cNamespace().free = cwrapper.prototype("void lookup_table_free( lookup_table )")
