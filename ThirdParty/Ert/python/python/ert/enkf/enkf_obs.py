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
import os.path

from cwrap import BaseCClass
from ert.util import StringList, IntVector
from ert.sched import History
from ert.ecl import EclSum , EclGrid
from ert.enkf import EnkfPrototype
from ert.enkf import EnkfFs, LocalObsdataNode , LocalObsdata, MeasData, ObsData
from ert.enkf.enums import EnkfObservationImplementationType

from ert.enkf.observations import ObsVector


class EnkfObs(BaseCClass):
    TYPE_NAME = "enkf_obs"

    _alloc                    = EnkfPrototype("void* enkf_obs_alloc( history , time_map , ecl_grid , ecl_sum , ens_config )", bind = False)
    _free                     = EnkfPrototype("void enkf_obs_free( enkf_obs )")
    _get_size                 = EnkfPrototype("int enkf_obs_get_size( enkf_obs )")
    _load                     = EnkfPrototype("bool enkf_obs_load( enkf_obs , char*)")
    _clear                    = EnkfPrototype("void enkf_obs_clear( enkf_obs )")
    _alloc_typed_keylist      = EnkfPrototype("stringlist_obj enkf_obs_alloc_typed_keylist(enkf_obs, enkf_obs_impl_type)")
    _alloc_matching_keylist   = EnkfPrototype("stringlist_obj enkf_obs_alloc_matching_keylist(enkf_obs, char*)")
    _has_key                  = EnkfPrototype("bool enkf_obs_has_key(enkf_obs, char*)")
    _obs_type                 = EnkfPrototype("enkf_obs_impl_type enkf_obs_get_type(enkf_obs, char*)")
    _get_vector               = EnkfPrototype("obs_vector_ref enkf_obs_get_vector(enkf_obs, char*)")
    _iget_vector              = EnkfPrototype("obs_vector_ref enkf_obs_iget_vector(enkf_obs, int)")
    _iget_obs_time            = EnkfPrototype("time_t enkf_obs_iget_obs_time(enkf_obs, int)")
    _add_obs_vector           = EnkfPrototype("void enkf_obs_add_obs_vector(enkf_obs, obs_vector)")
    _get_obs_and_measure_data = EnkfPrototype("void enkf_obs_get_obs_and_measure_data(enkf_obs, enkf_fs, local_obsdata, int_vector, meas_data, obs_data)")
    _create_all_active_obs    = EnkfPrototype("local_obsdata_obj enkf_obs_alloc_all_active_local_obs( enkf_obs , char*)");
    _scale_correlated_std     = EnkfPrototype("double  enkf_obs_scale_correlated_std( enkf_obs , enkf_fs ,       int_vector , local_obsdata)");
    _local_scale_std          = EnkfPrototype("void  enkf_obs_local_scale_std( enkf_obs ,        local_obsdata , double)");

    def __init__(self , ensemble_config , history = None , external_time_map = None , grid = None , refcase = None ):
        c_ptr = self._alloc( history , external_time_map , grid , refcase , ensemble_config )
        super(EnkfObs, self).__init__(c_ptr)

    def __len__(self):
        return self._get_size()

    def __contains__(self , key):
        return self._has_key(key)

    def __iter__(self):
        """ @rtype: ObsVector """
        iobs = 0
        while iobs < len(self):
            vector = self[iobs]
            yield vector
            iobs += 1


    def __getitem__(self, key_or_index):
        """ @rtype: ObsVector """
        if isinstance(key_or_index, str):
            if self.hasKey(key_or_index):
                return self._get_vector(key_or_index).setParent(self)
            else:
                raise KeyError("Unknown key: %s" % key_or_index)
        elif isinstance(key_or_index, int):
            idx = key_or_index
            if idx < 0:
                idx += len(self)
            if 0 <= idx < len(self):
                return self._iget_vector(idx).setParent(self)
            else:
                raise IndexError("Invalid index: %d.  Valid range is [0, %d)." % (key_or_index, len(self)))
        else:
            raise TypeError("Key or index must be of type str or int, not %s." % str(type(key_or_index)))


    def createLocalObsdata(self , key , add_active_steps = True):
        # Use getAllActiveLocalObsdata()
        raise NotImplementedError("Hmmm C function: enkf_obs_alloc_all_active_local_obs() removed")



    def getAllActiveLocalObsdata(self , key = "ALL-OBS"):
        return self._create_all_active_obs( key )



    def getTypedKeylist(self, observation_implementation_type):
        """
         @type observation_implementation_type: EnkfObservationImplementationType
         @rtype: StringList
        """
        return self._alloc_typed_keylist(observation_implementation_type)

    def obsType(self , key):
        if key in self:
            return self._obs_type(key)
        else:
            raise KeyError("Unknown observation key:%s" % key)


    def getMatchingKeys(self , pattern , obs_type = None):
        """
        Will return a list of all the observation keys matching the input
        pattern. The matching is based on fnmatch().
        """
        key_list = self._alloc_matching_keylist(pattern)
        if obs_type:
            new_key_list = []
            for key in key_list:
                if self.obsType( key ) == obs_type:
                    new_key_list.append( key )
            return new_key_list
        else:
            return key_list


    def hasKey(self, key):
        """ @rtype: bool """
        return key in self


    def getObservationTime(self, index):
        """ @rtype: CTime """
        return self._iget_obs_time(index)


    def addObservationVector(self, observation_vector):
        assert isinstance(observation_vector, ObsVector)

        observation_vector.convertToCReference(self)

        self._add_obs_vector(observation_vector)

    def getObservationAndMeasureData(self, fs, local_obsdata, active_list, meas_data, obs_data):
        assert isinstance(fs, EnkfFs)
        assert isinstance(local_obsdata, LocalObsdata)
        assert isinstance(active_list, IntVector)
        assert isinstance(meas_data, MeasData)
        assert isinstance(obs_data, ObsData)

        self._get_obs_and_measure_data(fs, local_obsdata, active_list, meas_data, obs_data)


    def scaleCorrelatedStd( self , fs , local_obsdata , active_list):
        return self._scale_correlated_std( fs , active_list , local_obsdata )

    def localScaleStd( self , local_obsdata , scale_factor):
        return self._local_scale_std(local_obsdata, scale_factor)

    def load(self , config_file):
        if not os.path.isfile( config_file ):
            raise IOError('The observation config file "%s" does not exist.' % config_file)
        return self._load( config_file )

    def clear(self):
        self._clear()

    def free(self):
        self._free()

    def __repr__(self):
        return 'EnkfObs(len = %d) at 0x%x' % (len(self), self._address())
