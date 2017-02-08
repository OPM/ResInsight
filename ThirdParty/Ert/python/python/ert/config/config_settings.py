#  Copyright (C) 2017  Statoil ASA, Norway.
#
#  The file 'config_settings.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.config import ConfigPrototype, ContentTypeEnum
from ert.config import SchemaItem


class ConfigSettings(BaseCClass):
    TYPE_NAME = "config_settings"
    _alloc = ConfigPrototype("void* config_settings_alloc(char*)", bind = False)
    _free  = ConfigPrototype("void  config_settings_free(config_settings)")

    _add_setting = ConfigPrototype("bool config_settings_add_setting(config_settings , char* , config_content_type_enum, char*)")
    _add_double_setting = ConfigPrototype("void config_settings_add_double_setting(config_settings , char* , double)")
    _add_int_setting = ConfigPrototype("void config_settings_add_int_setting(config_settings , char* , int)")
    _add_string_setting = ConfigPrototype("void config_settings_add_string_setting(config_settings , char* , char*)")
    _add_bool_setting = ConfigPrototype("void config_settings_add_bool_setting(config_settings , char* , bool)")
    _has_key = ConfigPrototype("bool config_settings_has_key(config_settings , char*)")
    _get_type    = ConfigPrototype("config_content_type_enum config_settings_get_value_type(config_settings, char*)")
    _init_parser = ConfigPrototype("void config_settings_init_parser(config_settings, config_parser, bool)")
    _apply       = ConfigPrototype("void config_settings_apply(config_settings, config_content)")
    _alloc_keys  = ConfigPrototype("stringlist_obj config_settings_alloc_keys(config_settings)")

    _get         = ConfigPrototype("char* config_settings_get_value(config_settings, char*)")
    _get_int     = ConfigPrototype("int config_settings_get_int_value(config_settings, char*)")
    _get_double  = ConfigPrototype("double config_settings_get_double_value(config_settings, char*)")
    _get_bool    = ConfigPrototype("bool config_settings_get_bool_value(config_settings, char*)")

    _set         = ConfigPrototype("bool config_settings_set_value(config_settings, char*, char*)")
    _set_int     = ConfigPrototype("bool config_settings_set_int_value(config_settings, char*, int)")
    _set_double  = ConfigPrototype("bool config_settings_set_double_value(config_settings, char*, double)")
    _set_bool    = ConfigPrototype("bool config_settings_set_bool_value(config_settings, char*, bool)")

    
    
    
    def __init__(self , root_key):
        c_ptr = ConfigSettings._alloc( root_key )
        super(ConfigSettings, self).__init__(c_ptr)


    def __contains__(self, key):
        return self._has_key( key )


    def __getitem__(self,key):
        if key in self:
            value_type = self._get_type( key )
            if value_type == ContentTypeEnum.CONFIG_INT:
                return self._get_int( key )

            if value_type == ContentTypeEnum.CONFIG_FLOAT:
                return self._get_double( key )

            if value_type == ContentTypeEnum.CONFIG_BOOL:
                return self._get_bool( key )

            return self._get( key )
        else:
            raise KeyError("Settings object does not support key:%s" % key)


    def __setitem__(self, key, value):
        if key in self:
            value_type = self._get_type( key )

            if value_type == ContentTypeEnum.CONFIG_INT:
                self._set_int( key , value)
                return
                
            if value_type == ContentTypeEnum.CONFIG_FLOAT:
                self._set_double( key , value)
                return
                
            if value_type == ContentTypeEnum.CONFIG_BOOL:
                if type(value) is bool:
                    self._set_bool( key , value )
                    return 
                else:
                    raise TypeError("Type of %s should be boolean" % key)
                
            if not self._set( key , value ):
                raise TypeError("Setting %s=%s failed \n" % (key , value))
        else:
            raise KeyError("Settings object does not support key:%s" % key)

        
        
    def free(self):
        self._free( )

        
    def addSetting(self, key , value_type, initial_value):
        if not self._add_setting( key , value_type , str(initial_value) ):
            raise TypeError("Failed to add setting %s with initial value %s" % (key , initial_value))


    def addIntSetting(self, key , initial_value):
        self._add_int_setting( key , initial_value )

    def addDoubleSetting(self, key , initial_value):
        self._add_double_setting( key , initial_value )

    def addStringSetting(self, key , initial_value):
        self._add_string_setting( key , initial_value )

    def addBoolSetting(self, key , initial_value):
        self._add_bool_setting( key , initial_value )
    
    def initParser(self , parser, required = False):
        self._init_parser( parser , required )


    def apply(self , config_content):
        self._apply( config_content )


    def keys(self):
        return self._alloc_keys( )


    def getType(self , key):
        if key in self:
            return self._get_type( key )
        else:
            raise KeyError("No such key:%s" % key)
