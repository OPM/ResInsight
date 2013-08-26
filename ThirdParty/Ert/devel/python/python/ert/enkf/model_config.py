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
from    ert.cwrap.cwrap            import *
from    ert.cwrap.cclass           import CClass
from    ert.util.tvector           import * 
from    enkf_enum                  import *
from    ert.job_queue.forward_model import ForwardModel
from    libenkf import *
from    ert.sched.libsched import *
from    ert.sched.history import HistoryType
from    ert.sched.sched_file import *
from    ert.ecl.ecl_sum import *
from    ert.sched.history import *
class ModelConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    @property
    def get_enkf_sched_file(self):
        return cfunc.get_enkf_sched_file( self )

    def set_enkf_sched_file(self, file):
        cfunc.get_enkf_sched_file( self , file)

    @property
    def get_history_source(self):
        return HistoryType(c_ptr = cfunc.get_history_source( self ) , parent = self)

    def set_history_source(self, history_source, sched_file, refcase):
        return cfunc.select_history(self, history_source, sched_file, refcase)
        

    @property
    def get_max_internal_submit(self):
        return cfunc.get_max_internal_submit( self )

    def set_max_internal_submit(self, max):
        cfunc.get_max_internal_submit( self , max)

    @property     
    def get_forward_model(self):
        ford_model = ForwardModel( c_ptr = cfunc.get_forward_model( self ), parent = self)
        return ford_model

    @property
    def get_case_table_file(self):
        return cfunc.get_case_table_file(self)

    @property
    def get_runpath_as_char(self):
        return cfunc.get_runpath_as_char(self)

    def select_runpath(self, path_key):
        return cfunc.select_runpath(self, path_key)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "model_config" , ModelConfig )

cfunc = CWrapperNameSpace("model_config")

##################################################################
##################################################################

cfunc.free                    = cwrapper.prototype("void model_config_free( model_config )")
cfunc.get_enkf_sched_file     = cwrapper.prototype("char* model_config_get_enkf_sched_file( model_config )")
cfunc.set_enkf_sched_file     = cwrapper.prototype("void model_config_set_enkf_sched_file( model_config, char*)")
cfunc.get_history_source      = cwrapper.prototype("c_void_p model_config_get_history_source(model_config)")
cfunc.select_history          = cwrapper.prototype("bool model_config_select_history(model_config, history_type, c_void_p, ecl_sum)")
cfunc.get_forward_model       = cwrapper.prototype("c_void_p model_config_get_forward_model(model_config)")
cfunc.get_max_internal_submit = cwrapper.prototype("int model_config_get_max_internal_submit(model_config)")
cfunc.set_max_internal_submit = cwrapper.prototype("void model_config_set_max_internal_submit(model_config, int)")
cfunc.get_case_table_file     = cwrapper.prototype("char* model_config_get_case_table_file(model_config)")
cfunc.get_runpath_as_char     = cwrapper.prototype("char* model_config_get_runpath_as_char(model_config)")
cfunc.select_runpath          = cwrapper.prototype("void model_config_select_runpath(model_config, char*)")
                                 
