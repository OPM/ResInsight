import grpc
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

from Empty_pb2 import Empty

import App_pb2
import App_pb2_grpc

class App:
    def __init__(self, channel):
        self.app = App_pb2_grpc.AppStub(channel)
    def versionMessage(self):
        return self.app.GetVersion(Empty())
    def majorVersion(self):
        return self.versionMessage().major_version
    def minorVersion(self):
        return self.versionMessage().minor_version
    def patchVersion(self):
        return self.versionMessage().patch_version
    def versionString(self):
        return str(self.majorVersion()) + "." + str(self.minorVersion()) + "." + str(self.patchVersion())
    def exit(self):
        print("Telling ResInsight to Exit")
        return self.app.Exit(Empty())
    def isConsole(self):
        print("RunTimeInfo stuff: ", self.app.GetRuntimeInfo(Empty()).app_type)
        return self.app.GetRuntimeInfo(Empty()).app_type == App_pb2.ApplicationTypeEnum.Value('CONSOLE_APPLICATION')
    def isGui(self):
        print("RunTimeInfo stuff: ", self.app.GetRuntimeInfo(Empty()).app_type)
        return self.app.GetRuntimeInfo(Empty()).app_type == App_pb2.ApplicationTypeEnum.Value('GUI_APPLICATION')
