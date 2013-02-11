#!/usr/bin/env python
import unittest
import sys

import sum_test
import sched_test
import large_mem_test
import file_test
import grdecl_test
import grid_test

def run_suite(name , suite):
    print "Running tests from %12s:" % name,
    sys.stdout.flush()
    unittest.TextTestRunner().run( suite )


def run(name , module):
    if hasattr(module , "fast_suite"):
        run_suite( name , getattr(module , "fast_suite")())
        
run("kw"         , kw_test)    
run("summary"    , sum_test)
run("sched_file" , sched_test)
run("file_test"  , file_test)
run("large_mem"  , large_mem_test)
run("grdecl"     , grdecl_test)
run("grid"       , grid_test)
