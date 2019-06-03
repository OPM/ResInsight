import grpc
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

from Empty_pb2 import Empty

import AppInfo_pb2
import AppInfo_pb2_grpc

class AppInfo:
    def __init__(self, channel):
        self.appInfo      = AppInfo_pb2_grpc.AppInfoStub(channel)
    def versionMessage(self):
        return self.appInfo.GetVersion(Empty())
    def majorVersion(self):
        return self.versionMessage().major_version
    def minorVersion(self):
        return self.versionMessage().minor_version
    def patchVersion(self):
        return self.versionMessage().patch_version
    def versionString(self):
        return str(self.majorVersion()) + "." + str(self.minorVersion()) + "." + str(self.patchVersion())

