#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'grid_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os
import ert
import ert.ecl.ecl as ecl
import datetime
import time
import unittest
import ert
import ert.ecl.ecl as ecl
from   ert.util.tvector import DoubleVector
from   ert.util.tvector import DoubleVector

from   test_util import approx_equal, approx_equalv

troll_case = "/d/proj/bg/restroll6/ressim/ff/2007a/e100/simu/O22/O22YH_UPDATE" 
bprod_path = "/d/proj/bg/restroll6/bin/bprod.py"
dump_path  = "/d/proj/bg/restroll6/bin/eclipse_dump.py"

class TrollTest( unittest.TestCase ):
    def setUp(self):
        pass


    def testBPROD(self):
        os.system("%s %s" % (bprod_path , troll_case))
        self.assertTrue( True )


    def testDUMP( self ):
        os.system("%s %s" % (dump_path , troll_case))
        self.assertTrue( True )


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( TrollTest( 'testBPROD' ))
    suite.addTest( TrollTest( 'testDUMP' ))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
