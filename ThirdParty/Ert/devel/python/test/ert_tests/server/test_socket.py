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

try:
    from unittest2 import skipIf, skipUnless, skipIf
except ImportError:
    from unittest import skipIf, skipUnless, skipIf

from .test_client import TestClient
from ert.server import ErtSocket
from ert.util import StringList, TimeVector, DoubleVector

from ert.test import ExtendedTestCase , TestAreaContext






class SocketTest(ExtendedTestCase):
    def setUp(self):
         self.config_path = self.createTestPath("local/resopt/config/simple")
         self.config_file = "config"
         self.port = 9125


    def sendRecv(self , send , expect):
        recv = TestClient.sendRecv( self.port , json.dumps(send) + "\n")
        self.assertEqual( json.loads(recv) , expect )


    def sendRecvRAW(self , send , expect):
        recv = TestClient.sendRecv( self.port , send)
        self.assertEqual( json.loads(recv) , expect )
        

         
    def test_connect(self):
        with TestAreaContext("server/socket") as work_area:
            work_area.copy_directory_content(self.config_path)
            pid = os.fork()
            if pid == 0:
                s = ErtSocket(self.config_file , self.port)
                s.listen( )
            else:
                time.sleep(0.50)
                
                self.sendRecv( ["ECHO" ,  "HEI"] , ["HEI"] )
                self.sendRecv( ["STATUS"] , ["OPEN"] )
                self.sendRecvRAW( "INVALID\n" , {"input" : "INVALID" , "ERROR" : "No JSON object could be decoded"})
                self.sendRecv( ["QUIT"] , ["QUIT"] )
            
                
