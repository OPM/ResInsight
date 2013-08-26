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
class FieldConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_type(self):
        return cfunc.get_type(self)

    @property
    def get_truncation_mode(self):
        return cfunc.get_truncation_mode(self)

    @property
    def get_truncation_min(self):
        return cfunc.get_truncation_min(self)

    @property
    def get_init_transform_name(self):
        return cfunc.get_init_transform_name(self)

    @property
    def get_output_transform_name(self):
        return cfunc.get_output_transform_name(self)

    @property
    def get_truncation_max(self):
        return cfunc.get_truncation_max(self)

    @property
    def get_nx(self):
        return cfunc.get_nx(self)

    @property
    def get_ny(self):
        return cfunc.get_ny(self)

    @property
    def get_nz(self):
        return cfunc.get_ny(self)

    def ijk_active(self, i,j,k):
        return cfunc.ijk_active(self,i,j,k)
##################################################################
cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "field_config" , FieldConfig )

cfunc = CWrapperNameSpace("field_config")
##################################################################
##################################################################
cfunc.free                      = cwrapper.prototype("void field_config_free( field_config )")
cfunc.get_type                  = cwrapper.prototype("int field_config_get_type(field_config)")
cfunc.get_truncation_mode       = cwrapper.prototype("int field_config_get_truncation_mode(field_config)")
cfunc.get_truncation_min        = cwrapper.prototype("double field_config_get_truncation_min(field_config)")
cfunc.get_truncation_max        = cwrapper.prototype("double field_config_get_truncation_max(field_config)")
cfunc.get_init_transform_name   = cwrapper.prototype("char* field_config_get_init_transform_name(field_config)")
cfunc.get_output_transform_name = cwrapper.prototype("char* field_config_get_output_transform_name(field_config)")
cfunc.ijk_active                = cwrapper.prototype("bool field_config_ijk_active(field_config, int, int, int)")
cfunc.get_nx                    = cwrapper.prototype("int field_config_get_nx(field_config)")
cfunc.get_ny                    = cwrapper.prototype("int field_config_get_ny(field_config)")
cfunc.get_nz                    = cwrapper.prototype("int field_config_get_nz(field_config)")
cfunc.get_grid                  = cwrapper.prototype("c_void_p field_config_get_grid(field_config)")
