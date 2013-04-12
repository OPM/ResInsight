#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'config_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os
import unittest
import stat
import math
import ert
import ert.ecl.ecl as ecl
import ert.config.config as config
import ert.config.config_enums as config_enums

import sys
from   test_util import *



class ConfigTest( unittest.TestCase ):
    
    def setUp( self ):
        self.file_list = []


    def test_enums(self):
        self.assertTrue( config_enums.content_type.CONFIG_STRING )
        self.assertTrue( config_enums.content_type.CONFIG_INVALID )
        self.assertTrue( config_enums.unrecognized.CONFIG_UNRECOGNIZED_ERROR )



    def test_parse(self):
        conf = config.ConfigParser()
        conf.add("FIELD" , False)
        schema_item = conf.add("RSH_HOST" , False)
        self.assertTrue( isinstance( schema_item , config.SchemaItem ))
        self.assertTrue( conf.parse("test-data/local/config/simple_config" , unrecognized = config_enums.unrecognized.CONFIG_UNRECOGNIZED_IGNORE) )
        
        
        content_item = conf["RSH_HOST"]
        self.assertTrue( isinstance( content_item , config.ContentItem ))
        self.assertTrue( conf["BJARNE"] is None )

        self.assertTrue( len(content_item) == 1)
        self.assertRaises( ValueError , content_item.__getitem__ , "BJARNE")
        self.assertRaises( IndexError , content_item.__getitem__ , 10 )
        
        content_node = content_item[0]
        self.assertTrue( isinstance( content_node , config.ContentNode ))
        
        self.assertTrue( len(content_node) == 2)
        self.assertRaises( ValueError , content_node.__getitem__ , "BJARNE")
        self.assertRaises( IndexError , content_node.__getitem__ , 10 )
        self.assertTrue( content_node[1] == "be-lx633214:2")

        self.assertTrue( content_node.content( sep = ",") == "be-lx655082:2,be-lx633214:2")
        self.assertTrue( content_node.content( ) == "be-lx655082:2 be-lx633214:2")


        content_item = conf["FIELD"]
        self.assertTrue( len(content_item) == 5)
        self.assertRaises(IOError , config.ConfigParser.parse , conf , "DoesNotExits")
        


    def test_schema(self):
        schema_item = config.SchemaItem("TestItem")
        self.assertTrue( isinstance( schema_item , config.SchemaItem ))
        self.assertTrue( schema_item.iget_type( 6 ) == config_enums.content_type.CONFIG_STRING )
        schema_item.iset_type( 0 , config_enums.content_type.CONFIG_INT )
        self.assertTrue( schema_item.iget_type( 0 ) == config_enums.content_type.CONFIG_INT )
        schema_item.set_argc_minmax( 3 , 6)
        


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( ConfigTest( 'test_enums' ))
    suite.addTest( ConfigTest( 'test_schema' ))
    suite.addTest( ConfigTest( 'test_parse' ))
    return suite

                   

if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )


