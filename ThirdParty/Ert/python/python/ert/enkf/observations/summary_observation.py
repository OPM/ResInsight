#  Copyright (C) 2012 Statoil ASA, Norway.
#
#  The file 'summary_observation.py' is part of ERT - Ensemble based Reservoir Tool.
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


class SummaryObservation(BaseCClass):
    def __init__(self, summary_key, observation_key, value, std, auto_corrf_name=None, auto_corrf_param=0.0):
        assert isinstance(summary_key, str)
        assert isinstance(observation_key, str)
        assert isinstance(value, float)
        assert isinstance(std, float)

        if auto_corrf_name is not None:
            assert isinstance(auto_corrf_name, str)

        assert isinstance(auto_corrf_param, float)
        pointer = SummaryObservation.cNamespace().alloc(summary_key, observation_key, value, std, auto_corrf_name, auto_corrf_param)
        super(SummaryObservation, self).__init__(pointer)

    def getValue(self):
        """ @rtype: float """
        return SummaryObservation.cNamespace().get_value(self)

    def getStandardDeviation(self):
        """ @rtype: float """
        return SummaryObservation.cNamespace().get_std(self)

    def getStdScaling(self , index = 0):
        """ @rtype: float """
        return SummaryObservation.cNamespace().get_std_scaling(self)

    def __len__(self):
        return 1
    
    
    def getSummaryKey(self):
        """ @rtype: str """
        return SummaryObservation.cNamespace().get_summary_key(self)


    def updateStdScaling(self , factor , active_list):
        SummaryObservation.cNamespace().update_std_scale(self , factor , active_list)
    

    def free(self):
        SummaryObservation.cNamespace().free(self)


    



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("summary_obs", SummaryObservation)

SummaryObservation.cNamespace().alloc = cwrapper.prototype("c_void_p summary_obs_alloc(char*, char*, double, double, char*, double)")
SummaryObservation.cNamespace().free = cwrapper.prototype("void summary_obs_free(summary_obs)")
SummaryObservation.cNamespace().get_value = cwrapper.prototype("double summary_obs_get_value(summary_obs)")
SummaryObservation.cNamespace().get_std = cwrapper.prototype("double summary_obs_get_std(summary_obs)")
SummaryObservation.cNamespace().get_std_scaling = cwrapper.prototype("double summary_obs_get_std_scaling(summary_obs)")
SummaryObservation.cNamespace().get_summary_key = cwrapper.prototype("char* summary_obs_get_summary_key(summary_obs)")
SummaryObservation.cNamespace().update_std_scale = cwrapper.prototype("void summary_obs_update_std_scale(summary_obs , double , active_list)")
