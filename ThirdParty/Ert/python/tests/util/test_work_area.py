#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_work_area.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

from ert.test import ExtendedTestCase , TestAreaContext, TempAreaContext


class WorkAreaTest(ExtendedTestCase):

    def test_full_path(self):
        with TestAreaContext("TestArea") as test_area:
            with open("test_file" , "w") as fileH:
                fileH.write("Something")
            
                self.assertTrue( os.path.isfile( "test_file") )
            
            with self.assertRaises(IOError):
                full_path = test_area.getFullPath( "does/not/exists" )

            with self.assertRaises(IOError):
                full_path = test_area.getFullPath( "/already/absolute" )
                
            full_path = test_area.getFullPath( "test_file" )
            self.assertTrue( os.path.isfile( full_path ))
            self.assertTrue( os.path.isabs( full_path ))
            

    def test_temp_area(self):
        with TestAreaContext("TestArea") as test_area:
            cwd = os.getcwd()
            with open("file.txt" , "w") as f:
                f.write("File")
                
            with TempAreaContext("TempArea") as temp_area:
                self.assertEqual( cwd, os.getcwd())
                self.assertEqual( cwd, temp_area.get_cwd())
                temp_area.copy_file( "file.txt" )

                self.assertTrue( os.path.isfile( os.path.join( temp_area.getPath( ) , "file.txt")))

                os.mkdir("tmp")
                os.chdir("tmp")

            self.assertEqual( os.getcwd() , os.path.join( cwd , "tmp"))

            
    def test_IOError(self):
        with TestAreaContext("TestArea") as test_area:
            with self.assertRaises(IOError):
                test_area.copy_file( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.install_file( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.copy_directory( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.copy_parent_directory( "Does/not/exist" )

            os.makedirs("path1/path2")
            with open("path1/file.txt" , "w") as f:
                f.write("File ...")

            with self.assertRaises(IOError):
                test_area.copy_directory( "path1/file.txt" )
                
    def test_sync(self):
        with TestAreaContext("test_sync") as t:
            with open("file.txt" , "w") as f:
                f.write("content")

            t.sync()
            self.assertTrue( os.path.isfile( "file.txt"))
