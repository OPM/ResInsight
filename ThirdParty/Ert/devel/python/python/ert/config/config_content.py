#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'config_content.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os.path

from ert.config import UnrecognizedEnum, ContentTypeEnum, ConfigError, ConfigPrototype
from ert.cwrap import BaseCClass


class ContentNode(BaseCClass):
    TYPE_NAME = "content_node"

    _iget = ConfigPrototype("char* config_content_node_iget( content_node , int)")
    _size = ConfigPrototype("int config_content_node_get_size( content_node )")
    _get_full_string = ConfigPrototype("char* config_content_node_get_full_string( content_node , char* )")
    _iget_type = ConfigPrototype("config_content_type_enum config_content_node_iget_type( content_node , int)")
    _iget_as_abspath = ConfigPrototype("char* config_content_node_iget_as_abspath( content_node , int)")
    _iget_as_relpath = ConfigPrototype("char* config_content_node_iget_as_relpath( content_node , int)")
    _iget_as_string = ConfigPrototype("char* config_content_node_iget( content_node , int)")
    _iget_as_int = ConfigPrototype("int config_content_node_iget_as_int( content_node , int)")
    _iget_as_double = ConfigPrototype("double config_content_node_iget_as_double( content_node , int)")
    _iget_as_path = ConfigPrototype("char* config_content_node_iget_as_path( content_node , int)")
    _iget_as_bool = ConfigPrototype("bool config_content_node_iget_as_bool( content_node , int)")

    typed_get = {
        ContentTypeEnum.CONFIG_STRING: _iget_as_string,
        ContentTypeEnum.CONFIG_INT: _iget_as_int,
        ContentTypeEnum.CONFIG_FLOAT: _iget_as_double,
        ContentTypeEnum.CONFIG_PATH: _iget_as_path,
        ContentTypeEnum.CONFIG_EXISTING_PATH: _iget_as_path,
        ContentTypeEnum.CONFIG_BOOL: _iget_as_bool
    }


    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        return self._size()

    def __assertIndex(self , index):
        if isinstance(index, int):
            if index < 0:
                index += len(self)
                
            if not 0 <= index < len(self):
                raise IndexError
            return index
        else:
            raise TypeError("Invalid argument type: %s" % index)

                
    def __getitem__(self, index):
        index = self.__assertIndex(index)
        
        content_type = self._iget_type(index)
        typed_get = self.typed_get[content_type]
        return typed_get(self, index)

    def getPath(self , index = 0, absolute = True , relative_start = None):
        index = self.__assertIndex(index)
        content_type = self._iget_type(index)
        if content_type in [ContentTypeEnum.CONFIG_EXISTING_PATH , ContentTypeEnum.CONFIG_PATH]:
            if absolute:
                return self._iget_as_abspath(index)
            else:
                if relative_start is None:
                    return self._iget_as_relpath(index)
                else:
                    abs_path = self._iget_as_abspath(index)
                    return os.path.relpath( abs_path , relative_start )
        else:
            raise TypeError("The getPath() method can only be called on PATH items")
        
    def content(self, sep=" "):
        return self._get_full_string(sep)


    def igetString(self , index):
        index = self.__assertIndex(index)
        return self._iget(index )


    def asList(self):
        return [x for x in self]



class ContentItem(BaseCClass):
    TYPE_NAME = "content_item"

    _size = ConfigPrototype("int config_content_item_get_size( content_item )")
    _iget_content_node = ConfigPrototype("content_node_ref config_content_item_iget_node( content_item , int)")

    # Not possible to create new python instances of this class
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")


    def __len__(self):
        return self._size()


    def __getitem__(self, index):
        if isinstance(index, int):
            if index < 0:
                index += len(self)

            if (index >= 0) and (index < len(self)):
                return self._iget_content_node(index).setParent(self)
            else:
                raise IndexError
        else:
            raise TypeError("[] operator must have integer index")

    def last(self):
        return self[-1]

    def getValue(self , item_index = -1 , node_index = 0):
        node = self[item_index]
        return node[node_index]




class ConfigContent(BaseCClass):
    TYPE_NAME = "config_content"

    _free = ConfigPrototype("void config_content_free( config_content )")
    _is_valid = ConfigPrototype("bool config_content_is_valid( config_content )")
    _has_key = ConfigPrototype("bool config_content_has_item( config_content , char*)")
    _get_item = ConfigPrototype("content_item_ref config_content_get_item( config_content , char*)")
    _get_errors = ConfigPrototype("config_error_ref config_content_get_errors( content_node )")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __contains__(self , key):
        return self._has_key(key)

    def __getitem__(self , key):
        if key in self:
            item = self._get_item(key)
            item.setParent( self )
            return item
        else:
            raise KeyError("Unrecognized key:%s" % key)


    def hasKey(self,key):
        return key in self


    def getValue(self , key , item_index = -1 , node_index = 0):
        item = self[key]
        return item.getValue( item_index , node_index )
        

    def isValid(self):
        return self._is_valid()

        
    def free(self):
        self._free()


    def getErrors(self):
        """ @rtype: ConfigError """
        return self._get_errors()
