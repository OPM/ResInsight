#!/usr/bin/env python
import sys

try:
    from unittest2 import TextTestRunner
except ImportError:
    from unittest import TextTestRunner


def runTestCase(tests):
    test_result = TextTestRunner(verbosity=0).run(tests)
    if test_result.errors or test_result.failures:
        for (test, trace_back) in test_result.errors:
            sys.stderr.write("=================================================================\n")
            sys.stderr.write("Test:%s error \n" % test.id())
            sys.stderr.write("%s\n" % trace_back)

        for (test, trace_back) in test_result.failures:
            sys.stderr.write("=================================================================\n")
            sys.stderr.write("Test:%s failure \n" % test.id())
            sys.stderr.write("%s\n" % trace_back)

        return False
    else:
        return True


if __name__ == '__main__':
    PYTHONPATH = sys.argv[1]
    sys.path.insert(0, PYTHONPATH)

    test_class_path = sys.argv[2]
    argv = []

    try:
        argv = sys.argv[3:]
    except IndexError:
        pass

    from ert.test import ErtTestRunner
    tests = ErtTestRunner.getTestsFromTestClass(test_class_path, argv)

    if runTestCase(tests):
        sys.exit(0)
    else:
        sys.exit(1)
