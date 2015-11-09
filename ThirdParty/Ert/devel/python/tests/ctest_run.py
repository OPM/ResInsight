#!/usr/bin/env python
import os
import sys

try:
    from unittest2 import TextTestRunner
except ImportError:
    from unittest import TextTestRunner


def runTestCase(tests, verbosity=0):
    test_result = TextTestRunner(verbosity=verbosity).run(tests)

    if len(test_result.errors) or len(test_result.failures):
        test_result.printErrors()
        return False
    else:
        return True


if __name__ == '__main__':
    TEST_PYTHONPATH = sys.argv[1]
    os.environ["PYTHONPATH"] = TEST_PYTHONPATH + os.pathsep + os.getenv("PYTHONPATH", "")
    for path_element in reversed(TEST_PYTHONPATH.split(os.pathsep)):
        sys.path.insert(0, path_element)

    test_class_path = sys.argv[2]
    argv = []

    try:
        argv = sys.argv[3:]
    except IndexError:
        pass

    from ert.test import ErtTestRunner

    tests = ErtTestRunner.getTestsFromTestClass(test_class_path, argv)

    # Set verbosity to 2 to see which test method in a class that fails.
    if runTestCase(tests, verbosity=0):
        sys.exit(0)
    else:
        sys.exit(1)
