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

from ert.config import UnrecognizedEnum, CONFIG_LIB, ContentTypeEnum
from ert.cwrap import BaseCClass, CWrapper

class ContentNode(BaseCClass):
    typed_get = {}
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        return ContentNode.cNamespace().size(self)

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
        
        content_type = ContentNode.cNamespace().iget_type(self, index)
        typed_get = self.typed_get[content_type]
        return typed_get( self , index )

        
    def content(self, sep=" "):
        return ContentNode.cNamespace().get_full_string(self, sep)


    def igetString(self , index):
        index = self.__assertIndex(index)
        return ContentNode.cNamespace().iget(self , index )


    def asList(self):
        return [x for x in self]



class ContentItem(BaseCClass):
    # Not possible to create new python instances of this class
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")


    def __len__(self):
        return ContentItem.cNamespace().size(self)


    def __getitem__(self, index):
        if isinstance(index, int):
            if index < 0:
                index += len(self)

            if (index >= 0) and (index < len(self)):
                return ContentItem.cNamespace().iget_content_node(self, index).setParent(self)
            else:
                raise IndexError
        else:
            raise TypeError("[] operator must have integer index")


    def getValue(self , item_index = -1 , node_index = 0):
        node = self[item_index]
        return node[node_index]




class ConfigContent(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __contains__(self , key):
        return ConfigContent.cNamespace().has_key(self , key)

    def __getitem__(self , key):
        if key in self:
            item = ConfigContent.cNamespace().get_item( self , key)
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
        return ConfigContent.cNamespace().is_valid( self )

        
    def free(self):
        ConfigContent.cNamespace().free(self)




cwrapper = CWrapper(CONFIG_LIB)
cwrapper.registerObjectType("config_content", ConfigContent)
cwrapper.registerObjectType("content_item", ContentItem)
cwrapper.registerObjectType("content_node", ContentNode)

ConfigContent.cNamespace().free     = cwrapper.prototype("void config_content_free( config_content )")
ConfigContent.cNamespace().is_valid = cwrapper.prototype("bool config_content_is_valid( config_content )")
ConfigContent.cNamespace().has_key = cwrapper.prototype("bool config_content_has_item( config_content , char*)")
ConfigContent.cNamespace().get_item = cwrapper.prototype("content_item_ref config_content_get_item( config_content , char*)")

ContentItem.cNamespace().size = cwrapper.prototype("int config_content_item_get_size( content_item )")
ContentItem.cNamespace().iget_content_node = cwrapper.prototype("content_node_ref config_content_item_iget_node( content_item , int)")

ContentNode.cNamespace().iget = cwrapper.prototype("char* config_content_node_iget( content_node , int)")
ContentNode.cNamespace().size = cwrapper.prototype("int config_content_node_get_size( content_node )")
ContentNode.cNamespace().get_full_string = cwrapper.prototype("char* config_content_node_get_full_string( content_node , char* )")
ContentNode.cNamespace().iget_type = cwrapper.prototype("config_content_type_enum config_content_node_iget_type( content_node , int)")

ContentNode.typed_get[ContentTypeEnum.CONFIG_INT] = iget_as_int = cwrapper.prototype("int config_content_node_iget_as_int( content_node , int)")
ContentNode.typed_get[ContentTypeEnum.CONFIG_BOOL] = iget_as_bool = cwrapper.prototype("bool config_content_node_iget_as_bool( content_node , int)")
ContentNode.typed_get[ContentTypeEnum.CONFIG_FLOAT] = iget_as_double = cwrapper.prototype("double config_content_node_iget_as_double( content_node , int)")
ContentNode.typed_get[ContentTypeEnum.CONFIG_PATH] = iget_as_path = cwrapper.prototype("char* config_content_node_iget_as_path( content_node , int)")
ContentNode.typed_get[ContentTypeEnum.CONFIG_STRING] = iget_as_string = cwrapper.prototype("char* config_content_node_iget( content_node , int)")

