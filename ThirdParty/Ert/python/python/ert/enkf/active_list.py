#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'active_list.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB

class ActiveList(BaseCClass):
    def __init__(self):
        c_ptr = ActiveList.cNamespace().alloc()
        super(ActiveList, self).__init__(c_ptr)


    def getMode(self):
        return ActiveList.cNamespace().get_mode(self)

    def addActiveIndex(self, index):
        ActiveList.cNamespace().add_index(self , index)

        
    def free(self):
        ActiveList.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("active_list", ActiveList)


ActiveList.cNamespace().alloc     = cwrapper.prototype("c_void_p active_list_alloc()")
ActiveList.cNamespace().free      = cwrapper.prototype("void     active_list_free(active_list)")
ActiveList.cNamespace().get_mode  = cwrapper.prototype("active_mode_enum active_list_get_mode(active_list)")
ActiveList.cNamespace().add_index = cwrapper.prototype("void active_list_add_index(active_list , int)")
