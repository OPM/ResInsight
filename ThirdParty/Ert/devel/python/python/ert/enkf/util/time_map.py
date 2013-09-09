#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'time_map.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB


class TimeMap(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def iget_sim_days(self, step):
        return TimeMap.cNamespace().iget_sim_days(self, step)

    def iget(self, step):
        return TimeMap.cNamespace().iget(self, step)

    def free(self):
        TimeMap.cNamespace().free(self)

##################################################################
cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("time_map", TimeMap)
cwrapper.registerType("time_map_obj", TimeMap.createPythonObject)
cwrapper.registerType("time_map_ref", TimeMap.createCReference)


##################################################################
##################################################################

TimeMap.cNamespace().free = cwrapper.prototype("void time_map_free( time_map )")
TimeMap.cNamespace().iget_sim_days = cwrapper.prototype("double time_map_iget_sim_days(time_map, int)")
TimeMap.cNamespace().iget = cwrapper.prototype("int time_map_iget(time_map, int)")
