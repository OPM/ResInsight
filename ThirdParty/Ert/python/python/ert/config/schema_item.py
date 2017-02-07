#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file  is part of ERT - Ensemble based Reservoir Tool.
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
import ctypes

from ert.util import UtilPrototype
from ert.config import ContentTypeEnum , ConfigPrototype
from cwrap import BaseCClass


class SchemaItem(BaseCClass):
    TYPE_NAME = "schema_item"

    _alloc = ConfigPrototype("void* config_schema_item_alloc( char* , bool )", bind=False)
    _free = ConfigPrototype("void config_schema_item_free( schema_item )")
    _iget_type = ConfigPrototype("config_content_type_enum config_schema_item_iget_type( schema_item, int)")
    _iset_type = ConfigPrototype("void config_schema_item_iset_type( schema_item , int , config_content_type_enum)")
    _set_argc_minmax = ConfigPrototype("void config_schema_item_set_argc_minmax( schema_item , int , int)")
    _add_alternative = ConfigPrototype("void config_schema_item_add_indexed_alternative(schema_item , int , char*)")
    _set_deprecated = ConfigPrototype("void config_schema_item_set_deprecated(schema_item ,  char*)")
    _valid_string = ConfigPrototype("bool config_schema_item_valid_string(config_content_type_enum ,  char*)", bind = False)
    _sscanf_bool = UtilPrototype("bool util_sscanf_bool( char* , bool*)", bind = False)
    
    def __init__(self, keyword, required=False):
        c_ptr = self._alloc(keyword, required)
        super(SchemaItem, self).__init__(c_ptr)

    @classmethod
    def validString(cls , value_type, value):
        return cls._valid_string( value_type , value )

        
    
    @classmethod
    def convert(cls, value_type, value_string):
        if cls.validString(value_type , value_string):
            if value_type == ContentTypeEnum.CONFIG_INT:
                return int(value_string)

            if value_type == ContentTypeEnum.CONFIG_FLOAT:
                return float(value_string)

            if value_type == ContentTypeEnum.CONFIG_BOOL:
                value = ctypes.c_bool()
                SchemaItem._sscanf_bool( value_string , ctypes.byref( value ))
                return value.value

            return value_string
        else:
            raise ValueError("Invalid string value: %s" % value_string)

        

    def iget_type( self, index):
        """ @rtype: ContentTypeEnum """
        return self._iget_type(index)

    def iset_type( self, index, schema_type ):
        """
        @type schema_type: ContentTypeEnum
        """
        assert isinstance(schema_type, ContentTypeEnum)
        self._iset_type(index, schema_type)

    def set_argc_minmax(self, minimum, maximum):
        self._set_argc_minmax(minimum, maximum)


    def initSelection(self , index , alternatives):
        for alt in alternatives:
            self.addAlternative( index , alt )

            
    def addAlternative(self , index , alt):
        self._add_alternative( index , alt )

    def setDeprecated(self , msg):
        """This method can be used to mark this item as deprecated.

        If the deprecated item is used in a configuration file the
        @msg will be added to the warnings of the ConfigContent
        object,
        """
        self._set_deprecated( msg )
        
        
    def free(self):
        self._free()
