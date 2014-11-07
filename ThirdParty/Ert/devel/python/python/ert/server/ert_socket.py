#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'ert_socket.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import SocketServer
import time
import sys
import threading
import json
import traceback

from ert.server import ErtServer

class ErtHandler(SocketServer.StreamRequestHandler):
    ert_server = None
    config_file = None
    
    
    def handle(self):
        data = self.rfile.readline().strip()
        try:
            json_data = json.loads( data )
            if json_data[0] == "QUIT":
                self.handleQuit()
            else:
                self.evalJson( json_data )
        except Exception,e:
            self.handleInvalidJSON(data , "%s %s" % (e , traceback.format_exc()))
            


    def evalJson(self , json_data):
        cmd = json_data[0]
        if cmd == "ECHO":
            self.wfile.write( json.dumps( json_data[1:] ))
        else:
            result = self.ert_server.evalCmd( json_data )
            self.wfile.write( json.dumps( result ))
            

    
    def handleInvalidJSON(self , data , e):
        json_string = json.dumps({"ERROR" : "%s" % e , "input" : "%s" % data})
        self.wfile.write( json_string )


        
    def handleQuit(self):
        print "Handling QUIT - shutting down ert server"
        self.wfile.write(json.dumps(["QUIT"]))
        shutdown_thread = threading.Thread( target = self.server.shutdown )
        shutdown_thread.start()




class ErtSocket(object):

    def __init__(self , config_file , port , host = "localhost"):
        self.open(config_file)
        self.server = SocketServer.TCPServer((host , port) , ErtHandler)

    def open(self , config_file):
        try:
            ert_server = ErtServer( config_file )
        except Exception:
            ert_server = ErtServer( )
        ErtHandler.ert_server = ert_server


    def evalCmd(self , cmd):
        return ErtHandler.ert_server.evalCmd( cmd )


    def listen(self):
        self.server.serve_forever( )
        
