#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_state.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB


class EnKFState(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def kill_simulation(self):
        EnKFState.cNamespace().kill_simulation(self)

    def resubmit_simulation(self, sim_number):
        EnKFState.cNamespace().resubmit_simulation(self, sim_number)

    def get_run_status(self):
        return EnKFState.cNamespace().get_run_status(self)

    def get_start_time(self):
        return EnKFState.cNamespace().get_start_time(self)

    def get_submit_time(self):
        return EnKFState.cNamespace().get_submit_time(self)

    def free(self):
        EnKFState.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_state", EnKFState)
cwrapper.registerType("enkf_state_obj", EnKFState.createPythonObject)
cwrapper.registerType("enkf_state_ref", EnKFState.createCReference)

EnKFState.cNamespace().free = cwrapper.prototype("void enkf_state_free( enkf_state )")
EnKFState.cNamespace().kill_simulation = cwrapper.prototype("void enkf_state_kill_simulation(enkf_state)")
EnKFState.cNamespace().resubmit_simulation = cwrapper.prototype("void enkf_state_resubmit_simulation(enkf_state, int)")
EnKFState.cNamespace().get_run_status = cwrapper.prototype("int enkf_state_get_run_status(enkf_state)")
EnKFState.cNamespace().get_start_time = cwrapper.prototype("int enkf_state_get_start_time(enkf_state)")
EnKFState.cNamespace().get_submit_time = cwrapper.prototype("int enkf_state_get_submit_time(enkf_state)")
