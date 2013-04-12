#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ecl_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class EclConfig(CClass):
    
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
cwrapper.registerType( "ecl_config" , EclConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_config")


cfunc.free               = cwrapper.prototype("void ecl_config_free( ecl_config )")
cfunc.get_eclbase        = cwrapper.prototype("char* ecl_config_get_eclbase( ecl_config )")
cfunc.get_data_file      = cwrapper.prototype("char* ecl_config_get_data_file(ecl_config)")
cfunc.get_gridfile       = cwrapper.prototype("char* ecl_config_get_gridfile(ecl_config)")
cfunc.set_gridfile       = cwrapper.prototype("void ecl_config_set_grid(ecl_config, char*)")
cfunc.get_schedule       = cwrapper.prototype("char* ecl_config_get_schedule_file(ecl_config)")
cfunc.set_schedule       = cwrapper.prototype("void ecl_config_set_schedule_file(ecl_config, char*)")
cfunc.get_init_section   = cwrapper.prototype("char* ecl_config_get_init_section(ecl_config)")
cfunc.set_init_section   = cwrapper.prototype("void ecl_config_set_init_section(ecl_config, char*)")
cfunc.get_refcase_name   = cwrapper.prototype("char* ecl_config_get_refcase_name(ecl_config)")
cfunc.set_refcase_name   = cwrapper.prototype("void ecl_config_load_refcase(ecl_config, char*)")
cfunc.get_static_kw_list = cwrapper.prototype("c_void_p ecl_config_get_static_kw_list(ecl_config)")
cfunc.clear_static_kw    = cwrapper.prototype("void ecl_config_clear_static_kw(ecl_config)")
cfunc.add_static_kw      = cwrapper.prototype("void ecl_config_add_static_kw(ecl_config, char*)")
cfunc.get_grid           = cwrapper.prototype("c_void_p ecl_config_get_grid(ecl_config)")
