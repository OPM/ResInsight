#!/usr/bin/env python
import sys
import os
import unittest

def run_suite( test_suite ):
    test_result = unittest.TextTestRunner(verbosity = 0).run( test_suite )
    if test_result.errors or test_result.failures:
        for (test , trace_back) in test_result.errors:
            sys.stderr.write("=================================================================\n")
            sys.stderr.write("Test:%s error \n" % test.id())
            sys.stderr.write("%s\n" % trace_back)

        for (test , trace_back) in test_result.failures:
            sys.stderr.write("=================================================================\n")
            sys.stderr.write("Test:%s failure \n" % test.id())
            sys.stderr.write("%s\n" % trace_back)

        return False
    else:
        return True



PYTHONPATH = sys.argv[1]
test_module = sys.argv[2]
argv = []

sys.path.insert( 0 , PYTHONPATH )

test_module = __import__(sys.argv[2])

try:
    argv = sys.argv[3:]
except:
    pass

test_suite = test_module.test_suite( argv )
if test_suite:
    if run_suite( test_suite ):
        sys.exit( 0 )
    else:
        sys.exit( 1 )
else:
    sys.exit( 0 )

