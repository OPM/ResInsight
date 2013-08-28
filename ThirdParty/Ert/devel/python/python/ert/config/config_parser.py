#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'config_parser.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.config import UnrecognizedEnum, CONFIG_LIB
from ert.cwrap import CClass, CWrapper, CWrapperNameSpace


class SchemaItem(CClass):
    def __init__(self, keyword, required=False):
        c_ptr = cfunc.schema_alloc(keyword, required)
        self.init_cref(c_ptr, cfunc.schema_free)


    @classmethod
    def wrap(cls, c_ptr, parent):
        obj = object.__new__(cls)
        obj.init_cref(c_ptr, parent)
        return obj


    def iget_type( self, index ):
        return cfunc.schema_iget_type(self, index)

    def iset_type( self, index, schema_type ):
        cfunc.schema_iset_type(self, index, schema_type)

    def set_argc_minmax(self, minimum, maximum):
        cfunc.schema_set_argc_minmax(self, minimum, maximum)

#-----------------------------------------------------------------


class ContentItem(CClass):
    # Not possible to create new python instances of this class

    @classmethod
    def wrap(cls, c_ptr, parent):
        obj = object.__new__(cls)
        obj.init_cref(c_ptr, parent)
        return obj

    def __len__(self):
        return cfunc.content_size(self)


    def __getitem__(self, index):
        if isinstance(index, int):
            if (index >= 0) and (index < self.__len__()):
                c_ptr = cfunc.iget_content_node(self, index)
                return ContentNode.wrap(c_ptr, self)
            else:
                raise IndexError
        else:
            raise ValueError("[] operator must have integer index")


#-----------------------------------------------------------------



class ContentNode(CClass):
    # Not possible to create new python instances of this class

    @classmethod
    def wrap(cls, c_ptr, parent):
        obj = object.__new__(cls)
        obj.init_cref(c_ptr, parent)
        return obj

    def __len__(self):
        return cfunc.content_node_size(self)

    def __getitem__(self, index):
        if isinstance(index, int):
            if (index >= 0) and (index < self.__len__()):
                return cfunc.content_node_iget(self, index)
            else:
                raise IndexError
        else:
            raise ValueError("[] operator must have integer index")

    def content(self, sep=" "):
        return cfunc.content_full_string(self, sep)


#-----------------------------------------------------------------


class ConfigParser(CClass):
    def __init__(self):
        c_ptr = cfunc.config_alloc()
        self.init_cobj(c_ptr, cfunc.config_free)


    def add(self, keyword, required=False):
        c_ptr = cfunc.add(self, keyword, required)
        schema_item = SchemaItem.wrap(c_ptr, self)
        return schema_item


    def parse( self, config_file, comment_string="--", include_kw="INCLUDE", define_kw="DEFINE",
               unrecognized=UnrecognizedEnum.CONFIG_UNRECOGNIZED_WARN, validate=True):
        if os.path.exists(config_file):
            return cfunc.parse(self, config_file, comment_string, include_kw, define_kw, unrecognized, validate)
        else:
            raise IOError("File: %s does not exists")


    def __getitem__(self, keyword):
        if cfunc.has_content(self, keyword):
            c_ptr = cfunc.get_content(self, keyword)
            return ContentItem.wrap(c_ptr, self)
        else:
            return None

#-----------------------------------------------------------------

cwrapper = CWrapper(CONFIG_LIB)
cwrapper.registerType("config_parser", ConfigParser)
cwrapper.registerType("schema_item", SchemaItem)
cwrapper.registerType("content_item", ContentItem)
cwrapper.registerType("content_node", ContentNode)

cfunc = CWrapperNameSpace("config")

cfunc.add = cwrapper.prototype("c_void_p config_add_schema_item( config_parser , char* , bool)")
cfunc.config_alloc = cwrapper.prototype("c_void_p config_alloc( )")
cfunc.config_free = cwrapper.prototype("void config_free( config_parser )")
cfunc.parse = cwrapper.prototype("bool config_parse( config_parser , char* , char* , char* , char* , int , bool )")
cfunc.has_content = cwrapper.prototype("bool config_has_content_item( config_parser , char*) ")
cfunc.get_content = cwrapper.prototype("c_void_p config_get_content_item( config_parser , char*) ")

cfunc.schema_alloc = cwrapper.prototype("c_void_p config_schema_item_alloc( char* , bool )")
cfunc.schema_free = cwrapper.prototype("void config_schema_item_free( schema_item )")
cfunc.schema_iget_type = cwrapper.prototype("int config_schema_item_iget_type( schema_item ,int)")
cfunc.schema_iset_type = cwrapper.prototype("void config_schema_item_iset_type( schema_item , int , int)")
cfunc.schema_set_argc_minmax = cwrapper.prototype("void config_schema_item_set_argc_minmax( schema_item , int , int)")

cfunc.content_size = cwrapper.prototype("int config_content_item_get_size( content_item )")
cfunc.iget_content_node = cwrapper.prototype("int config_content_item_iget_node( content_item , int)")
cfunc.content_node_iget = cwrapper.prototype("char* config_content_node_iget( content_node , int)")
cfunc.content_node_size = cwrapper.prototype("int config_content_node_get_size( content_node )")
cfunc.content_full_string = cwrapper.prototype("char* config_content_node_get_full_string( content_node , char* )")


