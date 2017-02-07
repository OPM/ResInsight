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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf import AnalysisIterConfig
from ert.analysis import AnalysisModule
from ert.util import StringList


class AnalysisConfig(BaseCClass):
    TYPE_NAME = "analysis_config"
    _alloc = EnkfPrototype("void* analysis_config_alloc()", bind = False)
    _free = EnkfPrototype("void analysis_config_free( analysis_config )")
    _get_rerun = EnkfPrototype("int analysis_config_get_rerun( analysis_config )")
    _set_rerun = EnkfPrototype("void analysis_config_set_rerun( analysis_config, bool)")
    _get_rerun_start = EnkfPrototype("int analysis_config_get_rerun_start( analysis_config )")
    _set_rerun_start = EnkfPrototype("void analysis_config_set_rerun_start( analysis_config, int)")
    _get_log_path = EnkfPrototype("char* analysis_config_get_log_path( analysis_config)")
    _set_log_path = EnkfPrototype("void analysis_config_set_log_path( analysis_config, char*)")
    _get_merge_observations = EnkfPrototype("bool analysis_config_get_merge_observations(analysis_config)")
    _set_merge_observations = EnkfPrototype("void analysis_config_set_merge_observations(analysis_config, bool)")
    _get_iter_config = EnkfPrototype("analysis_iter_config_ref analysis_config_get_iter_config(analysis_config)")
    _have_enough_realisations = EnkfPrototype("bool analysis_config_have_enough_realisations(analysis_config, int, int)")
    _get_max_runtime = EnkfPrototype("int analysis_config_get_max_runtime(analysis_config)")
    _set_max_runtime = EnkfPrototype("void analysis_config_set_max_runtime(analysis_config, int)")
    _get_stop_long_running = EnkfPrototype("bool analysis_config_get_stop_long_running(analysis_config)")
    _set_stop_long_running = EnkfPrototype("void analysis_config_set_stop_long_running(analysis_config, bool)")
    _get_active_module_name = EnkfPrototype("char* analysis_config_get_active_module_name(analysis_config)")
    _get_module_list = EnkfPrototype("stringlist_obj analysis_config_alloc_module_names(analysis_config)")
    _get_module = EnkfPrototype("analysis_module_ref analysis_config_get_module(analysis_config, char*)")
    _select_module = EnkfPrototype("bool analysis_config_select_module(analysis_config, char*)")
    _has_module = EnkfPrototype("bool analysis_config_has_module(analysis_config, char*)")
    _get_alpha = EnkfPrototype("double analysis_config_get_alpha(analysis_config)")
    _set_alpha = EnkfPrototype("void analysis_config_set_alpha(analysis_config, double)")
    _get_std_cutoff = EnkfPrototype("double analysis_config_get_std_cutoff(analysis_config)")
    _set_std_cutoff = EnkfPrototype("void analysis_config_set_std_cutoff(analysis_config, double)")
    _set_global_std_scaling = EnkfPrototype("void analysis_config_set_global_std_scaling(analysis_config, double)")
    _get_global_std_scaling = EnkfPrototype("double analysis_config_get_global_std_scaling(analysis_config)")

    def __init__(self):
        c_ptr = AnalysisConfig._alloc()
        super(AnalysisConfig , self).__init__(c_ptr)


    def get_rerun(self):
        return self._get_rerun()

    def set_rerun(self, rerun):
        self._set_rerun(rerun)

    def get_rerun_start(self):
        return self._get_rerun_start()

    def set_rerun_start(self, index):
        self._set_rerun_start(index)

    def get_log_path(self):
        return self._get_log_path()

    def set_log_path(self, path):
        self._set_log_path(path)

    def getEnkfAlpha(self):
        """ :rtype: float """
        return self._get_alpha()

    def setEnkfAlpha(self, alpha):
        self._set_alpha(alpha)

    def getStdCutoff(self):
        """ :rtype: float """
        return self._get_std_cutoff()

    def setStdCutoff(self, std_cutoff):
        self._set_std_cutoff(std_cutoff)

    def get_merge_observations(self):
        return self._get_merge_observations()

    def set_merge_observations(self, merge_observations):
        return self._set_merge_observations(merge_observations)

    def getAnalysisIterConfig(self):
        """ @rtype: AnalysisIterConfig """
        return self._get_iter_config().setParent(self)

    def get_stop_long_running(self):
        """ @rtype: bool """
        return self._get_stop_long_running()
    
    def set_stop_long_running(self, stop_long_running):
        self._set_stop_long_running(stop_long_running)
  
    def get_max_runtime(self):
        """ @rtype: int """
        return self._get_max_runtime()

    def set_max_runtime(self, max_runtime):
        self._set_max_runtime(max_runtime)

    def free(self):
        self._free()
        
    def activeModuleName(self):
        """ :rtype: str """
        return self._get_active_module_name()

    def getModuleList(self):
        """ :rtype: StringList """
        return self._get_module_list()

    def getModule(self, module_name):
        """ @rtype: AnalysisModule """
        return self._get_module(module_name)

    def hasModule(self, module_name):
        """ @rtype: bool """
        return self._has_module(module_name)

    
    def selectModule(self, module_name):
        """ @rtype: bool """
        return self._select_module(module_name)

    def getActiveModule(self):
        """ :rtype: AnalysisModule """
        return self.getModule(self.activeModuleName())

    def setGlobalStdScaling(self, std_scaling):
        self._set_global_std_scaling(std_scaling)

    def getGlobalStdScaling(self):
        return self._get_global_std_scaling()

    def haveEnoughRealisations(self, realizations, ensemble_size):
        return self._have_enough_realisations(realizations, ensemble_size)


