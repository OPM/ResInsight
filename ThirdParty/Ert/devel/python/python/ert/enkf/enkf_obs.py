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

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
import  libenkf
from    ert.util.stringlist   import StringList
from ert.enkf.obs_vector import ObsVector

class EnkfObs(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_config_file(self):
        return cfunc.get_config_file(self)

    def alloc_typed_keylist(self, type):
        return StringList(c_ptr = cfunc.alloc_typed_keylist(self, type), parent = self)

    @property
    def has_key(self, key):
        return cfunc.has_key(self, key)

    @property
    def get_vector(self, key):
        return ObsVector(cfunc.get_vector(self,key), parent = self)

##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "enkf_obs" , EnkfObs )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("enkf_obs")


cfunc.free                = cwrapper.prototype("void enkf_obs_free( enkf_obs )")
cfunc.get_config_file     = cwrapper.prototype("char* enkf_obs_get_config_file( enkf_obs )")
cfunc.alloc_typed_keylist = cwrapper.prototype("c_void_p enkf_obs_alloc_typed_keylist(enkf_obs, int)")
cfunc.has_key             = cwrapper.prototype("bool enkf_obs_has_key(enkf_obs, char*)")
cfunc.get_vector          = cwrapper.prototype("c_void_p enkf_obs_get_vector(enkf_obs, char*)")
