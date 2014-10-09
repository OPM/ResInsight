from ert.cwrap import CNamespace, CWrapper
from ert.util import UTIL_LIB


class Version(object):
    __namespace = CNamespace("Version")

    @classmethod
    def getBuildTime(cls):
        return Version.cNamespace().build_time()

    @classmethod
    def getGitCommit(cls, short=False):
        if not short:
            return Version.cNamespace().git_commit()
        else:
            return Version.cNamespace().git_commit_short()


    @classmethod
    def getVersion(cls):
        return Version.cNamespace().ert_version()

    @classmethod
    def cNamespace(cls):
        return Version.__namespace


cwrapper = CWrapper(UTIL_LIB)

Version.cNamespace().build_time  = cwrapper.prototype("char* version_get_build_time()")
Version.cNamespace().git_commit  = cwrapper.prototype("char* version_get_git_commit()")
Version.cNamespace().git_commit_short  = cwrapper.prototype("char* version_get_git_commit_short()")
Version.cNamespace().ert_version = cwrapper.prototype("char* version_get_ert_version()")