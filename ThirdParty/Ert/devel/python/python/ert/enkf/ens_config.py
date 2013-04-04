#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ens_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class EnsConfig(CClass):
    
    def __init__(self , c_ptr = None):
        self.owner = False
        self.c_ptr = c_ptr
        
        
    def __del__(self):
        if self.owner:
            cfunc.free( self )


    def has_key(self , key):
        return cfunc.has_key( self ,key )



##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "ens_config" , EnsConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ens_config")


cfunc.free          = cwrapper.prototype("void ensemble_config_free( ens_config )")
cfunc.has_key       = cwrapper.prototype("bool ensemble_config_has_key( ens_config , char* )")
cfunc.get_node      = cwrapper.prototype("c_void_p ensemble_config_get_node( ens_config , char*)")
cfunc.alloc_keylist = cwrapper.prototype("c_void_p ensemble_config_alloc_keylist( ens_config )")
cfunc.add_summary   = cwrapper.prototype("c_void_p ensemble_config_add_summary( ens_config, char*)")
cfunc.add_gen_kw    = cwrapper.prototype("c_void_p ensemble_config_add_gen_kw( ens_config, char*)")
cfunc.add_gen_data  = cwrapper.prototype("c_void_p ensemble_config_add_gen_data( ens_config, char*)")
cfunc.add_field     = cwrapper.prototype("c_void_p ensemble_config_add_field( ens_config, char*, ecl_grid)")
cfunc.alloc_keylist_from_var_type = cwrapper.prototype("c_void_p ensemble_config_alloc_keylist_from_var_type(ens_config, int)")
