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
import socket

from ert.server import ErtServer, SUCCESS , ERROR

class ErtHandler(SocketServer.StreamRequestHandler):
    ert_server = None
    config_file = None
    logger = None
    
    def handle(self):
        string_data = self.rfile.readline().strip()
        try:
            data = json.loads( string_data )
        except Exception,e:
            result = ERROR( "Invalid JSON input" , exception = e)
            self.returnToClient( result )
            return
            
        if data[0] == "QUIT":
            self.handleQuit()
        else:
            self.evalCmd( data )


    def returnToClient(self , data):
        self.wfile.write(json.dumps(data))
        

    def evalCmd(self , data):
        try:
            result = self.ert_server.evalCmd( data )
        except Exception,e:
            result = ERROR( "Exception raised" , exception = e)

        self.returnToClient( result )



    def handleQuit(self):
        shutdown_thread = threading.Thread( target = self.server.shutdown )
        shutdown_thread.start()
        self.returnToClient( SUCCESS(["QUIT"]) )



class ErtSocketServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass
    

class ErtSocket(object):

    def __init__(self , config_file , port , host , logger):
        self.server = ErtSocketServer((host , port) , ErtHandler)
        self.open(config_file , logger)


    @staticmethod
    def connect(config_file , port , host , logger , info_callback = None , timeout = 60 , sleep_time = 5):
        start_time = time.time()
        ert_socket = None
        while True:
            try:
                ert_socket = ErtSocket(config_file , port, host , logger)
                break
            except socket.error:
                if info_callback:
                    info_callback( sleep_time , host , port )
                    
                if time.time() - start_time > timeout:
                    break
                    
            time.sleep( sleep_time )
        return ert_socket



    def open(self , config_file , logger):
        ErtHandler.ert_server = ErtServer( config_file , logger )



    def evalCmd(self , cmd):
        return ErtHandler.ert_server.evalCmd( cmd )


    def listen(self):
        self.server.serve_forever( )
        
        
