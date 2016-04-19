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
from ert.enkf.config import CustomKWConfig


class CustomKW(BaseCClass):
    def __init__(self, custom_kw_config):
        assert isinstance(custom_kw_config, CustomKWConfig)
        c_ptr = CustomKW.cNamespace().alloc(custom_kw_config)
        super(CustomKW, self).__init__(c_ptr)

    def fload(self, filename):
        """
        @type filename: str
        @rtype: bool
        """
        return CustomKW.cNamespace().fload(self, filename)


    def __getitem__(self, key):
        """ @rtype: str or float """
        config = self.getConfig()

        if not key in config:
            raise KeyError("The key: '%s' is not available!" % key)

        index = config.indexOfKey(key)

        if CustomKW.cNamespace().key_is_null(self, key):
            return None

        if config.keyIsDouble(key):
            return CustomKW.cNamespace().iget_as_double(self, index)

        return CustomKW.cNamespace().iget_as_string(self, index)

    def __setitem__(self, key, value):
        """
        @type key: str
        @type value: float|int|str
        """

        config = self.getConfig()

        if not key in config:
            raise KeyError("The key: '%s' is not available!" % key)

        if isinstance(value, (float, int, long)):
            CustomKW.cNamespace().set_double(self, key, value)
        else:
            CustomKW.cNamespace().set_string(self, key, str(value))


    def getConfig(self):
        """ @rtype: CustomKWConfig """
        return CustomKW.cNamespace().get_config(self)

    def free(self):
       CustomKW.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("custom_kw", CustomKW)

CustomKW.cNamespace().free = cwrapper.prototype("void custom_kw_free(custom_kw)")
CustomKW.cNamespace().alloc = cwrapper.prototype("void* custom_kw_alloc(custom_kw_config)")
CustomKW.cNamespace().fload = cwrapper.prototype("bool custom_kw_fload(custom_kw, char*)")
CustomKW.cNamespace().get_config = cwrapper.prototype("custom_kw_config_ref custom_kw_get_config(custom_kw)")
CustomKW.cNamespace().key_is_null = cwrapper.prototype("bool custom_kw_key_is_null(custom_kw, char*)")
CustomKW.cNamespace().iget_as_double = cwrapper.prototype("double custom_kw_iget_as_double(custom_kw, int)")
CustomKW.cNamespace().iget_as_string = cwrapper.prototype("char* custom_kw_iget_as_string(custom_kw, int)")
CustomKW.cNamespace().set_string = cwrapper.prototype("void custom_kw_set_string(custom_kw, char*, char*)")
CustomKW.cNamespace().set_double = cwrapper.prototype("void custom_kw_set_double(custom_kw, char*, double)")
