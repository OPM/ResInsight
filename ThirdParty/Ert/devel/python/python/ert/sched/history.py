#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'history.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import CClass, CWrapper, CWrapperNameSpace
from ert.sched import SCHED_LIB


class HistoryType(CClass):
    def __init__(self, c_ptr, parent=None):
        if parent:
            self.init_cref(c_ptr, parent)
        else:
            self.init_cobj(c_ptr, cfunc.free)

    def get_source_string(self):
        return cfunc.get_source_string(self)

    @staticmethod
    def alloc_from_refcase(refcase, use_history):
        return HistoryType(cfunc.alloc_from_refcase(refcase, use_history))

    @staticmethod
    def alloc_from_sched_file(sched_file):
        return HistoryType(cfunc.alloc_from_sched_file(":", sched_file))

    ##################################################################

cwrapper = CWrapper(SCHED_LIB)
cwrapper.registerType("history_type", HistoryType)

cfunc = CWrapperNameSpace("history_type")

##################################################################
##################################################################

cfunc.free = cwrapper.prototype("void history_free( history_type )")
cfunc.get_source_string = cwrapper.prototype("char* history_get_source_string(history_type)")
cfunc.alloc_from_refcase = cwrapper.prototype("c_void_p history_alloc_from_refcase(ecl_sum, bool)")
cfunc.alloc_from_sched_file = cwrapper.prototype("c_void_p history_alloc_from_sched_file(char*, c_void_p)")
