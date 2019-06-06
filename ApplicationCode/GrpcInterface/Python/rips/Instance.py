import grpc
import os
import sys
import socket
import logging
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../generated'))

import RiaVersionInfo

from .App import App
from .Commands import Commands
from .Project import Project

class Instance:
    launched = False

    @staticmethod
    def is_port_in_use(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.2)
            return s.connect_ex(('localhost', port)) == 0
    
    @staticmethod
    def launch(resInsightExecutable = '', console = False):        
        port = 50051
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            port = int(portEnv)
        
        if not resInsightExecutable:
            resInsightExecutable = os.environ.get('RESINSIGHT_EXECUTABLE')
            if not resInsightExecutable:
                print('ERROR: Could not launch ResInsight because the environment variable'
                      ' RESINSIGHT_EXECUTABLE is not set')
                return None
        
        while Instance.is_port_in_use(port):
            port += 1

        print('Port ' + str(port))
        print('Trying to launch', resInsightExecutable)
        parameters = ["ResInsight", "--grpcserver", str(port)]
        if console:
            print("Launching as console app")
            parameters.append("--console")
        pid = os.spawnv(os.P_NOWAIT, resInsightExecutable, parameters)
        if pid:
            return Instance(port=port, launched=True)
        return None
    
    @staticmethod
    def find(startPort = 50051, endPort = 50071):
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            startPort = int(portEnv)		
            endPort   = startPort + 20 
        
        for tryPort in range(startPort, endPort):
            if Instance.is_port_in_use(tryPort):
                return Instance(tryPort)
                
        print('Error: Could not find any ResInsight instances responding between ports ' + str(startPort) + ' and ' + str(endPort))
        return None

    def checkVersion(self):
        try:
            majorVersionOk = self.app.majorVersion() == int(RiaVersionInfo.RESINSIGHT_MAJOR_VERSION)
            minorVersionOk = self.app.minorVersion() == int(RiaVersionInfo.RESINSIGHT_MINOR_VERSION)
            return True, majorVersionOk and minorVersionOk
        except grpc.RpcError as e:
            return False, False

    def __init__(self, port = 50051, launched = False):
        logging.basicConfig()
        location = "localhost:" + str(port)

        self.channel = grpc.insecure_channel(location)
        self.launched = launched

        # Main version check package
        self.app     = App(self.channel)

        connectionOk = False
        versionOk = False

        if launched:
            for i in range(0, 10):
                connectionOk, versionOk = self.checkVersion()
                if connectionOk:
                    break
                time.sleep(1.0)
        else:
            connectionOk, versionOk = self.checkVersion()

        if not connectionOk:
            if launched:
                raise Exception('Error: Could not connect to resinsight at ', location, ' after trying 10 times with 1 second apart')
            else:
                raise Exception('Error: Could not connect to resinsight at ', location)
            exit(1)
        if not versionOk:
            raise Exception('Error: Wrong Version of ResInsight at ', location)

        # Service packages
        self.commands   = Commands(self.channel)
        self.project    = Project(self.channel)
    
        path = os.getcwd()
        self.commands.setStartDir(path=path)
