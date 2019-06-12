import grpc
import os
import sys
import socket
import logging
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import RiaVersionInfo

from .App import App
from .Commands import Commands
from .Project import Project

class Instance:
    """The ResInsight Instance class. Use to launch or find existing ResInsight instances

    Attributes:
        launched(bool): Tells us whether the application was launched as a new process.
            If the application was launched we may need to close it when exiting the script.
        app(App): Application information object. Set when creating an instance.
        commands(Commands): Command executor. Set when creating an instance.
        project(Project): Current project in ResInsight.
            Set when creating an instance and updated when opening/closing projects.
    """

    @staticmethod
    def __is_port_in_use(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.2)
            return s.connect_ex(('localhost', port)) == 0
    
    @staticmethod
    def launch(resInsightExecutable = '', console = False):
        """ Launch a new Instance of ResInsight. This requires the environment variable
        RESINSIGHT_EXECUTABLE to be set or the parameter resInsightExecutable to be provided.
        The RESINSIGHT_GRPC_PORT environment variable can be set to an alternative port number.

        Args:
            resInsightExecutable (str): Path to a valid ResInsight executable. If set
                will take precedence over what is provided in the RESINSIGHT_EXECUTABLE
                environment variable.
            console (bool): If True, launch as console application, without GUI.
        Returns:
            Instance: an instance object if it worked. None if not.
        """

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
        
        while Instance.__is_port_in_use(port):
            port += 1

        print('Port ' + str(port))
        print('Trying to launch', resInsightExecutable)
        parameters = ["ResInsight", "--grpcserver", str(port)]
        if console:
            print("Launching as console app")
            parameters.append("--console")
        pid = os.spawnv(os.P_NOWAIT, resInsightExecutable, parameters)
        if pid:
            instance = Instance(port=port)
            instance.launched = True
            return instance
        return None
    
    @staticmethod
    def find(startPort = 50051, endPort = 50071):
        """ Search for an existing Instance of ResInsight by testing ports.
         
        By default we search from port 50051 to 50071 or if the environment
        variable RESINSIGHT_GRPC_PORT is set we search
        RESINSIGHT_GRPC_PORT to RESINSIGHT_GRPC_PORT+20

        Args:
            startPort(int): start searching from this port
            endPort(int): search up to but not including this port
        """
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            startPort = int(portEnv)		
            endPort   = startPort + 20 
        
        for tryPort in range(startPort, endPort):
            if Instance.__is_port_in_use(tryPort):
                return Instance(tryPort)
                
        print('Error: Could not find any ResInsight instances responding between ports ' + str(startPort) + ' and ' + str(endPort))
        return None

    def __checkVersion(self):
        try:
            majorVersionOk = self.app.majorVersion() == int(RiaVersionInfo.RESINSIGHT_MAJOR_VERSION)
            minorVersionOk = self.app.minorVersion() == int(RiaVersionInfo.RESINSIGHT_MINOR_VERSION)
            return True, majorVersionOk and minorVersionOk
        except grpc.RpcError as e:
            return False, False

    def __init__(self, port = 50051):
        """ Attempts to connect to ResInsight at aa specific port on localhost

        Args:
            port(int): port number
        """
        logging.basicConfig()
        location = "localhost:" + str(port)

        self.channel = grpc.insecure_channel(location)
        self.launched = False

        # Main version check package
        self.app     = App(self.channel)

        connectionOk = False
        versionOk = False

        if self.launched:
            for i in range(0, 10):
                connectionOk, versionOk = self.__checkVersion()
                if connectionOk:
                    break
                time.sleep(1.0)
        else:
            connectionOk, versionOk = self.__checkVersion()

        if not connectionOk:
            if self.launched:
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
