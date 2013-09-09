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

from ert.cwrap import CWrapper, BaseCClass
from ert.sched import SCHED_LIB, SchedFile, HistorySourceEnum
from ert.ecl import EclSum



class History(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    @staticmethod
    def get_source_string(history_source_type):
        """
        @type history_source_type: HistorySourceEnum
        @rtype: str
        """
        return History.cNamespace().get_source_string(history_source_type)

    #todo: change this to __init__?
    @staticmethod
    def alloc_from_refcase(refcase, use_history):
        """
        @type refcase: EclSum
        @type use_history: bool
        @rtype: HistoryType
        """
        return History.cNamespace().alloc_from_refcase(refcase, use_history)

    @staticmethod
    def alloc_from_sched_file(sched_file):
        """ @rtype: HistoryType """
        assert isinstance(sched_file, SchedFile)
        return History.cNamespace().alloc_from_sched_file(":", sched_file)

    def free(self):
        History.cNamespace().free(self)


cwrapper = CWrapper(SCHED_LIB)
cwrapper.registerType("history", History)
cwrapper.registerType("history_obj", History.createPythonObject)
cwrapper.registerType("history_ref", History.createCReference)



History.cNamespace().free = cwrapper.prototype("void history_free( history )")
History.cNamespace().get_source_string = cwrapper.prototype("char* history_get_source_string(history_source_enum)")
History.cNamespace().alloc_from_refcase = cwrapper.prototype("history_obj history_alloc_from_refcase(ecl_sum, bool)")
History.cNamespace().alloc_from_sched_file = cwrapper.prototype("history_obj history_alloc_from_sched_file(char*, sched_file)")

# History.cNamespace().history_get_source_type = cwrapper.prototype("history_source_type_enum history_get_source_type(char*)")
