#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_server.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import time
import os
import signal
import json
import sys 

try:
    from unittest2 import skipIf, skipUnless, skipIf
except ImportError:
    from unittest import skipIf, skipUnless, skipIf

from ert.server import ErtServer,ErtCmdError
from ert.util import StringList, TimeVector, DoubleVector

from ert.test import ExtendedTestCase , TestAreaContext



class ServerTest(ExtendedTestCase):
    def setUp(self):
        self.config_path = self.createTestPath("local/resopt/config/simple")
        self.config_file = "config"


    def testCreate(self):
        with TestAreaContext("server/server") as work_area:
            work_area.copy_directory_content(self.config_path)

            with self.assertRaises(IOError):
                ert_server = ErtServer( "Does/not/exist" )
                
            ert_server = ErtServer(self.config_file)
            self.assertTrue( ert_server.isConnected() )
            ert_server.close()
            self.assertTrue( not ert_server.isConnected() )
            
            ert_server = ErtServer()
            self.assertTrue( not ert_server.isConnected() )
            ert_server.open( self.config_file )
            self.assertTrue( ert_server.isConnected() )
            
            cmd = ["STATUS"]
            res = ert_server.evalCmd( cmd )
            self.assertEqual( res , ["READY"] )

            with self.assertRaises(ErtCmdError):
                res = ert_server.evalCmd( ["UNKNWON-COMMAND"])
            

    def testSimulations(self):
        with TestAreaContext("server/server") as work_area:
            work_area.copy_directory_content(self.config_path)

            
            ert_server = ErtServer(self.config_file)
            cmd = ["INIT_SIMULATIONS"]
            with self.assertRaises(ErtCmdError):
                res = ert_server.evalCmd( cmd )
                
            cmd = ["INIT_SIMULATIONS" , 100 , "Init_case"]
            res = ert_server.evalCmd( cmd )

            cmd = ["STATUS"]
            res = ert_server.evalCmd( cmd )
            self.assertEqual( res , ["RUNNING" , 0 , 0 ])
            
            cmd = ["START_SIMULATION" , "0"]
            
