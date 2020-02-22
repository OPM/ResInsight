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
from ecl.util.util import installAbortSignals
from ecl.util.util import Version


# Function wrapper which can be used to add decorator @log_test to test
# methods. When a test has been decorated with @log_test it will print
# "starting: <test_name>" when a method is complete and "complete: <test_name>"
# when the method is complete. Convenient when debugging tests which fail hard
# or lock up.

def log_test(test):
    def wrapper(*args):
        sys.stderr.write("starting: %s \n" % test.__name__)
        test(*args)
        sys.stderr.write("complete: %s \n" % test.__name__)
    return wrapper



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
    TESTDATA_ROOT = None
    SHARE_ROOT = None
    SOURCE_ROOT = None


    def __init__(self , *args , **kwargs):
        installAbortSignals()
        super(ExtendedTestCase , self).__init__(*args , **kwargs)


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
        buffer1 = open(first, "rb").read()
        buffer2 = open(second, "rb").read()

        return buffer1 == buffer2

    def assertEnumIsFullyDefined(self, enum_class, enum_name, source_path, verbose=False):
        if self.SOURCE_ROOT is None:
            raise Exception("SOURCE_ROOT is not set.")

        enum_values = SourceEnumerator.findEnumerators(enum_name, os.path.join( self.SOURCE_ROOT , source_path))

        for identifier, value in enum_values:
            if verbose:
                print("%s = %d" % (identifier, value))

            self.assertTrue(identifier in enum_class.__dict__, "Enum does not have identifier: %s" % identifier)
            class_value = enum_class.__dict__[identifier]
            self.assertEqual(class_value, value, "Enum value for identifier: %s does not match: %s != %s" % (identifier, class_value, value))


    @classmethod
    def createSharePath(cls, path):
        if cls.SHARE_ROOT is None:
            raise Exception("Trying to create directory rooted in 'SHARE_ROOT' - variable 'SHARE_ROOT' is not set.")
        return os.path.realpath(os.path.join(cls.SHARE_ROOT , path))


    @classmethod
    def createTestPath(cls, path):
        if cls.TESTDATA_ROOT is None:
            raise Exception("Trying to create directory rooted in 'TESTDATA_ROOT' - variable 'TESTDATA_ROOT' has not been set.")
        return os.path.realpath(os.path.join(cls.TESTDATA_ROOT , path))


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
