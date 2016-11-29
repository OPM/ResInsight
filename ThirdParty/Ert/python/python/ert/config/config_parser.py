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

import sys
import os.path

from ert.config import UnrecognizedEnum, ContentTypeEnum , ConfigContent, ConfigPrototype
from cwrap import BaseCClass


class ConfigParser(BaseCClass):
    TYPE_NAME = "config_parser"

    _alloc = ConfigPrototype("void* config_alloc( )", bind=False)
    _add = ConfigPrototype("schema_item_ref config_add_schema_item( config_parser , char* , bool)")
    _free = ConfigPrototype("void config_free( config_parser )")
    _parse = ConfigPrototype("config_content_obj config_parse( config_parser , char* , char* , char* , char* , hash , config_unrecognized_enum , bool )")
    _get_schema_item = ConfigPrototype("schema_item_ref config_get_schema_item( config_parser , char*)")
    _has_schema_item = ConfigPrototype("bool config_has_schema_item( config_parser , char*)")

    def __init__(self):
        c_ptr = self._alloc()
        super(ConfigParser, self).__init__(c_ptr)


    def __contains__(self , keyword):
        return self._has_schema_item( keyword )


    def add(self, keyword, required=False , value_type = None):
        item = self._add(keyword, required).setParent( self )
        if value_type:
            item.iset_type( 0 , value_type )

        return item


    def getSchemaItem(self , keyword):
        if keyword in self:
            item = self._get_schema_item(keyword)
            item.setParent( self )
        else:
            raise KeyError("Config parser does not have item:%s" % keyword)


    def parse(self, config_file, comment_string="--", include_kw="INCLUDE", define_kw="DEFINE",
              pre_defined_kw_map=None, unrecognized=UnrecognizedEnum.CONFIG_UNRECOGNIZED_WARN, validate=True):
        """ @rtype: ConfigContent """

        assert isinstance(unrecognized, UnrecognizedEnum)


        if os.path.exists(config_file):
            config_content = self._parse(config_file, comment_string, include_kw, define_kw, pre_defined_kw_map, unrecognized, validate)
            config_content.setParser( self )

            if not config_content.isValid():
                if validate:
                    sys.stderr.write("Errors parsing:%s \n" % config_file)
                    for count, error in enumerate(config_content.getErrors()):
                        sys.stderr.write("  %02d:%s\n" % (count , error))

                    raise ValueError("Parsing:%s failed" % config_file)

            return config_content
        else:
            raise IOError("File: %s does not exists" % config_file)


    def free(self):
        self._free()
