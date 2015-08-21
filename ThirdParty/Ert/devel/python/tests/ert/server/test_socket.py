#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_socket.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import random
import time
import socket 
import os
import signal
import json
import sys 
import logging
import datetime

try:
    from unittest2 import skipIf, skipUnless, skipIf
except ImportError:
    from unittest import skipIf, skipUnless, skipIf

from ert.server import ErtSocket, ErtClient
from ert.util import StringList, TimeVector, DoubleVector

from ert.test import ExtendedTestCase , TestAreaContext






class SocketTest(ExtendedTestCase):
    def setUp(self):
         self.base_port = random.randint( 1024 , 1024 * 30 )
         self.host = "localhost"
         self.pid = 0
         
         self.logger = logging.Logger("ert-server-test")
         self.logger.addHandler( logging.NullHandler() )



    def runCommand(self , send , port , expected = None):
        result = ErtClient.runCommand(send , port , self.host )
        if expected:
            self.assertEqual( result , expected )
        return result


    # If the client sends ["QUIT"] the server listen() method should
    # return, and the whole server process should terminate with
    # os._exit(0). If the client does not send ["QUIT"] the tearDown()
    # method should kill the server process.

    def startServer(self , config_file , port):
        self.pid = os.fork()
        if self.pid == 0:
            s = ErtSocket.connect(config_file , port , self.host , self.logger)
            s.listen( )
            os._exit(0) 
        else:
            time.sleep(0.50)
            return 

    def tearDown(self):
        if self.pid != 0:
            os.kill(self.pid, signal.SIGKILL)

         
    def test_connect(self):
        port = self.base_port 
        config_path = self.createTestPath("local/resopt/config/simple")
        with TestAreaContext("server/socket1") as work_area:
            work_area.copy_directory_content(config_path)
            self.startServer( "config" , port )
            
            self.runCommand( ["STATUS"] , port , expected = ["READY"] )
            
            with self.assertRaises(Exception):
                self.runCommand( "INVALID" , port )
                    
            with self.assertRaises(Exception):
                self.runCommand( ["MISSING_COMMAND"] , port )

            self.runCommand( ["QUIT"] , port , expected = ["QUIT"] )
            
            # Server is closed
            with self.assertRaises(Exception):
                self.runCommand( ["STATUS"] , port )



    def test_time_map(self):
        port = self.base_port + 1
        config_path = self.createTestPath("Statoil/config/with_data")
        with TestAreaContext("server/socket2") as work_area:
            work_area.copy_directory_content(config_path)
            self.startServer( "config" , port )

            data = self.runCommand(["TIME_STEP"] , port )
            self.assertTrue( isinstance( data[0] , datetime.datetime ))
            self.assertEqual( data[0] , datetime.datetime(2000 , 1 , 1, 0,0,0))

            self.runCommand( ["QUIT"] , port , expected = ["QUIT"] )



