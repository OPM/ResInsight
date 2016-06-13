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

from ert.cwrap import BaseCClass, CWrapper
from ert.util import StringList, IntVector
from ert.sched import History
from ert.ecl import EclSum , EclGrid
from ert.enkf import ENKF_LIB, EnkfFs, LocalObsdataNode , LocalObsdata, MeasData, ObsData
from ert.enkf.enums import EnkfObservationImplementationType

from ert.enkf.observations import ObsVector


class EnkfObs(BaseCClass):
    def __init__(self , ensemble_config , history = None , external_time_map = None , grid = None , refcase = None ):
        c_ptr = EnkfObs.cNamespace().alloc( history , external_time_map , grid , refcase , ensemble_config )
        super(EnkfObs, self).__init__(c_ptr)

        
    def __len__(self):
        return EnkfObs.cNamespace().get_size(self)

    def __contains__(self , key):
        return EnkfObs.cNamespace().has_key(self, key)

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


    def createLocalObsdata(self , key , add_active_steps = True):
        # Use getAllActiveLocalObsdata()
        raise NotImplementedError("Hmmm C function: enkf_obs_alloc_all_active_local_obs() removed")
    


    def getAllActiveLocalObsdata(self , key = "ALL-OBS"):
        return EnkfObs.cNamespace().create_all_active_obs( self , key )
        


    def getTypedKeylist(self, observation_implementation_type):
        """
         @type observation_implementation_type: EnkfObservationImplementationType
         @rtype: StringList
        """
        return EnkfObs.cNamespace().alloc_typed_keylist(self, observation_implementation_type)

    def obsType(self , key):
        if key in self:
            return EnkfObs.cNamespace().obs_type( self , key)
        else:
            raise KeyError("Unknown observation key:%s" % key)
            

    def getMatchingKeys(self , pattern , obs_type = None):
        """
        Will return a list of all the observation keys matching the input
        pattern. The matching is based on fnmatch().
        """
        key_list = EnkfObs.cNamespace().alloc_matching_keylist(self, pattern)
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
        return EnkfObs.cNamespace().iget_obs_time(self, index)


    def addObservationVector(self, observation_vector):
        assert isinstance(observation_vector, ObsVector)

        observation_vector.convertToCReference(self)

        EnkfObs.cNamespace().add_obs_vector(self, observation_vector)

    def getObservationAndMeasureData(self, fs, local_obsdata, active_list, meas_data, obs_data):
        assert isinstance(fs, EnkfFs)
        assert isinstance(local_obsdata, LocalObsdata)
        assert isinstance(active_list, IntVector)
        assert isinstance(meas_data, MeasData)
        assert isinstance(obs_data, ObsData)

        EnkfObs.cNamespace().get_obs_and_measure_data(self, fs, local_obsdata, active_list, meas_data, obs_data)


    def scaleCorrelatedStd( self , fs , local_obsdata , active_list):
        return EnkfObs.cNamespace().scale_correlated_std( self , fs , active_list , local_obsdata )
    
    def localScaleStd( self , local_obsdata , scale_factor):
        return EnkfObs.cNamespace().local_scale_std( self , local_obsdata, scale_factor)

    def load(self , config_file):
        if not os.path.isfile( config_file ):
            raise IOError("The observation config file:%s does not exist" % config_file)
        return EnkfObs.cNamespace().load( self , config_file )


    def clear(self):
        EnkfObs.cNamespace().clear( self )


    def free(self):
        EnkfObs.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("enkf_obs", EnkfObs)

EnkfObs.cNamespace().alloc = cwrapper.prototype("c_void_p enkf_obs_alloc( history , time_map , ecl_grid , ecl_sum , ens_config )")
EnkfObs.cNamespace().free = cwrapper.prototype("void enkf_obs_free( enkf_obs )")
EnkfObs.cNamespace().get_size = cwrapper.prototype("int enkf_obs_get_size( enkf_obs )")
EnkfObs.cNamespace().load = cwrapper.prototype("bool enkf_obs_load( enkf_obs , char*)")
EnkfObs.cNamespace().clear = cwrapper.prototype("void enkf_obs_clear( enkf_obs )")
EnkfObs.cNamespace().alloc_typed_keylist = cwrapper.prototype("stringlist_obj enkf_obs_alloc_typed_keylist(enkf_obs, enkf_obs_impl_type)")
EnkfObs.cNamespace().alloc_matching_keylist = cwrapper.prototype("stringlist_obj enkf_obs_alloc_matching_keylist(enkf_obs, char*)")
EnkfObs.cNamespace().has_key = cwrapper.prototype("bool enkf_obs_has_key(enkf_obs, char*)")
EnkfObs.cNamespace().obs_type = cwrapper.prototype("enkf_obs_impl_type enkf_obs_get_type(enkf_obs, char*)")
EnkfObs.cNamespace().get_vector = cwrapper.prototype("obs_vector_ref enkf_obs_get_vector(enkf_obs, char*)")
EnkfObs.cNamespace().iget_vector = cwrapper.prototype("obs_vector_ref enkf_obs_iget_vector(enkf_obs, int)")
EnkfObs.cNamespace().iget_obs_time = cwrapper.prototype("time_t enkf_obs_iget_obs_time(enkf_obs, int)")
EnkfObs.cNamespace().add_obs_vector = cwrapper.prototype("void enkf_obs_add_obs_vector(enkf_obs, obs_vector)")

EnkfObs.cNamespace().get_obs_and_measure_data = cwrapper.prototype("void enkf_obs_get_obs_and_measure_data(enkf_obs, enkf_fs, local_obsdata, int_vector, meas_data, obs_data)")
EnkfObs.cNamespace().create_all_active_obs       = cwrapper.prototype("local_obsdata_obj enkf_obs_alloc_all_active_local_obs( enkf_obs , char*)");
EnkfObs.cNamespace().scale_correlated_std        = cwrapper.prototype("double  enkf_obs_scale_correlated_std( enkf_obs , enkf_fs , int_vector , local_obsdata)");
EnkfObs.cNamespace().local_scale_std             = cwrapper.prototype("void  enkf_obs_local_scale_std( enkf_obs , local_obsdata , double)");


