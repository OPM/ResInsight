import grpc
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from Definitions_pb2 import Empty

import App_pb2
import App_pb2_grpc

class App:
    """ResInsight application information and control.
    Allows retrieving of information and controlling the running instance
    Not meant to be constructed manually, but exists as part of the Instance method
    """
    def __init__(self, channel):
        self.app = App_pb2_grpc.AppStub(channel)
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
