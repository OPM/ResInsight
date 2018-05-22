from ecl import EclPrototype


def cmp_method(method):
    def cmp_wrapper(self, other):
        if not isinstance(other, Version):
            other = Version(other[0], other[1], other[2])

        return method(self, other)

    return cmp_wrapper



class Version(object):


    def __init__(self, major, minor, micro, git_commit = None, build_time = None):
        self.major = major
        self.minor = minor
        self.micro = micro
        try:
            self.micro_int = int(micro)
            self.is_devel = False
        except ValueError:
            self.micro_int = -1
            self.is_devel = True
        self.build_time = build_time
        self.git_commit = git_commit

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
        status = 'production'
        git_commit = self.getGitCommit( short = True )
        if self.is_devel:
            status = 'development'
        fmt = 'Version(major=%d, minor=%d, micro="%s", commit="%s", status="%s")'
        return fmt % (self.major, self.minor, self.micro, git_commit, status)

    @cmp_method
    def __eq__(self, other):
        return self.versionTuple() == other.versionTuple()

    def __ne__(self, other):
        return not (self == other)

    def __hash__(self):
        return hash(self.versionTuple())

    # All development versions are compared with micro version == -1;
    # i.e.  the two versions version(1,2,"Alpha") and
    # ecl_version(1,2,"Beta") compare as equal in the >= and <= tests -
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

    def getBuildTime(self):
        if self.build_time is None:
            return "?????"
        else:
            return self.build_time

    def getGitCommit(self, short=False):
        if self.git_commit is None:
            return "???????"
        else:
            if short:
                return self.git_commit[0:8]
            else:
                return self.git_commit


class EclVersion(Version):
    _build_time = EclPrototype("char* ecl_version_get_build_time()", bind = False)
    _git_commit = EclPrototype("char* ecl_version_get_git_commit()", bind = False)
    _major_version = EclPrototype("int ecl_version_get_major_version()", bind = False)
    _minor_version = EclPrototype("int ecl_version_get_minor_version()", bind = False)
    _micro_version = EclPrototype("char* ecl_version_get_micro_version()", bind = False)
    _is_devel = EclPrototype("bool ecl_version_is_devel_version()", bind = False)

    def __init__(self):
        major = self._major_version( )
        minor = self._minor_version( )
        micro = self._micro_version( )
        git_commit = self._git_commit( )
        build_time = self._build_time( )
        super( EclVersion, self).__init__( major, minor , micro , git_commit, build_time)



