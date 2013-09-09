#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'sched_file.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import BaseCClass, CWrapper
from ert.sched import SCHED_LIB
from ert.util import ctime


class SchedFile(BaseCClass):
    def __init__(self, filename, start_time):
        c_ptr = SchedFile.cNamespace().parse(filename, ctime(start_time))
        super(SchedFile, self).__init__(c_ptr)

    @property
    def length(self):
        """ @rtype: int """
        return SchedFile.cNamespace().length(self)

    def write(self, filename, num_dates, add_end=True):
        SchedFile.cNamespace().write(self, num_dates, filename, add_end)

    def free(self):
        SchedFile.cNamespace().free(self)


cwrapper = CWrapper(SCHED_LIB)
cwrapper.registerType("sched_file", SchedFile)
cwrapper.registerType("sched_file_obj", SchedFile.createPythonObject)
cwrapper.registerType("sched_file_ref", SchedFile.createCReference)

SchedFile.cNamespace().parse = cwrapper.prototype("c_void_p sched_file_parse_alloc( char*, time_t )")
SchedFile.cNamespace().write = cwrapper.prototype("void sched_file_fprintf_i( sched_file , int , char* , bool)")
SchedFile.cNamespace().length = cwrapper.prototype("int sched_file_get_num_restart_files( sched_file )")
SchedFile.cNamespace().free = cwrapper.prototype("void sched_file_free( sched_file )")
