#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_kw_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.util import StringList


class GenKwConfig(BaseCClass):
    def __init__(self, key, template_file , parameter_file , tag_fmt = "<%s>"):
        """
         @type key: str
         @type tag_fmt: str
        """
        c_ptr = GenKwConfig.cNamespace().alloc_empty(key, tag_fmt)
        super(GenKwConfig, self).__init__(c_ptr)

        self.setTemplateFile(template_file)
        self.setParameterFile(parameter_file)


    def setTemplateFile(self, template_file):
        GenKwConfig.cNamespace().set_template_file(self , template_file)

    def getTemplateFile(self):
        return GenKwConfig.cNamespace().get_template_file(self)

    def getParameterFile(self):
        return GenKwConfig.cNamespace().get_parameter_file(self)

    def setParameterFile(self, parameter_file):
        GenKwConfig.cNamespace().set_parameter_file(self, parameter_file)

    def getKeyWords(self):
        """ @rtype: StringList """
        return GenKwConfig.cNamespace().alloc_name_list(self)

    def shouldUseLogScale(self, index):
        """ @rtype: bool """
        return GenKwConfig.cNamespace().should_use_log_scale(self, index)

    def free(self):
        GenKwConfig.cNamespace().free(self)

    def getKey(self):
        """ @rtype: str """
        return GenKwConfig.cNamespace().get_key(self)

    def __len__(self):
        return GenKwConfig.cNamespace().size(self)

    def __getitem__(self, index):
        """ @rtype: str """
        return GenKwConfig.cNamespace().iget_name(self, index)

    def __iter__(self):
        index = 0
        while index < len(self):
            yield self[index]
            index += 1


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("gen_kw_config", GenKwConfig)

GenKwConfig.cNamespace().free = cwrapper.prototype("void gen_kw_config_free( gen_kw_config )")
GenKwConfig.cNamespace().alloc_empty = cwrapper.prototype("c_void_p gen_kw_config_alloc_empty( char*, char* )")
GenKwConfig.cNamespace().get_template_file = cwrapper.prototype("char* gen_kw_config_get_template_file(gen_kw_config)")
GenKwConfig.cNamespace().set_template_file = cwrapper.prototype("void gen_kw_config_set_template_file(gen_kw_config , char*)")
GenKwConfig.cNamespace().get_parameter_file = cwrapper.prototype("char* gen_kw_config_get_parameter_file(gen_kw_config)")
GenKwConfig.cNamespace().set_parameter_file = cwrapper.prototype("void gen_kw_config_set_parameter_file( gen_kw_config, char* )")
GenKwConfig.cNamespace().alloc_name_list = cwrapper.prototype("stringlist_obj gen_kw_config_alloc_name_list(gen_kw_config)")

GenKwConfig.cNamespace().should_use_log_scale = cwrapper.prototype("bool gen_kw_config_should_use_log_scale(gen_kw_config, int)")
GenKwConfig.cNamespace().get_key = cwrapper.prototype("char* gen_kw_config_get_key(gen_kw_config)")
GenKwConfig.cNamespace().size = cwrapper.prototype("int gen_kw_config_get_data_size(gen_kw_config)")
GenKwConfig.cNamespace().iget_name = cwrapper.prototype("char* gen_kw_config_iget_name(gen_kw_config, int)")
