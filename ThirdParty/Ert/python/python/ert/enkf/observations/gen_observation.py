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

from cwrap import BaseCClass
from ert.util import IntVector
from ert.enkf import EnkfPrototype
from ert.enkf import GenDataConfig


class GenObservation(BaseCClass):
    TYPE_NAME = "gen_obs"

    _alloc              = EnkfPrototype("void*  gen_obs_alloc__(gen_data_config , char*)", bind = False)
    _free               = EnkfPrototype("void   gen_obs_free(gen_obs)")
    _load               = EnkfPrototype("void   gen_obs_load_observation(gen_obs , char*)")
    _scalar_set         = EnkfPrototype("void   gen_obs_set_scalar(gen_obs , double , double)")
    _get_std            = EnkfPrototype("double gen_obs_iget_std(gen_obs, int)")
    _get_value          = EnkfPrototype("double gen_obs_iget_value(gen_obs, int)")
    _get_std_scaling    = EnkfPrototype("double gen_obs_iget_std_scaling(gen_obs, int)")
    _get_size           = EnkfPrototype("int    gen_obs_get_size(gen_obs)")
    _get_data_index     = EnkfPrototype("int    gen_obs_get_obs_index(gen_obs, int)")
    _load_data_index    = EnkfPrototype("void   gen_obs_load_data_index(gen_obs , char*)")
    _add_data_index     = EnkfPrototype("void   gen_obs_attach_data_index(gen_obs , int_vector)")
    _update_std_scaling = EnkfPrototype("void   gen_obs_update_std_scale(gen_obs , double , active_list)")

    def __init__(self , obs_key , data_config , scalar_value = None , obs_file = None , data_index = None):
        c_ptr = self._alloc( data_config , obs_key )
        if c_ptr:
            super(GenObservation, self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct GenObservation with given obs_key and data_config!')

        if scalar_value is None and obs_file is None:
            raise ValueError("Exactly one the scalar_value and obs_file arguments must be present")

        if scalar_value is not None and obs_file is not None:
            raise ValueError("Exactly one the scalar_value and obs_file arguments must be present")

        if obs_file is not None:
            if not os.path.isfile( obs_file ):
                raise IOError("The file with observation data:%s does not exist" % obs_file )
            else:
                self._load( obs_file )
        else:
            obs_value , obs_std = scalar_value
            self._scalar_set( obs_value , obs_std )

        if not data_index is None:
            if os.path.isfile( data_index ):
                self._load_data_index( data_index )
            else:
                index_list = IntVector.active_list( data_index )
                self._add_data_index( index_list )
    
    
    def __len__(self):
        return self._get_size()

    def __getitem__(self , obs_index):
        if obs_index < 0:
            obs_index += len(self)
        
        if 0 <= obs_index < len(self):
            return (self.getValue(obs_index) , self.getStandardDeviation(obs_index))
        else:
            raise IndexError("Invalid index.  Valid range: [0,%d)" % len(self))


    def getValue(self, obs_index):
        """ @rtype: float """
        return self._get_value(obs_index)

    def getStandardDeviation(self, obs_index):
        """ @rtype: float """
        return self._get_std(obs_index)

    def getStdScaling(self, obs_index):
        """ @rtype: float """
        return self._get_std_scaling(obs_index)

    def updateStdScaling(self , factor , active_list):
        self._update_std_scaling(factor , active_list)


    def getSize(self):
        """ @rtype: float """
        return len(self)

    def getIndex(self, obs_index):
        """ @rtype: int """
        return self.getDataIndex( obs_index )
        
    def getDataIndex(self, obs_index):
        return self._get_data_index(obs_index)

    def free(self):
        self._free()

    def __repr__(self):
        si = len(self)
        ad = self._ad_str()
        return 'GenObservation(size = %d) %s' % (si, ad)
