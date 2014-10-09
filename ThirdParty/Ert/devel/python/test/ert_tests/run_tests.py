#!/usr/bin/env python
import sys
import os
from ert.test import ErtTestRunner

sys.path.append( os.path.realpath( os.path.join(os.path.dirname( os.path.abspath( __file__)) , "../") ))

if __name__ == '__main__':
    ErtTestRunner.runTestsInDirectory(".")
