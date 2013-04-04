#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'field_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class GenDataConfig(CClass):
    
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
cwrapper.registerType( "field_config" , GenDataConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("field_config")


cfunc.free                      = cwrapper.prototype("void field_config_free( field_config )")
cfunc.get_type                  = cwrapper.prototype("int field_config_get_type(field_config)")
cfunc.get_truncation_mode       = cwrapper.prototype("int field_config_get_truncation_mode(field_config)")
cfunc.get_truncation_min        = cwrapper.prototype("double field_config_get_truncation_min(field_config)")
cfunc.get_truncation_max        = cwrapper.prototype("double field_config_get_truncation_max(field_config)")
cfunc.get_init_transform_name   = cwrapper.prototype("char* field_config_get_init_transform_name(field_config)")
cfunc.get_output_transform_name = cwrapper.prototype("char* field_config_get_output_transform_name(field_config)")
cfunc.get_init_file_fmt         = cwrapper.prototype("char* field_config_get_init_file_fmt(field_config)")
