#  Copyright (C) 2012 Statoil ASA, Norway.
#
#  The file 'gen_observation.py' is part of ERT - Ensemble based Reservoir Tool.
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


class GenObservation(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    
    def __len__(self):
        return GenObservation.cNamespace().get_size(self)

    def __getitem__(self , obs_index):
        if obs_index < 0:
            obs_index += len(self)
        
        if 0 <= obs_index < len(self):
            return (self.getValue(obs_index) , self.getStandardDeviation(obs_index) , self.getDataIndex( obs_index ))
        else:
            raise IndexError("Valid range: [0,%d)" % len(self))


    def getValue(self, obs_index):
        """ @rtype: float """
        return GenObservation.cNamespace().get_data(self, obs_index)

    def getStandardDeviation(self, obs_index):
        """ @rtype: float """
        return GenObservation.cNamespace().get_std(self, obs_index)

    def getSize(self):
        """ @rtype: float """
        return len(self)

    def getIndex(self, obs_index):
        """ @rtype: int """
        return self.getDataIndex( obs_index )
        
    def getDataIndex(self, obs_index):
        return GenObservation.cNamespace().get_data_index(self, obs_index)




cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("gen_obs", GenObservation)
cwrapper.registerType("gen_obs_obj", GenObservation.createPythonObject)
cwrapper.registerType("gen_obs_ref", GenObservation.createCReference)

GenObservation.cNamespace().get_value = cwrapper.prototype("double summary_obs_get_value(summary_obs)")
GenObservation.cNamespace().get_std = cwrapper.prototype("double gen_obs_iget_std(gen_obs, int)")
GenObservation.cNamespace().get_data = cwrapper.prototype("double gen_obs_iget_data(gen_obs, int)")
GenObservation.cNamespace().get_size = cwrapper.prototype("int gen_obs_get_size(gen_obs)")
GenObservation.cNamespace().get_data_index = cwrapper.prototype("int gen_obs_get_obs_index(gen_obs, int)")
