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
import os

import ert
from ert.config import ContentTypeEnum, UnrecognizedEnum, SchemaItem, ContentItem, ContentNode, ConfigParser, ConfigContent,ConfigSettings
from cwrap import Prototype, clib
from ert.test import ExtendedTestCase, TestAreaContext


class TestConfigPrototype(Prototype):
    lib = ert.load("libconfig")

    def __init__(self, prototype, bind=False):
        super(TestConfigPrototype, self).__init__(TestConfigPrototype.lib, prototype, bind=bind)

# Adding extra functions to the ConfigContent object for the ability
# to test low level C functions which are not exposed in Python.
_safe_iget      = TestConfigPrototype("char* config_content_safe_iget( config_content , char* , int , int)")
_iget           = TestConfigPrototype("char* config_content_iget( config_content , char* , int , int)")
_iget_as_int    = TestConfigPrototype("int config_content_iget_as_int( config_content , char* , int , int)")
_iget_as_bool   = TestConfigPrototype("bool config_content_iget_as_bool( config_content , char* , int , int)")
_iget_as_double = TestConfigPrototype("double config_content_iget_as_double( config_content , char* , int , int)")
_get_occurences = TestConfigPrototype("int config_content_get_occurences( config_content , char* )")


class ConfigTest(ExtendedTestCase):
    def setUp( self ):
        self.file_list = []


    def test_enums(self):
        source_file_path = "libconfig/include/ert/config/config_schema_item.h"
        self.assertEnumIsFullyDefined(ContentTypeEnum, "config_item_types", source_file_path)
        self.assertEnumIsFullyDefined(UnrecognizedEnum, "config_schema_unrecognized_enum", source_file_path)


    def test_item_types(self):
        with TestAreaContext("config/types") as test_area:
            with open("config" , "w") as f:
                f.write("TYPE_ITEM 10 3.14 TruE  String  file\n")
                
            conf = ConfigParser()
            schema_item = conf.add("TYPE_ITEM", False)
            schema_item.iset_type(0 , ContentTypeEnum.CONFIG_INT )
            schema_item.iset_type(1 , ContentTypeEnum.CONFIG_FLOAT )
            schema_item.iset_type(2 , ContentTypeEnum.CONFIG_BOOL )
            schema_item.iset_type(3 , ContentTypeEnum.CONFIG_STRING )
            schema_item.iset_type(4 , ContentTypeEnum.CONFIG_PATH )
            self.assertFalse( "TYPE_XX" in conf )
            self.assertTrue( "TYPE_ITEM" in conf )
            
            content = conf.parse("config")
            type_item = content["TYPE_ITEM"][0]
            int_value = type_item[0]
            self.assertEqual( int_value , 10 )
            self.assertEqual( type_item.igetString(0) , "10")

            float_value = type_item[1]
            self.assertEqual( float_value , 3.14 )
            self.assertEqual( type_item.igetString(1) , "3.14")

            bool_value = type_item[2]
            self.assertEqual( bool_value , True)
            self.assertEqual( type_item.igetString(2) , "TruE")
            
            string_value = type_item[3]
            self.assertEqual( string_value , "String")
            self.assertEqual( type_item.igetString(3) , "String")
            
            path_value = type_item[4]
            self.assertEqual( path_value , "file")
            self.assertEqual( type_item.igetString(4) , "file")


            


    def test_parse(self):
        conf = ConfigParser()
        conf.add("FIELD", False)
        schema_item = conf.add("RSH_HOST", False)
        self.assertIsInstance(schema_item, SchemaItem)
        test_path = self.createTestPath("local/config/simple_config")
        content = conf.parse(test_path, unrecognized=UnrecognizedEnum.CONFIG_UNRECOGNIZED_IGNORE)
        self.assertTrue( content.isValid() )
        
        
        content_item = content["RSH_HOST"]
        self.assertIsInstance(content_item, ContentItem)
        self.assertEqual(len(content_item), 1)
        with self.assertRaises(TypeError):
            content_item["BJARNE"]
        
        with self.assertRaises(IndexError):
            content_item[10]

        content_node = content_item[0]
        self.assertIsInstance(content_node, ContentNode)
        self.assertEqual(len(content_node), 2)
        self.assertEqual(content_node[1], "be-lx633214:2")
        self.assertEqual(content_node.content(sep=","), "be-lx655082:2,be-lx633214:2")
        self.assertEqual(content_node.content(), "be-lx655082:2 be-lx633214:2")


        content_item = content["FIELD"]
        self.assertEqual(len(content_item), 5)
        with self.assertRaises(IOError):
            conf.parse("DoesNotExits")
            

    def test_parse_invalid(self):
        conf = ConfigParser()
        conf.add("INT", value_type = ContentTypeEnum.CONFIG_INT )
        with TestAreaContext("config/parse2"):
            with open("config","w") as fileH:
                fileH.write("INT xx\n")

            with self.assertRaises(ValueError):
                conf.parse("config")

            content = conf.parse("config" , validate = False )
            self.assertFalse( content.isValid() )
            self.assertEqual( len(content.getErrors()) , 1 )


    def test_parse_deprecated(self):
        conf = ConfigParser()
        item = conf.add("INT", value_type = ContentTypeEnum.CONFIG_INT )
        msg = "ITEM INT IS DEPRECATED"
        item.setDeprecated( msg )
        with TestAreaContext("config/parse2"):
            with open("config","w") as fileH:
                fileH.write("INT 100\n")

            content = conf.parse("config"  )
            self.assertTrue( content.isValid() )

            warnings = content.getWarnings()
            self.assertEqual( len(warnings) , 1 )
            self.assertEqual( warnings[0] , msg )


    def test_parse_dotdot_relative(self):
        conf = ConfigParser()
        schema_item = conf.add("EXECUTABLE", False)
        schema_item.iset_type(0 , ContentTypeEnum.CONFIG_PATH )
        
        
        with TestAreaContext("config/parse_dotdot"):
            os.makedirs("cwd/jobs")
            os.makedirs("eclipse/bin")
            script_path = os.path.join( os.getcwd() , "eclipse/bin/script.sh")
            with open(script_path,"w") as f:
                f.write("This is a test script")

            with open("cwd/jobs/JOB","w") as fileH:
                fileH.write("EXECUTABLE ../../eclipse/bin/script.sh\n")

            os.makedirs("cwd/ert")
            os.chdir("cwd/ert")
            content = conf.parse("../jobs/JOB")
            item = content["EXECUTABLE"]
            node = item[0]
            self.assertEqual( script_path , node.getPath( ))
            


            
            
    def test_parser_content(self):
        conf = ConfigParser()
        conf.add("KEY2", False)
        schema_item = conf.add("KEY", False)
        schema_item.iset_type(2 , ContentTypeEnum.CONFIG_INT )
        schema_item.iset_type(3 , ContentTypeEnum.CONFIG_BOOL )
        schema_item.iset_type(4 , ContentTypeEnum.CONFIG_FLOAT )
        schema_item.iset_type(5 , ContentTypeEnum.CONFIG_PATH )
        schema_item = conf.add("NOT_IN_CONTENT", False)
        
        
        with TestAreaContext("config/parse2"):
            with open("config","w") as fileH:
                fileH.write("KEY VALUE1 VALUE2 100  True  3.14  path/file.txt\n")

            cwd0 = os.getcwd( )
            os.makedirs("tmp")
            os.chdir("tmp")
            content = conf.parse("../config")
            self.assertTrue( content.isValid() )
            self.assertTrue( "KEY" in content )
            self.assertFalse( "NOKEY" in content )

            self.assertFalse( "NOT_IN_CONTENT" in content )
            item = content["NOT_IN_CONTENT"]
            self.assertEqual( len(item) , 0 )
            
            with self.assertRaises(KeyError):
                content["Nokey"]
                
            item = content["KEY"]
            self.assertEqual(len(item) , 1)

            line = item[0]
            with self.assertRaises(TypeError):
                line.getPath(4)

            with self.assertRaises(TypeError):
                line.getPath()

                
            rel_path = line.getPath(index = 5, absolute = False)
            self.assertEqual( rel_path , "../path/file.txt" )
            get = line[5]
            self.assertEqual( get , "../path/file.txt")
            abs_path = line.getPath(index = 5)
            self.assertEqual( abs_path , os.path.join(cwd0 , "path/file.txt"))
            
            rel_path = line.getPath(index = 5, absolute = False , relative_start = "../")
            self.assertEqual( rel_path , "path/file.txt" )

            
            with self.assertRaises(IndexError):
                item[10]

            node = item[0]
            self.assertEqual(len(node) , 6)
            with self.assertRaises(IndexError):
                node[6]
            
            self.assertEqual( node[0] , "VALUE1" )
            self.assertEqual( node[1] , "VALUE2" )
            self.assertEqual( node[2] , 100 )
            self.assertEqual( node[3] , True )
            self.assertEqual( node[4] , 3.14)

            self.assertEqual( content.getValue( "KEY" , 0 , 1 ) , "VALUE2" )
            self.assertEqual( _iget( content , "KEY" , 0 , 1) , "VALUE2")

            self.assertEqual( content.getValue( "KEY" , 0 , 2 ) , 100 )
            self.assertEqual( _iget_as_int( content , "KEY" , 0 , 2) , 100)

            self.assertEqual( content.getValue( "KEY" , 0 , 3 ) , True )
            self.assertEqual( _iget_as_bool( content , "KEY" , 0 , 3) , True)

            self.assertEqual( content.getValue( "KEY" , 0 , 4 ) , 3.14 )
            self.assertEqual( _iget_as_double( content , "KEY" , 0 , 4) , 3.14)

            self.assertIsNone( _safe_iget( content , "KEY2" , 0 , 0))

            self.assertEqual(  _get_occurences( content , "KEY2" ) , 0)
            self.assertEqual(  _get_occurences( content , "KEY" ) , 1)
            self.assertEqual(  _get_occurences( content , "MISSING-KEY" ) , 0)
            
            

    def test_schema(self):
        schema_item = SchemaItem("TestItem")
        self.assertIsInstance(schema_item, SchemaItem)
        self.assertEqual(schema_item.iget_type(6), ContentTypeEnum.CONFIG_STRING)
        schema_item.iset_type(0, ContentTypeEnum.CONFIG_INT)
        self.assertEqual(schema_item.iget_type(0), ContentTypeEnum.CONFIG_INT)
        schema_item.set_argc_minmax(3, 6)

        self.assertTrue( SchemaItem.validString( ContentTypeEnum.CONFIG_INT , "100") )
        self.assertFalse( SchemaItem.validString( ContentTypeEnum.CONFIG_INT , "100.99") )
        self.assertTrue( SchemaItem.validString( ContentTypeEnum.CONFIG_FLOAT , "100.99") )
        self.assertFalse( SchemaItem.validString( ContentTypeEnum.CONFIG_FLOAT , "100.99X") )
        self.assertTrue( SchemaItem.validString( ContentTypeEnum.CONFIG_STRING , "100.99XX") )
        self.assertTrue( SchemaItem.validString( ContentTypeEnum.CONFIG_PATH , "100.99XX") )
        
        
        del schema_item


    

    def test_settings(self):
        cs = ConfigSettings("SETTINGS")
        
        with self.assertRaises(TypeError):
            cs.addSetting("SETTING1" , ContentTypeEnum.CONFIG_INT , 100.67)
            
        self.assertFalse( "SETTING1" in cs )
        cs.addSetting("SETTING2" , ContentTypeEnum.CONFIG_INT , 100)
        self.assertTrue( "SETTING2" in cs )

        with self.assertRaises(KeyError):
            value = cs["NO_SUCH_KEY"]

        self.assertEqual( cs["SETTING2"] , 100 )

        with self.assertRaises(KeyError):
            cs["NO_SUCH_KEY"] = 100


        parser = ConfigParser()
        cs = ConfigSettings("SETTINGS")
        cs.addSetting("A" , ContentTypeEnum.CONFIG_INT , 1)
        cs.addSetting("B" , ContentTypeEnum.CONFIG_INT , 1)
        cs.addSetting("C" , ContentTypeEnum.CONFIG_INT , 1)
        cs.initParser( parser )

        
        with TestAreaContext("config/parse3"):
            with open("config","w") as fileH:
                fileH.write("SETTINGS A 100\n")
                fileH.write("SETTINGS B 200\n")
                fileH.write("SETTINGS C 300\n")

            content = parser.parse( "config" )
        cs.apply( content )
        self.assertEqual( cs["A"] , 100)
        self.assertEqual( cs["B"] , 200)
        self.assertEqual( cs["C"] , 300)
            
        keys = cs.keys()
        self.assertTrue("A" in keys )
        self.assertTrue("B" in keys )
        self.assertTrue("C" in keys )
        self.assertEqual( len(keys) , 3 )

        cs = ConfigSettings("SETTINGS")
        cs.addDoubleSetting("A" , 1.0)
        self.assertEqual( ContentTypeEnum.CONFIG_FLOAT , cs.getType("A"))
        
        cs = ConfigSettings("SETTINGS")
        cs.addDoubleSetting("A" , 1.0)
        cs.addIntSetting("B" , 1)
        cs.addStringSetting("C" , "1")
        cs.addBoolSetting("D" ,  True)
        cs.initParser( parser )

        with TestAreaContext("config/parse4"):
            with open("config","w") as fileH:
                fileH.write("SETTINGS A 100.1\n")
                fileH.write("SETTINGS B 200\n")
                fileH.write("SETTINGS C 300\n")
                fileH.write("SETTINGS D False\n")

            content = parser.parse( "config" )

        cs.apply( content )
        self.assertEqual( cs["A"] , 100.1)
        self.assertEqual( cs["B"] , 200)
        self.assertEqual( cs["C"] , "300")
        self.assertEqual( cs["D"] , False)

        with self.assertRaises(Exception):
            cs["A"] = "Hei"


