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
import os.path 

from cwrap import BaseCClass, CWrapper
from ert.util import IntVector
from ert.enkf import ENKF_LIB


class GenObservation(BaseCClass):

    def __init__(self , obs_key , data_config , scalar_value = None , obs_file = None , data_index = None):
        c_pointer = GenObservation.cNamespace().alloc( data_config , obs_key )
        super(GenObservation, self).__init__(c_pointer)

        if scalar_value is None and obs_file is None:
            raise ValueError("Exactly one the scalar_value and obs_file arguments must be present")

        if scalar_value is not None and obs_file is not None:
            raise ValueError("Exactly one the scalar_value and obs_file arguments must be present")

        if obs_file is not None:
            if not os.path.isfile( obs_file ):
                raise IOError("The file with observation data:%s does not exist" % obs_file )
            else:
                GenObservation.cNamespace().load( self , obs_file )
        else:
            obs_value , obs_std = scalar_value
            GenObservation.cNamespace().scalar_set( self , obs_value , obs_std )

        if not data_index is None:
            if os.path.isfile( data_index ):
                GenObservation.cNamespace().load_data_index( self , data_index )
            else:
                index_list = IntVector.active_list( data_index )
                GenObservation.cNamespace().add_data_index( self , index_list )
    
    
    def __len__(self):
        return GenObservation.cNamespace().get_size(self)

    def __getitem__(self , obs_index):
        if obs_index < 0:
            obs_index += len(self)
        
        if 0 <= obs_index < len(self):
            return (self.getValue(obs_index) , self.getStandardDeviation(obs_index))
        else:
            raise IndexError("Valid range: [0,%d)" % len(self))


    def getValue(self, obs_index):
        """ @rtype: float """
        return GenObservation.cNamespace().get_value(self, obs_index)

    def getStandardDeviation(self, obs_index):
        """ @rtype: float """
        return GenObservation.cNamespace().get_std(self, obs_index)

    def getStdScaling(self, obs_index):
        """ @rtype: float """
        return GenObservation.cNamespace().get_std_scaling(self, obs_index)

    def updateStdScaling(self , factor , active_list):
        GenObservation.cNamespace().update_std_scaling(self, factor , active_list)


    def getSize(self):
        """ @rtype: float """
        return len(self)

    def getIndex(self, obs_index):
        """ @rtype: int """
        return self.getDataIndex( obs_index )
        
    def getDataIndex(self, obs_index):
        return GenObservation.cNamespace().get_data_index(self, obs_index)

    def free(self):
        GenObservation.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("gen_obs", GenObservation)
cwrapper.registerType("gen_obs_obj", GenObservation.createPythonObject)
cwrapper.registerType("gen_obs_ref", GenObservation.createCReference)

GenObservation.cNamespace().alloc = cwrapper.prototype("c_void_p gen_obs_alloc__(gen_data_config , char*)")
GenObservation.cNamespace().free = cwrapper.prototype("void gen_obs_free(gen_data_config)")
GenObservation.cNamespace().load = cwrapper.prototype("void gen_obs_load_observation(gen_obs , char*)")
GenObservation.cNamespace().scalar_set = cwrapper.prototype("void gen_obs_set_scalar(gen_obs , double , double)")
GenObservation.cNamespace().get_value = cwrapper.prototype("double gen_obs_iget_value(summary_obs)")
GenObservation.cNamespace().get_std_scaling = cwrapper.prototype("double gen_obs_iget_std_scaling(summary_obs)")
GenObservation.cNamespace().get_std = cwrapper.prototype("double gen_obs_iget_std(gen_obs, int)")
GenObservation.cNamespace().get_size = cwrapper.prototype("int gen_obs_get_size(gen_obs)")
GenObservation.cNamespace().get_data_index = cwrapper.prototype("int gen_obs_get_obs_index(gen_obs, int)")
GenObservation.cNamespace().load_data_index = cwrapper.prototype("void gen_obs_load_data_index(gen_obs , char*)")
GenObservation.cNamespace().add_data_index = cwrapper.prototype("void gen_obs_attach_data_index(gen_obs , int_vector)")
GenObservation.cNamespace().update_std_scaling = cwrapper.prototype("void gen_obs_update_std_scale(gen_obs , double , active_list)")
