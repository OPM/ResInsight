#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'large_mem_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import datetime
import unittest
import ert
import ert.ecl.ecl as ecl
from   ert.util.tvector import DoubleVector
from   test_util import approx_equal, approx_equalv


case = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE"


class MemTest( unittest.TestCase ):
    def setUp(self):
        # Funny setup to ensure that the 32 bit memory region is
        # exhausted. Then we do some (grid-based) pointer
        # dereferencing in the 64 bit memory region - this has been
        # problematic in the past.
        vecList = []
        for i in range(12500):
            vec = DoubleVector()
            vec[81920] = 0
            vecList.append( vec )
            
        self.grid1 = ecl.EclGrid( case )  
        self.grid2 = ecl.EclGrid( case )
        

    def test_nactive(self):
        self.assertEqual( self.grid1.nactive , self.grid2.nactive )
        self.assertEqual( self.grid1.nactive , 34770 )


def slow_suite():
    suite = unittest.TestSuite()
    suite.addTest( MemTest( 'test_nactive' ))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner().run( slow_suite() )

