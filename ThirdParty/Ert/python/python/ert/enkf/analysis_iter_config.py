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
from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB


class AnalysisIterConfig(BaseCClass):
    def __init__(self):
        c_ptr = AnalysisIterConfig.cNamespace().alloc()
        super(AnalysisIterConfig , self).__init__(c_ptr)

    def getNumIterations(self):
        """ @rtype: int """
        return AnalysisIterConfig.cNamespace().get_num_iterations(self)

    def setNumIterations(self, num_iterations):
        AnalysisIterConfig.cNamespace().set_num_iterations(self, num_iterations)

    def numIterationsSet(self):
        return AnalysisIterConfig.cNamespace().num_iterations_set(self)

    def getNumRetries(self):
        """ @rtype: int """
        return AnalysisIterConfig.cNamespace().get_num_retries(self)

    def getCaseFormat(self):
        """ @rtype: str """
        return AnalysisIterConfig.cNamespace().get_case_fmt(self)

    def setCaseFormat(self, case_fmt):
        AnalysisIterConfig.cNamespace().set_case_fmt(self, case_fmt)

    def caseFormatSet(self):
        return AnalysisIterConfig.cNamespace().case_fmt_set(self)

    def free(self):
        AnalysisIterConfig.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("analysis_iter_config", AnalysisIterConfig)


AnalysisIterConfig.cNamespace().alloc = cwrapper.prototype("c_void_p analysis_iter_config_alloc( )")
AnalysisIterConfig.cNamespace().free = cwrapper.prototype("void analysis_iter_config_free( analysis_iter_config )")
AnalysisIterConfig.cNamespace().set_num_iterations = cwrapper.prototype("void analysis_iter_config_set_num_iterations(analysis_iter_config, int)")
AnalysisIterConfig.cNamespace().get_num_iterations = cwrapper.prototype("int analysis_iter_config_get_num_iterations(analysis_iter_config)")
AnalysisIterConfig.cNamespace().get_num_retries = cwrapper.prototype("int analysis_iter_config_get_num_retries_per_iteration(analysis_iter_config)")
AnalysisIterConfig.cNamespace().num_iterations_set = cwrapper.prototype("bool analysis_iter_config_num_iterations_set(analysis_iter_config)")
AnalysisIterConfig.cNamespace().set_case_fmt = cwrapper.prototype("void analysis_iter_config_set_case_fmt( analysis_iter_config , char* )")
AnalysisIterConfig.cNamespace().get_case_fmt = cwrapper.prototype("char* analysis_iter_config_get_case_fmt( analysis_iter_config)")
AnalysisIterConfig.cNamespace().case_fmt_set = cwrapper.prototype("bool analysis_iter_config_case_fmt_set(analysis_iter_config)")
