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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import StringList


class GenKwConfig(BaseCClass):
    TYPE_NAME = "gen_kw_config"

    _free                 = EnkfPrototype("void  gen_kw_config_free( gen_kw_config )")
    _alloc_empty          = EnkfPrototype("void* gen_kw_config_alloc_empty( char*, char* )", bind = False)
    _get_template_file    = EnkfPrototype("char* gen_kw_config_get_template_file(gen_kw_config)")
    _set_template_file    = EnkfPrototype("void  gen_kw_config_set_template_file(gen_kw_config , char*)")
    _get_parameter_file   = EnkfPrototype("char* gen_kw_config_get_parameter_file(gen_kw_config)")
    _set_parameter_file   = EnkfPrototype("void  gen_kw_config_set_parameter_file(gen_kw_config, char*)")
    _alloc_name_list      = EnkfPrototype("stringlist_obj gen_kw_config_alloc_name_list(gen_kw_config)")
    _should_use_log_scale = EnkfPrototype("bool  gen_kw_config_should_use_log_scale(gen_kw_config, int)")
    _get_key              = EnkfPrototype("char* gen_kw_config_get_key(gen_kw_config)")
    _size                 = EnkfPrototype("int   gen_kw_config_get_data_size(gen_kw_config)")
    _iget_name            = EnkfPrototype("char* gen_kw_config_iget_name(gen_kw_config, int)")


    def __init__(self, key, template_file , parameter_file , tag_fmt = "<%s>"):
        """
         @type key: str
         @type tag_fmt: str
        """
        c_ptr = self._alloc_empty(key, tag_fmt)
        if c_ptr:
            super(GenKwConfig, self).__init__(c_ptr)
        else:
            raise ValueError('Could not instantiate GenKwConfig with key="%s" and tag_fmt="%s"' % (key, tag_fmt))
        self._key = key
        self._tag_fmt = tag_fmt
        self.setTemplateFile(template_file)
        self.setParameterFile(parameter_file)
        self.__str__ = self.__repr__


    def setTemplateFile(self, template_file):
        self._set_template_file(template_file)

    def getTemplateFile(self):
        return self._get_template_file()

    def getParameterFile(self):
        return self._get_parameter_file()

    def setParameterFile(self, parameter_file):
        self._set_parameter_file(parameter_file)

    def getKeyWords(self):
        """ @rtype: StringList """
        return self._alloc_name_list()

    def shouldUseLogScale(self, index):
        """ @rtype: bool """
        return self._should_use_log_scale(index)

    def free(self):
        self._free()

    def __repr__(self):
        return 'GenKwConfig(key = "%s", tag_fmt = "%s") at 0x%x' % (self._key, self._tag_fmt, self._address())

    def getKey(self):
        """ @rtype: str """
        return self._get_key()

    def __len__(self):
        return self._size()

    def __getitem__(self, index):
        """ @rtype: str """
        return self._iget_name(index)

    def __iter__(self):
        index = 0
        while index < len(self):
            yield self[index]
            index += 1
