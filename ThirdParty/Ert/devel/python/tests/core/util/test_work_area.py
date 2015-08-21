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

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

from ert.test import ExtendedTestCase , TestAreaContext


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
            
