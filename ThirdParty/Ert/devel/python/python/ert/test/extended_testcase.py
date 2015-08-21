import numbers
import os
import os.path
import traceback

try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from .source_enumerator import SourceEnumerator
from ert.util import installAbortSignals
from ert.util import Version




"""
This class provides some extra functionality for testing values that are almost equal.
"""
class ExtendedTestCase(TestCase):
    def __init__(self , *args , **kwargs):
        self.__testdata_root = None
        self.__share_root = None
        installAbortSignals()
        super(ExtendedTestCase , self).__init__(*args , **kwargs)


    def assertFloatEqual(self, first, second, msg=None):

        if msg is None:
            msg = "Value %f and %f are not almost equal!" % (first, second)

        if isinstance(first, numbers.Number) and isinstance(second, numbers.Number):
            tolerance = 1e-6
            diff = abs(first - second)
            scale = max(1, abs(first) + abs(second))

            self.assertTrue(diff < tolerance * scale, msg=msg)
        else:
            self.fail("Elements not comparable as float: %s and %s" % (first, second))


    def assertAlmostEqualList(self, first, second, msg=None):
        if len(first) != len(second):
            self.fail("Lists are not of same length!")

        for index in range(len(first)):
            self.assertFloatEqual(first[index], second[index], msg=msg)


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
        enum_values = SourceEnumerator.findEnumerators(enum_name, source_path)

        for identifier, value in enum_values:
            if verbose:
                print("%s = %d" % (identifier, value))

            self.assertTrue(enum_class.__dict__.has_key(identifier), "Enum does not have identifier: %s" % identifier)
            class_value = enum_class.__dict__[identifier]
            self.assertEqual(class_value, value, "Enum value for identifier: %s does not match: %s != %s" % (identifier, class_value, value))


    def setTestDataRoot(self, testdata_root):
        self.__testdata_root = testdata_root
        if not os.path.exists(self.__testdata_root):
            raise IOError("Path:%s not found" % self.__testdata_root)

    def setShareRoot(self, share_root):
        self.__share_root = share_root
        if not os.path.exists(self.__share_root):
            raise IOError("Path: %s not found" % self.__share_root)


    def createTestPath(self, path, testdata_root=None):
        if testdata_root is None and self.__testdata_root is None:
            file_path = os.path.realpath(__file__)
            build_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../devel/test-data/"))
            site_packages_build_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../../../devel/test-data/"))
            src_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../test-data/"))
            env_root = os.getenv("ERT_TEST_ROOT_PATH")

            if env_root is not None and os.path.exists(env_root):
                root = os.path.realpath(env_root)
            elif os.path.exists(build_root):
                root = os.path.realpath(build_root)
            elif os.path.exists(site_packages_build_root):
                root = os.path.realpath(site_packages_build_root)
            elif os.path.exists(src_root):
                root = os.path.realpath(src_root)
            else:
                root = None

            self.setTestDataRoot(root)

        root_path = self.__testdata_root 
        if testdata_root is not None:
            if not os.path.exists(testdata_root):
                raise IOError("Path: %s not found" % testdata_root)

            root_path = testdata_root

        return os.path.realpath(os.path.join(root_path , path))


    def createSharePath(self, path, share_root=None):
        if share_root is None and self.__share_root is None:
            file_path = os.path.realpath(__file__)
            build_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../devel/share/"))
            site_packages_build_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../../../devel/share/"))
            src_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../../../../share/"))
            env_root = os.getenv("ERT_TEST_ROOT_PATH")

            if env_root is not None and os.path.exists(env_root):
                root = os.path.realpath(env_root)
            elif os.path.exists(build_root):
                root = os.path.realpath(build_root)
            elif os.path.exists(site_packages_build_root):
                root = os.path.realpath(site_packages_build_root)
            elif os.path.exists(src_root):
                root = os.path.realpath(src_root)
            else:
                root = None

            self.setShareRoot(root)

        root_path = self.__share_root
        if share_root is not None:
            if not os.path.exists(share_root):
                raise IOError("Path: %s not found" % share_root)

            root_path = share_root

        return os.path.realpath(os.path.join(root_path , path))

    
    def assertNotRaises(self, func):
        try:
            func()
        except:
            self.fail()


            
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
    
