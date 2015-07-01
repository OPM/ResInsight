#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'ert_server.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import socket
import json
import datetime
from ert.server import ErtServer


class ErtClient(object):
    def __init__(self , port , host):
        self.socket = socket.socket( socket.AF_INET , socket.SOCK_STREAM)
        self.port = port
        self.host = host
        self.socket.setblocking(1)
        try:
            self.socket.connect((self.host , self.port))
        except socket.error:
            sys.exit("Failed to connect to port:%d at %s." % (port , host))

    @staticmethod
    def convert_to_datetime(data):
        result = []
        for d in data:
            try:
                result.append(datetime.datetime.strptime(d, ErtServer.DATE_FORMAT))
            except:
                pass
        return result

    def sendRecv(self , data):
        self.socket.sendall( json.dumps(data) + "\n" )
        recv = self.socket.recv(4096)
        result = json.loads(recv)

        result0 = result[0]
        if result0 == "OK":
            result = result[1:]
            if data == ["TIME_STEP"]:
                result = ErtClient.convert_to_datetime(result)

            return result
        elif result0 == "ERROR":
            raise Exception("Ert server returned error: %s" % result[1:])
        else:
            raise Exception("Ert server returned result[0] == %s - must have OK|ERROR as first element in return list" % result[0])



    @staticmethod
    def runCommand( cmd , port , host):
        client = ErtClient( port , host )
        return client.sendRecv( cmd )
