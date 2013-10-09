#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysis_iter_config.py' is part of ERT - Ensemble based Reservoir Tool.
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


class AnalysisIterConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getNumIterations(self):
        return AnalysisIterConfig.cNamespace().get_num_iterations(self)

    def setNumIterations(self, num_iterations):
        AnalysisIterConfig.cNamespace().set_num_iterations(self, num_iterations)

    def getCaseFormat(self):
        return AnalysisIterConfig.cNamespace().getCaseFormat(self)

    def setCaseFormat(self, case_fmt):
        AnalysisIterConfig.cNamespace().setCaseFormat(self, case_fmt)

    def getRunpathFormat(self):
        return AnalysisIterConfig.cNamespace().get_case_fmt(self)

    def setRunpathFormat(self, case_fmt):
        AnalysisIterConfig.cNamespace().set_case_fmt(self, case_fmt)

    ##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("analysis_iter_config", AnalysisIterConfig)
cwrapper.registerType("analysis_iter_config_obj", AnalysisIterConfig.createPythonObject)
cwrapper.registerType("analysis_iter_config_ref", AnalysisIterConfig.createCReference)


AnalysisIterConfig.cNamespace().free = cwrapper.prototype("void analysis_iter_config_free( analysis_iter_config )")
AnalysisIterConfig.cNamespace().set_num_iterations = cwrapper.prototype("void analysis_iter_config_set_num_iterations(analysis_iter_config, int)")
AnalysisIterConfig.cNamespace().get_num_iterations = cwrapper.prototype("int analysis_iter_config_get_num_iterations(analysis_iter_config)")
AnalysisIterConfig.cNamespace().set_case_fmt = cwrapper.prototype("void analysis_iter_config_set_case_fmt( analysis_iter_config , char* )")
AnalysisIterConfig.cNamespace().get_case_fmt = cwrapper.prototype("char* analysis_iter_config_get_case_fmt( analysis_iter_config)")
AnalysisIterConfig.cNamespace().set_runpath_fmt = cwrapper.prototype("void analysis_iter_config_set_runpath_fmt( analysis_iter_config , char*)");
AnalysisIterConfig.cNamespace().get_runpath_fmt = cwrapper.prototype("char* analysis_iter_config_get_runpath_fmt( analysis_iter_config)")
