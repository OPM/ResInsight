# Copyright (C) 2012  Statoil ASA, Norway.
#   
#  The file 'enkf_obs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB, EnkfFs, LocalObsdata, MeasData, ObsData
from ert.enkf.enums import EnkfStateType

from ert.enkf.observations import ObsVector
from ert.util import StringList, IntVector


class EnkfObs(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")


    def __len__(self):
        return EnkfObs.cNamespace().get_size(self)


    def __iter__(self):
        iobs = 0
        while iobs < len(self):
            vector = self[iobs]
            yield vector
            iobs += 1


    def __getitem__(self, key_or_index):
        """ @rtype: ObsVector """
        if isinstance(key_or_index, str):
            if self.hasKey(key_or_index):
                return EnkfObs.cNamespace().get_vector(self, key_or_index).setParent(self)
            else:
                raise KeyError("Unknown key: %s" % key_or_index)
        elif isinstance(key_or_index, int):
            if 0 <= key_or_index < len(self):
                return EnkfObs.cNamespace().iget_vector(self, key_or_index).setParent(self)
            else:
                raise IndexError("Index must be in range: 0 <= %d < %d" % (key_or_index, len(self)))
        else:
            raise TypeError("Key or index must be of type str or int")

    def get_config_file(self):
        """ @rtype: Str """
        return EnkfObs.cNamespace().get_config_file(self)

    def getTypedKeylist(self, observation_implementation_type):
        """
         @type observation_implementation_type: EnkfObservationImplementationType
         @rtype: StringList
        """
        return EnkfObs.cNamespace().alloc_typed_keylist(self, observation_implementation_type)

    def hasKey(self, key):
        """ @rtype: bool """
        return EnkfObs.cNamespace().has_key(self, key)


    def getObservationTime(self, index):
        """ @rtype: CTime """
        return EnkfObs.cNamespace().iget_obs_time(self, index)


    def addObservationVector(self, observation_key, observation_vector):
        assert isinstance(observation_key, str)
        assert isinstance(observation_vector, ObsVector)

        observation_vector.convertToCReference(self)

        EnkfObs.cNamespace().add_obs_vector(self, observation_key, observation_vector)

    def getObservationAndMeasureData(self, fs, local_obsdata, state, active_list, meas_data, obs_data):
        assert isinstance(fs, EnkfFs)
        assert isinstance(local_obsdata, LocalObsdata)
        assert isinstance(state, EnkfStateType)
        assert isinstance(active_list, IntVector)
        assert isinstance(meas_data, MeasData)
        assert isinstance(obs_data, ObsData)

        EnkfObs.cNamespace().get_obs_and_measure_data(self, fs, local_obsdata, state, active_list, meas_data, obs_data)


    def free(self):
        EnkfObs.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_obs", EnkfObs)
cwrapper.registerType("enkf_obs_obj", EnkfObs.createPythonObject)
cwrapper.registerType("enkf_obs_ref", EnkfObs.createCReference)

EnkfObs.cNamespace().free = cwrapper.prototype("void enkf_obs_free( enkf_obs )")
EnkfObs.cNamespace().get_size = cwrapper.prototype("int enkf_obs_get_size( enkf_obs )")
EnkfObs.cNamespace().get_config_file = cwrapper.prototype("char* enkf_obs_get_config_file( enkf_obs )")
EnkfObs.cNamespace().alloc_typed_keylist = cwrapper.prototype("stringlist_obj enkf_obs_alloc_typed_keylist(enkf_obs, enkf_obs_impl_type)")
EnkfObs.cNamespace().has_key = cwrapper.prototype("bool enkf_obs_has_key(enkf_obs, char*)")
EnkfObs.cNamespace().get_vector = cwrapper.prototype("obs_vector_ref enkf_obs_get_vector(enkf_obs, char*)")
EnkfObs.cNamespace().iget_vector = cwrapper.prototype("obs_vector_ref enkf_obs_iget_vector(enkf_obs, int)")
EnkfObs.cNamespace().iget_obs_time = cwrapper.prototype("time_t enkf_obs_iget_obs_time(enkf_obs, int)")
EnkfObs.cNamespace().add_obs_vector = cwrapper.prototype("void enkf_obs_add_obs_vector(enkf_obs, char*, obs_vector)")

EnkfObs.cNamespace().get_obs_and_measure_data = cwrapper.prototype("void enkf_obs_get_obs_and_measure_data(enkf_obs, enkf_fs, local_obsdata, enkf_state_type_enum, int_vector, meas_data, obs_data)")
