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
import time
import socket 
import os
import signal
import json
import sys 
import logging

try:
    from unittest2 import skipIf, skipUnless, skipIf
except ImportError:
    from unittest import skipIf, skipUnless, skipIf

from ert.server import ErtSocket, ErtClient
from ert.util import StringList, TimeVector, DoubleVector

from ert.test import ExtendedTestCase , TestAreaContext






class SocketTest(ExtendedTestCase):
    def setUp(self):
         self.config_path = self.createTestPath("local/resopt/config/simple")
         self.config_file = "config"
         self.port = 9125
         self.host = "localhost"
         
         self.logger = logging.Logger("ert-server-test")
         self.logger.addHandler( logging.NullHandler() )



    def runCommand(self , send , expect):
        result = ErtClient.runCommand(send , self.port , self.host )
        #self.assertEqual( result , expect )


         
    def test_connect(self):
        with TestAreaContext("server/socket") as work_area:
            work_area.copy_directory_content(self.config_path)
            pid = os.fork()
            if pid == 0:
                s = ErtSocket.connect(self.config_file , self.port , self.host , self.logger)
                s.listen( )
            else:
                time.sleep(0.50)
                
                self.runCommand( ["STATUS"] , ["OPEN"] )
                self.runCommand( ["QUIT"] , ["QUIT"] )
            
                with self.assertRaises(Exception):
                    self.runCommand( "INVALID" )
                    
                with self.assertRaises(Exception):
                    self.runCommand( ["MISSING_COMMAND"] )

