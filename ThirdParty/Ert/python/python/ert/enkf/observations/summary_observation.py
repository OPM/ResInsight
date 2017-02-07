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

from cwrap import BaseCClass
from ert.enkf import EnkfPrototype


class SummaryObservation(BaseCClass):
    TYPE_NAME = "summary_obs"

    _alloc            = EnkfPrototype("void*  summary_obs_alloc(char*, char*, double, double, char*, double)", bind = False)
    _free             = EnkfPrototype("void   summary_obs_free(summary_obs)")
    _get_value        = EnkfPrototype("double summary_obs_get_value(summary_obs)")
    _get_std          = EnkfPrototype("double summary_obs_get_std(summary_obs)")
    _get_std_scaling  = EnkfPrototype("double summary_obs_get_std_scaling(summary_obs)")
    _get_summary_key  = EnkfPrototype("char*  summary_obs_get_summary_key(summary_obs)")
    _update_std_scale = EnkfPrototype("void   summary_obs_update_std_scale(summary_obs , double , active_list)")


    def __init__(self, summary_key, observation_key, value, std, auto_corrf_name=None, auto_corrf_param=0.0):
        assert isinstance(summary_key, str)
        assert isinstance(observation_key, str)
        assert isinstance(value, float)
        assert isinstance(std, float)

        if auto_corrf_name is not None:
            assert isinstance(auto_corrf_name, str)

        assert isinstance(auto_corrf_param, float)
        c_ptr = self._alloc(summary_key, observation_key, value, std, auto_corrf_name, auto_corrf_param)
        if c_ptr:
            super(SummaryObservation, self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct SummaryObservation with given configuration!')

    def getValue(self):
        """ @rtype: float """
        return self._get_value()

    def getStandardDeviation(self):
        """ @rtype: float """
        return self._get_std()

    def getStdScaling(self , index = 0):
        """ @rtype: float """
        return self._get_std_scaling()

    def __len__(self):
        return 1
    
    
    def getSummaryKey(self):
        """ @rtype: str """
        return self._get_summary_key()


    def updateStdScaling(self , factor , active_list):
        self._update_std_scale(factor , active_list)
    

    def free(self):
        self._free()

    def __repr__(self):
        sk = self.getSummaryKey()
        va = self.getValue()
        sd = self.getStandardDeviation()
        sc = self.getStdScaling()
        ad = self._address()
        fmt = 'SummaryObservation(key = %s, value = %f, std = %f, std_scaling = %f) at 0x%x'
        return fmt % (sk, va, sd, sc, ad)
