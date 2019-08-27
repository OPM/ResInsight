import os.path
import types
from ecl.util.test import ExtendedTestCase
from functools import wraps
from functools import partial
import unittest
from unittest import SkipTest

def source_root():
    src = '@CMAKE_CURRENT_SOURCE_DIR@/../..'
    if os.path.isdir(src):
        return os.path.realpath(src)
    
    # If the file was not correctly configured by cmake, look for the source
    # folder, assuming the build folder is inside the source folder.
    path_list = os.path.dirname(os.path.abspath(__file__)).split("/")
    while len(path_list) > 0:
        git_path = os.path.join(os.sep, "/".join(path_list), ".git")
        if os.path.isdir(git_path):
            return os.path.join(os.sep, *path_list)
        path_list.pop()
    raise RuntimeError('Cannot find the source folder')


# Decorator which is used to mark either an entire test class or individual
# test methods as requiring Equinor testdata. If Equinor testdata has not been
# configured as part of the build process these tests will be skipped.
#
# Ideally the equinor_test() implementation should just be a suitable wrapper of: 
#
#       skipUnless(EclTest.EQUINOR_DATA, "Missing Equinor testdata")
#
# but that has been surprisingly difficult to achieve. The current
# implemenation is based on the skip() function from the unittest/case.py
# module in the Python standard distribution. There are unfortunately several
# problems with this:
#
#  1. It is based on accessing the __unittest_skip attribute of the TestCase
#     class, that is certainly a private detail which we should not access.
#
#  2. The decorator must be invoked with an empty parenthesis when decorating a
#     class, that is not required when decorating method:
#
#
#     @equinor_test()
#     class EquinorTest(EclTest):
#     # This test class will be skipped entirely if we do not have access to
#     # Equinor testdata.
#
#
#     class XTest(EclTest):
#
#         @equinor_test
#         def test_method(self):

def equinor_test():
    """
    Will mark a test method or an entire test class as dependent on Equinor testdata.
    """
    def decorator(test_item):
        if not isinstance(test_item, type):
            if not EclTest.EQUINOR_DATA:
                @functools.wraps(test_item)
                def skip_wrapper(*args, **kwargs):
                    raise SkipTest("Missing Equinor testdata")
                test_item = skip_wrapper

        if not EclTest.EQUINOR_DATA:
            test_item.__unittest_skip__ = True
            test_item.__unittest_skip_why__ = "Missing Equinor testdata"
        return test_item
    return decorator




class EclTest(ExtendedTestCase):
    SOURCE_ROOT = source_root()
    TESTDATA_ROOT = os.path.join(SOURCE_ROOT, "test-data")
    EQUINOR_DATA = os.path.islink(os.path.join(TESTDATA_ROOT, "Equinor")) 


