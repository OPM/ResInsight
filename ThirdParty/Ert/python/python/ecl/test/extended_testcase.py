import numbers
import os
import os.path
import traceback
import sys

try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from .source_enumerator import SourceEnumerator
from ecl.util import installAbortSignals
from ecl.util import Version

TESTDATA_ROOT = None
SHARE_ROOT = None
SOURCE_ROOT = None
BUILD_ROOT = None
try:
    from test_env import *
    assert( os.path.isdir( TESTDATA_ROOT ))
    assert( os.path.isdir( SOURCE_ROOT ))
    assert( os.path.isdir( BUILD_ROOT ))
    if not SHARE_ROOT is None:
        assert( os.path.isdir( SHARE_ROOT ))
except ImportError:
    sys.stderr.write("Warning: could not import file test_env.py - this might lead to test failures.")


class _AssertNotRaisesContext(object):

    def __init__(self, test_class):
        super(_AssertNotRaisesContext, self).__init__()
        self._test_class = test_class

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, tb):
        if exc_type is not None:
            try:
                exc_name = exc_type.__name__
            except AttributeError:
                exc_name = str(exc_type)
            self._test_class.fail("Exception: %s raised\n%s" % (exc_name, traceback.print_exception(exc_type, exc_value, tb)))
        return True


"""
This class provides some extra functionality for testing values that are almost equal.
"""
class ExtendedTestCase(TestCase):
    def __init__(self , *args , **kwargs):
        self.__testdata_root = None
        self.__share_root = None
        installAbortSignals()
        super(ExtendedTestCase , self).__init__(*args , **kwargs)


    def __str__(self):
        return 'ExtendedTestCase( TESTADATA_ROOT=%s, SOURCE_ROOT=%s, SHARE_ROOT=%s, BUILD_ROOT=%s)' % (TESTDATA_ROOT,
                                                                                                       SOURCE_ROOT,
                                                                                                       SHARE_ROOT,
                                                                                                       BUILD_ROOT)

    def assertFloatEqual(self, first, second, msg=None, tolerance=1e-6):
        try:
            f_first, f_second = float(first), float(second)
            diff = abs(f_first - f_second)
            scale = max(1, abs(first) + abs(second))
            if msg is None:
                msg = "Floats not equal: |%f - %f| > %g" % (f_first, f_second, tolerance)
            self.assertTrue(diff < tolerance * scale, msg=msg)
        except TypeError:
            self.fail("Cannot compare as floats: %s (%s) and %s (%s)" %
                      (first, type(first), second, type(second)))


    def assertAlmostEqualList(self, first, second, msg=None, tolerance=1e-6):
        if len(first) != len(second):
            self.fail("Lists are not of same length!")

        for index in range(len(first)):
            self.assertFloatEqual(
                    first[index], second[index],
                    msg=msg, tolerance=tolerance
                    )


    def assertImportable(self, module_name):
        try:
            __import__(module_name)
        except ImportError:
            tb = traceback.format_exc()
            self.fail("Module %s not found!\n\nTrace:\n%s" % (module_name, str(tb)))
        except Exception:
            tb = traceback.format_exc()
            self.fail("Import of module %s caused errors!\n\nTrace:\n%s" % (module_name, str(tb)))


    def assertFilesAreEqual(self, first, second):
        if not self.__filesAreEqual(first, second):
            self.fail("Buffer contents of files are not identical!")


    def assertFilesAreNotEqual(self, first, second):
        if self.__filesAreEqual(first, second):
            self.fail("Buffer contents of files are identical!")

    def assertFileExists(self, path):
        if not os.path.exists(path) or not os.path.isfile(path):
            self.fail("The file: %s does not exist!" % path)

    def assertDirectoryExists(self, path):
        if not os.path.exists(path) or not os.path.isdir(path):
            self.fail("The directory: %s does not exist!" % path)

    def assertFileDoesNotExist(self, path):
        if os.path.exists(path) and os.path.isfile(path):
            self.fail("The file: %s exists!" % path)

    def assertDirectoryDoesNotExist(self, path):
        if os.path.exists(path) and os.path.isdir(path):
            self.fail("The directory: %s exists!" % path)

    def __filesAreEqual(self, first, second):
        buffer1 = open(first).read()
        buffer2 = open(second).read()

        return buffer1 == buffer2

    def assertEnumIsFullyDefined(self, enum_class, enum_name, source_path, verbose=False):
        enum_values = SourceEnumerator.findEnumerators(enum_name, os.path.join( SOURCE_ROOT , source_path))

        for identifier, value in enum_values:
            if verbose:
                print("%s = %d" % (identifier, value))

            self.assertTrue(enum_class.__dict__.has_key(identifier), "Enum does not have identifier: %s" % identifier)
            class_value = enum_class.__dict__[identifier]
            self.assertEqual(class_value, value, "Enum value for identifier: %s does not match: %s != %s" % (identifier, class_value, value))


    @staticmethod
    def createSharePath(path):
        return os.path.realpath(os.path.join(SHARE_ROOT , path))


    @staticmethod
    def createTestPath(path):
        return os.path.realpath(os.path.join(TESTDATA_ROOT , path))


    def assertNotRaises(self, func=None):

        context = _AssertNotRaisesContext(self)
        if func is None:
            return context

        with context:
            func()

    @staticmethod
    def slowTestShouldNotRun():
        """
        @param: The slow test flag can be set by environment variable SKIP_SLOW_TESTS = [True|False]
        """

        return os.environ.get("SKIP_SLOW_TESTS", "False") == "True"


    @staticmethod
    def requireVersion(major , minor , micro = "git"):
        required_version = Version(major, minor , micro)
        current_version = Version.currentVersion()

        if required_version < current_version:
            return True
        else:
            return False
