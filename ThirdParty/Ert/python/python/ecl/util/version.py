from ecl.util import UtilPrototype


def cmp_method(method):
    def cmp_wrapper(self, other):
        if not isinstance(other, Version):
            other = Version(other[0], other[1], other[2])

        return method(self, other)

    return cmp_wrapper


class Version(object):
    _build_time = UtilPrototype("char* version_get_build_time()")
    _git_commit = UtilPrototype("char* version_get_git_commit()")
    _git_commit_short = UtilPrototype("char* version_get_git_commit_short()")
    _major_version = UtilPrototype("int version_get_major_ert_version()")
    _minor_version = UtilPrototype("int version_get_minor_ert_version()")
    _micro_version = UtilPrototype("char* version_get_micro_ert_version()")
    _is_devel = UtilPrototype("bool version_is_ert_devel_version()")

    def __init__(self, major, minor, micro):
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
        return "%d.%d.%s" % (self.major, self.minor, self.micro)

    def versionTuple(self):
        return self.major, self.minor, self.micro

    def __cmpTuple(self):
        return self.major, self.minor, self.micro_int

    def __str__(self):
        return self.versionString()

    def __repr__(self):
        major, minor, micro = self.versionTuple()
        commit = self._git_commit_short()
        status = 'production'
        if self.isDevelVersion():
            status = 'development'
        fmt = 'Version(major=%d, minor=%d, micro="%s", commit="%s", status="%s")'
        return fmt % (major, minor, micro, commit, status)

    @cmp_method
    def __eq__(self, other):
        return self.versionTuple() == other.versionTuple()

    def __ne__(self, other):
        return not (self == other)

    def __hash__(self):
        return hash(self.versionTuple())

    # All development versions are compared with micro version == -1;
    # i.e.  the two versions Version(1,2,"Alpha") and
    # Version(1,2,"Beta") compare as equal in the >= and <= tests -
    # but not in the == test.

    @cmp_method
    def __ge__(self, other):
        return self.__cmpTuple() >= other.__cmpTuple()

    @cmp_method
    def __lt__(self, other):
        return not (self >= other)

    @cmp_method
    def __le__(self, other):
        return self.__cmpTuple() <= other.__cmpTuple()

    @cmp_method
    def __gt__(self, other):
        return not (self <= other)

    @classmethod
    def getBuildTime(cls):
        return cls._build_time()

    @classmethod
    def getGitCommit(cls, short=False):
        if not short:
            return cls._git_commit()
        else:
            return cls._git_commit_short()

    @classmethod
    def currentVersion(cls):
        major = cls._major_version()
        minor = cls._minor_version()
        micro = cls._micro_version()
        return Version(major, minor, micro)

    @classmethod
    def getVersion(cls):
        v = cls.currentVersion()
        return v.versionString()
