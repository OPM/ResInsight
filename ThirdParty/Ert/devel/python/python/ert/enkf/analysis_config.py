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


class AnalysisConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

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

    def free(self):
        AnalysisConfig.cNamespace().free(self)

    ##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("analysis_config", AnalysisConfig)
cwrapper.registerType("analysis_config_obj", AnalysisConfig.createPythonObject)
cwrapper.registerType("analysis_config_ref", AnalysisConfig.createCReference)


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

