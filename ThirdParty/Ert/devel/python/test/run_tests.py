#!/usr/bin/env python
import sys
from ert_tests.run_tests import *

from ert.test import ErtTestRunner

#runTestsInClass("ert_tests.util.test_string_list.StringListTest")
ErtTestRunner.runTestsInClass("ert_tests.run.test_run.RunTest")
