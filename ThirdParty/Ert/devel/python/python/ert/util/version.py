from ert.cwrap import CNamespace, CWrapper
from ert.util import UTIL_LIB


def cmp_method(method):

    def cmp_wrapper(self , other):
        if not isinstance(other , Version):
            other = Version(other[0] , other[1] , other[2])

        return method(self , other)

    return cmp_wrapper


class Version(object):
    __namespace = CNamespace("Version")

    def __init__(self , major , minor , micro):
        self.major = major
        self.minor = minor
        self.micro = micro
        try:
            self.micro_int = int(micro)
            self.is_devel = False
        except ValueError:
            self.micro_int = -1
            self.is_devel = True


    def isDevelVersion(self):
        return self.is_devel


    def versionString(self):
        return "%d.%d.%s" % (self.major , self.minor , self.micro)


    def versionTuple(self):
        return (self.major , self.minor , self.micro)

    def __cmpTuple(self):
        return (self.major , self.minor , self.micro_int)

    def __str__(self):
        return self.versionString()

    @cmp_method
    def __eq__(self , other):
        return self.versionTuple() == other.versionTuple()

    def __ne__(self , other):
        return not (self == other)

    # All development versions are compared with micro version == -1;
    # i.e.  the two versions Version(1,2,"Alpha") and
    # Version(1,2,"Beta") compare as equal in the >= and <= tests -
    # but not in the == test.

    @cmp_method
    def __ge__(self , other):
        return self.__cmpTuple() >= other.__cmpTuple()
                    
    @cmp_method
    def __lt__(self , other):
        return not (self >= other)

    @cmp_method
    def __le__(self , other):
        return self.__cmpTuple() <= other.__cmpTuple()
                    
    @cmp_method
    def __gt__(self , other):
        return not (self <= other)


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
    def currentVersion(cls):
        major = Version.cNamespace().major_version()
        minor = Version.cNamespace().minor_version()
        micro = Version.cNamespace().micro_version()
        return Version( major , minor , micro )


    @classmethod
    def getVersion(cls):
        v = cls.currentVersion( )
        return v.versionString( )
        

    @classmethod
    def cNamespace(cls):
        return Version.__namespace


cwrapper = CWrapper(UTIL_LIB)

Version.cNamespace().build_time  = cwrapper.prototype("char* version_get_build_time()")
Version.cNamespace().git_commit  = cwrapper.prototype("char* version_get_git_commit()")
Version.cNamespace().git_commit_short  = cwrapper.prototype("char* version_get_git_commit_short()")
Version.cNamespace().major_version = cwrapper.prototype("int version_get_major_ert_version()")
Version.cNamespace().minor_version = cwrapper.prototype("int version_get_minor_ert_version()")
Version.cNamespace().micro_version = cwrapper.prototype("char* version_get_micro_ert_version()")
Version.cNamespace().is_devel = cwrapper.prototype("bool version_is_ert_devel_version()")
