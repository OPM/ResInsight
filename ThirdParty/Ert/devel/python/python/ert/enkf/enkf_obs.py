#  Copyright (C) 2012  Statoil ASA, Norway. 
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
from ert.enkf import ENKF_LIB

from ert.util import StringList
from ert.enkf.util import ObsVector


class EnkfObs(BaseCClass):
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_config_file(self):
        """ @rtype: Str """
        return EnkfObs.cNamespace().get_config_file(self)

    def alloc_typed_keylist(self, type):
        """ @rtype: StringList """
        return EnkfObs.cNamespace().alloc_typed_keylist(self, type).setParent(self)

    def has_key(self, key):
        """ @rtype: bool """
        return EnkfObs.cNamespace().has_key(self, key)

    def get_vector(self, key):
        """ @rtype: ObsVector """
        return EnkfObs.cNamespace().get_vector(self,key).setParent(self)

    def free(self):
        EnkfObs.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_obs", EnkfObs)
cwrapper.registerType("enkf_obs_obj", EnkfObs.createPythonObject)
cwrapper.registerType("enkf_obs_ref", EnkfObs.createCReference)

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.

EnkfObs.cNamespace().free                = cwrapper.prototype("void enkf_obs_free( enkf_obs )")
EnkfObs.cNamespace().get_config_file     = cwrapper.prototype("char* enkf_obs_get_config_file( enkf_obs )")
EnkfObs.cNamespace().alloc_typed_keylist = cwrapper.prototype("stringlist_ref enkf_obs_alloc_typed_keylist(enkf_obs, int)")
EnkfObs.cNamespace().has_key             = cwrapper.prototype("bool enkf_obs_has_key(enkf_obs, char*)")
EnkfObs.cNamespace().get_vector          = cwrapper.prototype("obs_vector_ref enkf_obs_get_vector(enkf_obs, char*)")
