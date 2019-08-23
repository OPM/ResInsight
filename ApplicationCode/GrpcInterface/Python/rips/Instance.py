import grpc
import os
import sys
import socket
import logging
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))
import App_pb2
import App_pb2_grpc
from Definitions_pb2 import Empty

import RiaVersionInfo

from .Commands import Commands
from .Project import Project

class Instance:
    """The ResInsight Instance class. Use to launch or find existing ResInsight instances

    Attributes:
        launched (bool): Tells us whether the application was launched as a new process.
            If the application was launched we may need to close it when exiting the script.
        commands (Commands): Command executor. Set when creating an instance.
        project (Project): Current project in ResInsight.
            Set when creating an instance and updated when opening/closing projects.
    """

    @staticmethod
    def __is_port_in_use(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.2)
            return s.connect_ex(('localhost', port)) == 0
    
    @staticmethod
    def launch(resInsightExecutable = '', console = False, launchPort = -1, commandLineParameters=[]):
        """ Launch a new Instance of ResInsight. This requires the environment variable
        RESINSIGHT_EXECUTABLE to be set or the parameter resInsightExecutable to be provided.
        The RESINSIGHT_GRPC_PORT environment variable can be set to an alternative port number.

        Args:
            resInsightExecutable (str): Path to a valid ResInsight executable. If set
                will take precedence over what is provided in the RESINSIGHT_EXECUTABLE
                environment variable.
            console (bool): If True, launch as console application, without GUI.
            launchPort(int): If -1 will use the default port of 50051 or look for RESINSIGHT_GRPC_PORT
                             if anything else, ResInsight will try to launch with this port
            commandLineParameters(list): Additional command line parameters as string entries in the list.
        Returns:
            Instance: an instance object if it worked. None if not.
        """

        port = 50051
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            port = int(portEnv)
        if launchPort is not -1:
            port = launchPort
        
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

        if isinstance(commandLineParameters, str):
            commandLineParameters = [str]

        parameters = ["ResInsight", "--server", str(port)] + commandLineParameters
        if console:
            print("Launching as console app")
            parameters.append("--console")

        # Stringify all parameters
        for i in range(0, len(parameters)):
            parameters[i] = str(parameters[i])

        pid = os.spawnv(os.P_NOWAIT, resInsightExecutable, parameters)
        if pid:
            instance = Instance(port=port, launched=True)
            return instance
        return None
    
    @staticmethod
    def find(startPort = 50051, endPort = 50071):
        """ Search for an existing Instance of ResInsight by testing ports.
         
        By default we search from port 50051 to 50071 or if the environment
        variable RESINSIGHT_GRPC_PORT is set we search
        RESINSIGHT_GRPC_PORT to RESINSIGHT_GRPC_PORT+20

        Args:
            startPort (int): start searching from this port
            endPort (int): search up to but not including this port
        """
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            startPort = int(portEnv)		
            endPort   = startPort + 20 
        
        for tryPort in range(startPort, endPort):
            if Instance.__is_port_in_use(tryPort):
                return Instance(port=tryPort)
                
        print('Error: Could not find any ResInsight instances responding between ports ' + str(startPort) + ' and ' + str(endPort))
        return None

    def __checkVersion(self):
        try:
            majorVersionOk = self.majorVersion() == int(RiaVersionInfo.RESINSIGHT_MAJOR_VERSION)
            minorVersionOk = self.minorVersion() == int(RiaVersionInfo.RESINSIGHT_MINOR_VERSION)
            return True, majorVersionOk and minorVersionOk
        except grpc.RpcError as e:
            return False, False

    def __init__(self, port = 50051, launched = False):
        """ Attempts to connect to ResInsight at aa specific port on localhost

        Args:
            port(int): port number
        """
        logging.basicConfig()
        location = "localhost:" + str(port)

        self.channel = grpc.insecure_channel(location, options=[('grpc.enable_http_proxy', False)])
        self.launched = launched

        # Main version check package
        self.app     = self.app = App_pb2_grpc.AppStub(self.channel)

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

    def __versionMessage(self):
        return self.app.GetVersion(Empty())

    def majorVersion(self):
        """Get an integer with the major version number"""
        return self.__versionMessage().major_version

    def minorVersion(self):
        """Get an integer with the minor version number"""
        return self.__versionMessage().minor_version

    def patchVersion(self):
        """Get an integer with the patch version number"""
        return self.__versionMessage().patch_version

    def versionString(self):
        """Get a full version string, i.e. 2019.04.01"""
        return str(self.majorVersion()) + "." + str(self.minorVersion()) + "." + str(self.patchVersion())

    def exit(self):
        """Tell ResInsight instance to quit"""
        print("Telling ResInsight to Exit")
        return self.app.Exit(Empty())

    def isConsole(self):
        """Returns true if the connected ResInsight instance is a console app"""
        return self.app.GetRuntimeInfo(Empty()).app_type == App_pb2.ApplicationTypeEnum.Value('CONSOLE_APPLICATION')

    def isGui(self):
        """Returns true if the connected ResInsight instance is a GUI app"""
        return self.app.GetRuntimeInfo(Empty()).app_type == App_pb2.ApplicationTypeEnum.Value('GUI_APPLICATION')
