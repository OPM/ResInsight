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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf import AnalysisIterConfig

class AnalysisConfig(BaseCClass):
    def __init__(self):
        c_ptr = AnalysisConfig.cNamespace().alloc()
        super(AnalysisConfig , self).__init__(c_ptr)


    def get_rerun(self):
        return AnalysisConfig.cNamespace().get_rerun(self)

    def set_rerun(self, rerun):
        AnalysisConfig.cNamespace().set_rerun(self, rerun)

    def get_rerun_start(self):
        return AnalysisConfig.cNamespace().get_rerun_start(self)

    def set_rerun_start(self, index):
        AnalysisConfig.cNamespace().set_rerun_start(self, index)

    def get_log_path(self):
        return AnalysisConfig.cNamespace().get_log_path(self)

    def set_log_path(self, path):
        AnalysisConfig.cNamespace().set_log_path(self, path)

    def get_alpha(self):
        return AnalysisConfig.cNamespace().get_alpha(self)

    def set_alpha(self, alpha):
        AnalysisConfig.cNamespace().set_alpha(self, alpha)

    def get_merge_observations(self):
        return AnalysisConfig.cNamespace().get_merge_observations(self)

    def set_merge_observations(self, merge_observations):
        return AnalysisConfig.cNamespace().set_merge_observations(self, merge_observations)

    def get_iter_config(self):
        """ @rtype: AnalysisIterConfig """
        return AnalysisConfig.cNamespace().get_iter_config(self).setParent(self)

    def get_min_realisations(self):
        """ @rtype: int """
        return AnalysisConfig.cNamespace().get_min_realisations( self )

    def set_min_realisations(self , min_realisations):
        AnalysisConfig.cNamespace().set_min_realisations( self , min_realisations )

    def get_stop_long_running(self):
        """ @rtype: bool """
        return AnalysisConfig.cNamespace().get_stop_long_running( self )
    
    def set_stop_long_running(self, stop_long_running):
        AnalysisConfig.cNamespace().set_stop_long_running(self, stop_long_running)
  
    def get_max_runtime(self):
        """ @rtype: int """
        return AnalysisConfig.cNamespace().get_max_runtime( self )

    def set_max_runtime(self, max_runtime):
        AnalysisConfig.cNamespace().set_max_runtime( self, max_runtime )

    def free(self):
        AnalysisConfig.cNamespace().free(self)
        
    def activeModuleName(self):
        return AnalysisConfig.cNamespace().get_active_module_name(self)

    ##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("analysis_config", AnalysisConfig)
cwrapper.registerType("analysis_config_obj", AnalysisConfig.createPythonObject)
cwrapper.registerType("analysis_config_ref", AnalysisConfig.createCReference)

AnalysisConfig.cNamespace().alloc                  = cwrapper.prototype("c_void_p analysis_config_alloc()")
AnalysisConfig.cNamespace().free                   = cwrapper.prototype("void analysis_config_free( analysis_config )")
AnalysisConfig.cNamespace().get_rerun              = cwrapper.prototype("int analysis_config_get_rerun( analysis_config )")
AnalysisConfig.cNamespace().set_rerun              = cwrapper.prototype("void analysis_config_set_rerun( analysis_config, bool)")
AnalysisConfig.cNamespace().get_rerun_start        = cwrapper.prototype("int analysis_config_get_rerun_start( analysis_config )")
AnalysisConfig.cNamespace().set_rerun_start        = cwrapper.prototype("void analysis_config_set_rerun_start( analysis_config, int)")
AnalysisConfig.cNamespace().get_log_path           = cwrapper.prototype("char* analysis_config_get_log_path( analysis_config)")
AnalysisConfig.cNamespace().set_log_path           = cwrapper.prototype("void analysis_config_set_log_path( analysis_config, char*)")
AnalysisConfig.cNamespace().get_alpha              = cwrapper.prototype("double analysis_config_get_alpha(analysis_config)")
AnalysisConfig.cNamespace().set_alpha              = cwrapper.prototype("void analysis_config_set_alpha(analysis_config, double)")
AnalysisConfig.cNamespace().get_merge_observations = cwrapper.prototype("bool analysis_config_get_merge_observations(analysis_config)")
AnalysisConfig.cNamespace().set_merge_observations = cwrapper.prototype("void analysis_config_set_merge_observations(analysis_config, bool)")
AnalysisConfig.cNamespace().get_iter_config        = cwrapper.prototype("analysis_iter_config_ref analysis_config_get_iter_config(analysis_config)")
AnalysisConfig.cNamespace().get_min_realisations   = cwrapper.prototype("int analysis_config_get_min_realisations(analysis_config)")
AnalysisConfig.cNamespace().set_min_realisations   = cwrapper.prototype("void analysis_config_set_min_realisations(analysis_config, int)")
AnalysisConfig.cNamespace().get_max_runtime        = cwrapper.prototype("int analysis_config_get_max_runtime(analysis_config)")
AnalysisConfig.cNamespace().set_max_runtime        = cwrapper.prototype("void analysis_config_set_max_runtime(analysis_config, int)")
AnalysisConfig.cNamespace().get_stop_long_running  = cwrapper.prototype("bool analysis_config_get_stop_long_running(analysis_config)")
AnalysisConfig.cNamespace().set_stop_long_running  = cwrapper.prototype("void analysis_config_set_stop_long_running(analysis_config, bool)")
AnalysisConfig.cNamespace().get_active_module_name  = cwrapper.prototype("char* analysis_config_get_active_module_name(analysis_config)")
