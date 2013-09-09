#  Copyright (C) 2012  Statoil ASA, Norway.
#
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
import numbers
import os
import traceback
from unittest2 import TestCase


"""
    This class provides some extra functionality for testing values that are almost equal.
    """


class ExtendedTestCase(TestCase):
    def assertAlmostEqualScaled(self, first, second, msg=None):
        if isinstance(first, numbers.Number) and isinstance(second, numbers.Number):
            tolerance = 1e-6
            diff = abs(first - second)
            scale = max(1, abs(first) + abs(second))

            self.assertTrue(diff < tolerance * scale, msg=msg)
        else:
            self.assertTrue(first == second, msg=msg)


    def assertAlmostEqualList(self, first, second, msg=None):
        if len(first) != len(second):
            self.fail("Lists are not of same length!")

        for index in range(len(first)):
            self.assertAlmostEqualScaled(first[index], second[index], msg=msg)

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

    def __filesAreEqual(self, first, second):
        buffer1 = open(first).read()
        buffer2 = open(second).read()

        return buffer1 == buffer2

    def createTestPath(self, path):
        """
        @param: The test root path can be set by environment variable ERT_TEST_ROOT_PATH
        """
        file_path = os.path.realpath(__file__)
        default_test_root = os.path.realpath(os.path.join(os.path.dirname(file_path), "../test-data/"))
        test_root = os.path.realpath(os.environ.get("ERT_TEST_ROOT_PATH", default_test_root))

        return os.path.realpath(os.path.join(test_root, path))

    @staticmethod
    def slowTestShouldNotRun():
        """
        @param: The slow test flag can be set by environment variable SKIP_SLOW_TESTS = [True|False]
        """

        return os.environ.get("SKIP_SLOW_TESTS", "False") == "True"