#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'field_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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


class CustomKWConfig(BaseCClass):
    def __init__(self, key, result_file, output_file=None):
        """
        @type key: str
        @type result_file: str
        @type output_file: str
        """
        c_ptr = CustomKWConfig.cNamespace().alloc_empty(key, result_file, output_file)
        super(CustomKWConfig, self).__init__(c_ptr)

    def getName(self):
        """ @rtype: str """
        return CustomKWConfig.cNamespace().get_name(self)

    def getResultFile(self):
        """ @rtype: str """
        return CustomKWConfig.cNamespace().get_result_file(self)

    def getOutputFile(self):
        """ @rtype: str """
        return CustomKWConfig.cNamespace().get_output_file(self)

    def parseResultFile(self, result_file, result):
        """ @rtype: bool """
        return CustomKWConfig.cNamespace().parse_result_file(self, result_file, result)

    def keyIsDouble(self, key):
        """ @rtype: bool """
        return CustomKWConfig.cNamespace().key_is_double(self, key)

    def indexOfKey(self, key):
        """ @rtype: int """
        return CustomKWConfig.cNamespace().index_of_key(self, key)

    def __contains__(self, item):
        """ @rtype: bool """
        return CustomKWConfig.cNamespace().has_key(self, item)

    def __len__(self):
        """ @rtype: int """
        return CustomKWConfig.cNamespace().size(self)

    def __iter__(self):
        keys = self.getKeys()
        index = 0
        while index < len(keys):
            yield keys[index]
            index += 1

    def free(self):
       CustomKWConfig.cNamespace().free(self)

    def getKeys(self):
        """ @rtype: StringList """
        return CustomKWConfig.cNamespace().keys(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("custom_kw_config", CustomKWConfig)

CustomKWConfig.cNamespace().free = cwrapper.prototype("void custom_kw_config_free(custom_kw_config)")
CustomKWConfig.cNamespace().alloc_empty = cwrapper.prototype("c_void_p custom_kw_config_alloc_empty(char*, char*, char*)")
CustomKWConfig.cNamespace().get_name = cwrapper.prototype("char* custom_kw_config_get_name(custom_kw_config)")
CustomKWConfig.cNamespace().get_result_file = cwrapper.prototype("char* custom_kw_config_get_result_file(custom_kw_config)")
CustomKWConfig.cNamespace().get_output_file = cwrapper.prototype("char* custom_kw_config_get_output_file(custom_kw_config)")
CustomKWConfig.cNamespace().parse_result_file = cwrapper.prototype("bool custom_kw_config_parse_result_file(custom_kw_config, char*, stringlist)")
CustomKWConfig.cNamespace().has_key = cwrapper.prototype("bool custom_kw_config_has_key(custom_kw_config, char*)")
CustomKWConfig.cNamespace().key_is_double = cwrapper.prototype("bool custom_kw_config_key_is_double(custom_kw_config, char*)")
CustomKWConfig.cNamespace().index_of_key = cwrapper.prototype("int custom_kw_config_index_of_key(custom_kw_config, char*)")
CustomKWConfig.cNamespace().size = cwrapper.prototype("int custom_kw_config_size(custom_kw_config)")
CustomKWConfig.cNamespace().keys = cwrapper.prototype("stringlist_obj custom_kw_config_get_keys(custom_kw_config)")