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
from ert.cwrap import CWrapper, BaseCClass
from ert.ecl import EclSum
from ert.enkf import ENKF_LIB
from ert.sched import HistorySourceEnum, SchedFile
from ert.job_queue import ForwardModel
from ert.util import PathFormat


class ModelConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

        
    def hasHistory(self):
        return ModelConfig.cNamespace().has_history(self)

    def get_history_source(self):
        """ @rtype: HistorySourceEnum """
        return ModelConfig.cNamespace().get_history_source(self)

    def set_history_source(self, history_source, sched_file, refcase):
        """
         @type history_source: HistorySourceEnum
         @type sched_file: SchedFile
         @type refcase: EclSum
         @rtype: bool
        """
        assert isinstance(history_source, HistorySourceEnum)
        assert isinstance(sched_file, SchedFile)
        assert isinstance(refcase, EclSum)
        return ModelConfig.cNamespace().select_history(self, history_source, sched_file, refcase)


    def get_max_internal_submit(self):
        """ @rtype: int """
        return ModelConfig.cNamespace().get_max_internal_submit(self)

    def set_max_internal_submit(self, max_value):
        ModelConfig.cNamespace().get_max_internal_submit(self, max_value)

    def getForwardModel(self):
        """ @rtype: ForwardModel """
        return ModelConfig.cNamespace().get_forward_model(self).setParent(self)

    def get_case_table_file(self):
        """ @rtype: str """
        return ModelConfig.cNamespace().get_case_table_file(self)

    def getRunpathAsString(self):
        """ @rtype: str """
        return ModelConfig.cNamespace().get_runpath_as_char(self)

    def selectRunpath(self, path_key):
        """ @rtype: bool """
        return ModelConfig.cNamespace().select_runpath(self, path_key)

    def setRunpath(self, path_format):
        ModelConfig.cNamespace().set_runpath(self, path_format)

    def free(self):
        ModelConfig.cNamespace().free(self)

    def getFSType(self):
        return ModelConfig.cNamespace().get_fs_type( self )

    def getGenKWExportFile(self):
        """ @rtype: str """
        return ModelConfig.cNamespace().gen_kw_export_file(self)

    def runpathRequiresIterations(self):
        """ @rtype: bool """
        return ModelConfig.cNamespace().runpath_requires_iterations(self)

    def getJobnameFormat(self):
        """ @rtype: str """
        return ModelConfig.cNamespace().get_jobname_fmt(self)

    def getEnspath(self):
        """ @rtype: str """
        return ModelConfig.cNamespace().get_enspath(self)

    def getRunpathFormat(self):
        """ @rtype: PathFormat """
        return ModelConfig.cNamespace().get_runpath_fmt(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("model_config", ModelConfig)

ModelConfig.cNamespace().free = cwrapper.prototype("void model_config_free( model_config )")
ModelConfig.cNamespace().get_forward_model = cwrapper.prototype("forward_model_ref model_config_get_forward_model(model_config)")
ModelConfig.cNamespace().get_max_internal_submit = cwrapper.prototype("int model_config_get_max_internal_submit(model_config)")
ModelConfig.cNamespace().set_max_internal_submit = cwrapper.prototype("void model_config_set_max_internal_submit(model_config, int)")
ModelConfig.cNamespace().get_case_table_file = cwrapper.prototype("char* model_config_get_case_table_file(model_config)")
ModelConfig.cNamespace().get_runpath_as_char = cwrapper.prototype("char* model_config_get_runpath_as_char(model_config)")
ModelConfig.cNamespace().select_runpath = cwrapper.prototype("bool model_config_select_runpath(model_config, char*)")
ModelConfig.cNamespace().set_runpath = cwrapper.prototype("void model_config_set_runpath(model_config, char*)")
ModelConfig.cNamespace().get_fs_type = cwrapper.prototype("enkf_fs_type_enum model_config_get_dbase_type(model_config)")

ModelConfig.cNamespace().get_history = cwrapper.prototype("history_ref model_config_get_history(model_config)")
ModelConfig.cNamespace().get_history_source = cwrapper.prototype("history_source_enum model_config_get_history_source(model_config)")
ModelConfig.cNamespace().select_history = cwrapper.prototype("bool model_config_select_history(model_config, history_source_enum, sched_file, ecl_sum)")
ModelConfig.cNamespace().has_history = cwrapper.prototype("bool model_config_has_history(model_config)")
ModelConfig.cNamespace().gen_kw_export_file = cwrapper.prototype("char* model_config_get_gen_kw_export_file(model_config)")
ModelConfig.cNamespace().runpath_requires_iterations = cwrapper.prototype("bool model_config_runpath_requires_iter(model_config)")
ModelConfig.cNamespace().get_jobname_fmt = cwrapper.prototype("char* model_config_get_jobname_fmt(model_config)")
ModelConfig.cNamespace().get_runpath_fmt = cwrapper.prototype("path_fmt_ref model_config_get_runpath_fmt(model_config)")

ModelConfig.cNamespace().get_enspath = cwrapper.prototype("char* model_config_get_enspath(model_config)")
