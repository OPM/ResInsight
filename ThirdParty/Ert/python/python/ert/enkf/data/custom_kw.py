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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.config import CustomKWConfig


class CustomKW(BaseCClass):
    TYPE_NAME = "custom_kw"

    _alloc          = EnkfPrototype("void*  custom_kw_alloc(custom_kw_config)", bind = False)
    _free           = EnkfPrototype("void   custom_kw_free(custom_kw)")
    _fload          = EnkfPrototype("bool   custom_kw_fload(custom_kw, char*)")
    _key_is_null    = EnkfPrototype("bool   custom_kw_key_is_null(custom_kw, char*)")
    _iget_as_double = EnkfPrototype("double custom_kw_iget_as_double(custom_kw, int)")
    _iget_as_string = EnkfPrototype("char*  custom_kw_iget_as_string(custom_kw, int)")
    _set_string     = EnkfPrototype("void   custom_kw_set_string(custom_kw, char*, char*)")
    _set_double     = EnkfPrototype("void   custom_kw_set_double(custom_kw, char*, double)")
    _get_config     = EnkfPrototype("custom_kw_config_ref custom_kw_get_config(custom_kw)")

    def __init__(self, custom_kw_config):
        assert isinstance(custom_kw_config, CustomKWConfig)
        c_ptr = self._alloc(custom_kw_config)
        if c_ptr:
            super(CustomKW, self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct CustomKW with given config!')

    def fload(self, filename):
        """
        @type filename: str
        @rtype: bool
        """
        return self._fload(filename)


    def __getitem__(self, key):
        """ @rtype: str or float """
        config = self.getConfig()

        if not key in config:
            raise KeyError("The key: '%s' is not available!" % key)

        index = config.indexOfKey(key)

        if self._key_is_null(key):
            return None

        if config.keyIsDouble(key):
            return self._iget_as_double(index)

        return self._iget_as_string(index)

    def __setitem__(self, key, value):
        """
        @type key: str
        @type value: float|int|str
        """

        config = self.getConfig()

        if not key in config:
            raise KeyError("The key: '%s' is not available!" % key)

        if isinstance(value, (float, int, long)):
            self._set_double(key, value)
        else:
            self._set_string(key, str(value))


    def getConfig(self):
        """ @rtype: CustomKWConfig """
        return self._get_config()

    def free(self):
       self._free()

    def __repr__(self):
        return 'CustomKw() %s' % self._ad_str()
