#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'analysis_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class AnalysisConfig(CClass):
    
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
cwrapper.registerType( "analysis_config" , AnalysisConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("analysis_config")


cfunc.free                   = cwrapper.prototype("void analysis_config_free( analysis_config )")
cfunc.get_rerun              = cwrapper.prototype("int analysis_config_get_rerun( analysis_config )")
cfunc.set_rerun              = cwrapper.prototype("void analysis_config_set_rerun analysis_config, bool)")
cfunc.get_rerun_start        = cwrapper.prototype("int analysis_config_get_rerun_start( analysis_config )")
cfunc.set_rerun_start        = cwrapper.prototype("void analysis_config_set_rerun_start( analysis_config, int)")
cfunc.get_log_path           = cwrapper.prototype("char* analysis_config_get_log_path( analysis_config)")
cfunc.set_log_path           = cwrapper.prototype("void analysis_config_set_log_path( analysis_config, char*)")
cfunc.get_alpha              = cwrapper.prototype("double analysis_config_get_alpha(analysis_config)")
cfunc.set_alpha              = cwrapper.prototype("void analysis_config_set_alpha(analysis_config, double)")
cfunc.get_merge_observations = cwrapper.prototype("bool analysis_config_get_merge_observations(analysis_config)")
cfunc.set_merge_observations = cwrapper.prototype("void analysis_config_set_merge_observations(analysis_config, int)")
cfunc.get_enkf_mode          = cwrapper.prototype("int analysis_config_get_enkf_mode(analysis_config)")
cfunc.set_enkf_mode          = cwrapper.prototype("void analysis_config_set_enkf_mode(analysis_config, int)")
cfunc.get_truncation         = cwrapper.prototype("double analysis_config_get_truncation(analysis_config)")
cfunc.get_truncation         = cwrapper.prototype("void analysis_config_set_truncation(analysis_config, double)")
