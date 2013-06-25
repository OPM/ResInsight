#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_config_node.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class EnkfConfigNode(CClass):
    
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
cwrapper.registerType( "enkf_config_node" , EnkfConfigNode )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("enkf_config_node")


cfunc.free                   = cwrapper.prototype("void enkf_config_node_free( enkf_config_node )")
cfunc.get_impl_type          = cwrapper.prototype("c_void_p enkf_config_node_get_impl_type(enkf_config_node)")
cfunc.get_ref                = cwrapper.prototype("c_void_p enkf_config_node_get_ref(enkf_config_node)")
cfunc.is_valid               = cwrapper.prototype("bool enkf_config_node_is_valid(enkf_config_node)")
cfunc.get_min_std_file       = cwrapper.prototype("char* enkf_config_node_get_min_std_file(enkf_config_node)")
cfunc.get_enkf_outfile       = cwrapper.prototype("char* enkf_config_node_get_enkf_outfile(enkf_config_node)")
cfunc.get_enkf_infile        = cwrapper.prototype("char* enkf_config_node_get_enkf_infile(enkf_config_node)")
cfunc.update_gen_kw          = cwrapper.prototype("void enkf_config_node_update_gen_kw(enkf_config_node, char*, char*, char*, char*, char*)")
cfunc.update_state_field     = cwrapper.prototype("void enkf_config_node_update_state_field(enkf_config_node, int, double, double)")
cfunc.update_parameter_field = cwrapper.prototype("void enkf_config_node_update_parameter_field(enkf_config_node, char*, char*, char*, int, double, double, char*, char*)")
cfunc.update_general_field   = cwrapper.prototype("void enkf_config_node_update_general_field(enkf_config_node, char*, char*, char*, char*, int, double, double, char*, char*, char*)")
cfunc.update_gen_data        = cwrapper.prototype("void enkf_config_node_update_gen_data(enkf_config_node, int, int, char*, char*, char*, char*, char*, char*)")
