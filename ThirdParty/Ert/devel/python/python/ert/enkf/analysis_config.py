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
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_rerun(self):
        return cfunc.get_rerun( self )

    def set_rerun(self, rerun):
        cfunc.set_rerun(self, rerun)
        
    @property
    def get_rerun_start(self):
        return cfunc.get_rerun_start( self )

    def set_rerun_start(self, int):
        cfunc.set_rerun_start( self , int)

    @property
    def get_log_path(self):
        return cfunc.get_log_path( self )

    def set_log_path(self, path):
        cfunc.set_log_path( self, path) 

    @property
    def get_alpha(self):
        return cfunc.get_alpha( self )

    def set_alpha(self, alpha):
        cfunc.set_alpha( self , alpha)

    @property
    def get_merge_observations(self):
        return cfunc.get_merge_observations( self )
    
    def set_merge_observations(self, merge_observations):
        return cfunc.set_merge_observations( self , merge_observations)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "analysis_config" , AnalysisConfig )

cfunc = CWrapperNameSpace("analysis_config")


cfunc.free                   = cwrapper.prototype("void analysis_config_free( analysis_config )")
cfunc.get_rerun              = cwrapper.prototype("int analysis_config_get_rerun( analysis_config )")
cfunc.set_rerun              = cwrapper.prototype("void analysis_config_set_rerun( analysis_config, bool)")
cfunc.get_rerun_start        = cwrapper.prototype("int analysis_config_get_rerun_start( analysis_config )")
cfunc.set_rerun_start        = cwrapper.prototype("void analysis_config_set_rerun_start( analysis_config, int)")
cfunc.get_log_path           = cwrapper.prototype("char* analysis_config_get_log_path( analysis_config)")
cfunc.set_log_path           = cwrapper.prototype("void analysis_config_set_log_path( analysis_config, char*)")
cfunc.get_alpha              = cwrapper.prototype("double analysis_config_get_alpha(analysis_config)")
cfunc.set_alpha              = cwrapper.prototype("void analysis_config_set_alpha(analysis_config, double)")
cfunc.get_merge_observations = cwrapper.prototype("bool analysis_config_get_merge_observations(analysis_config)")
cfunc.set_merge_observations = cwrapper.prototype("void analysis_config_set_merge_observations(analysis_config, bool)")

