#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'test_config.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.config import ContentTypeEnum, UnrecognizedEnum, SchemaItem, ContentItem, ContentNode, ConfigParser
from ert_tests import ExtendedTestCase


class ConfigTest(ExtendedTestCase):
    def setUp( self ):
        self.file_list = []


    def test_enums(self):
        self.assertTrue(ContentTypeEnum.CONFIG_STRING)
        self.assertTrue(ContentTypeEnum.CONFIG_INVALID)
        self.assertTrue(UnrecognizedEnum.CONFIG_UNRECOGNIZED_ERROR)

        self.assertEqual(ContentTypeEnum.CONFIG_STRING, 1)
        self.assertEqual(ContentTypeEnum.CONFIG_INVALID, 512)


    def test_parse(self):
        conf = ConfigParser()
        conf.add("FIELD", False)
        schema_item = conf.add("RSH_HOST", False)
        self.assertIsInstance(schema_item, SchemaItem)
        test_path = self.createTestPath("local/config/simple_config")
        self.assertTrue(conf.parse(test_path, unrecognized=UnrecognizedEnum.CONFIG_UNRECOGNIZED_IGNORE))


        content_item = conf["RSH_HOST"]
        self.assertIsInstance(content_item, ContentItem)
        self.assertIsNone(conf["BJARNE"])

        self.assertEqual(len(content_item), 1)
        self.assertRaises(ValueError, content_item.__getitem__, "BJARNE")
        self.assertRaises(IndexError, content_item.__getitem__, 10)

        content_node = content_item[0]
        self.assertIsInstance(content_node, ContentNode)

        self.assertEqual(len(content_node), 2)
        self.assertRaises(ValueError, content_node.__getitem__, "BJARNE")
        self.assertRaises(IndexError, content_node.__getitem__, 10)
        self.assertEqual(content_node[1], "be-lx633214:2")

        self.assertEqual(content_node.content(sep=","), "be-lx655082:2,be-lx633214:2")
        self.assertEqual(content_node.content(), "be-lx655082:2 be-lx633214:2")

        content_item = conf["FIELD"]
        self.assertEqual(len(content_item), 5)
        self.assertRaises(IOError, ConfigParser.parse, conf, "DoesNotExits")


    def test_schema(self):
        schema_item = SchemaItem("TestItem")
        self.assertIsInstance(schema_item, SchemaItem)
        self.assertEqual(schema_item.iget_type(6), ContentTypeEnum.CONFIG_STRING)
        schema_item.iset_type(0, ContentTypeEnum.CONFIG_INT)
        self.assertEqual(schema_item.iget_type(0), ContentTypeEnum.CONFIG_INT)
        schema_item.set_argc_minmax(3, 6)

        del schema_item
