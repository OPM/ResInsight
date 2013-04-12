#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_kw_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class GenKwConfig(CClass):
    
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
cwrapper.registerType( "gen_kw_config" , GenKwConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("gen_kw_config")


cfunc.free                   = cwrapper.prototype("void gen_kw_config_free( gen_kw_config )")
cfunc.get_template_file      = cwrapper.prototype("char* gen_kw_config_get_template_file(gen_kw_config)")
cfunc.get_init_file_fmt      = cwrapper.prototype("char* gen_kw_config_get_init_file_fmt(gen_kw_config)")
cfunc.get_parameter_file     = cwrapper.prototype("char* gen_kw_config_get_parameter_file(gen_kw_config)")
