#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'model_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class ModelConfig(CClass):
    
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
cwrapper.registerType( "model_config" , ModelConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("model_config")


cfunc.free                = cwrapper.prototype("void model_config_free( model_config )")
cfunc.get_enkf_sched_file = cwrapper.prototype("char* model_config_get_enkf_sched_file( model_config )")
cfunc.set_enkf_sched_file = cwrapper.prototype("void model_config_set_enkf_sched_file( model_config, char*)")
cfunc.get_history_source  = cwrapper.prototype("int model_config_get_history_source(model_config)")
cfunc.set_history_source  = cwrapper.prototype("void model_config_set_history_source(model_config, int)")
cfunc.get_forward_model   = cwrapper.prototype("c_void_p model_config_get_forward_model(model_config)")
cfunc.get_max_resample    = cwrapper.prototype("int model_config_get_max_resample(model_config)")
cfunc.set_max_resample    = cwrapper.prototype("void model_config_set_max_resample(model_config, int)")
cfunc.get_case_table_file = cwrapper.prototype("char* model_config_get_case_table_file(model_config)")
cfunc.get_runpath_as_char = cwrapper.prototype("char* model_config_get_runpath_as_char(model_config)")
cfunc.set_runpath_fmt     = cwrapper.prototype("void model_config_set_runpath_fmt(model_config, char*)")
                                 
