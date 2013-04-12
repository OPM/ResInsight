#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'tvector_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import datetime
import unittest
import ert
from   ert.util.tvector import DoubleVector
from   ert.util.tvector import IntVector
from   ert.util.tvector import BoolVector

from   test_util import approx_equal, approx_equalv



class TVectorTest( unittest.TestCase ):
    

    def setUp(self):
        pass


    def test_activeList(self):
        active_list = IntVector.active_list("1,10,100-105")
        self.assertTrue( len(active_list) == 8 )
        self.assertTrue( active_list[0] == 1)
        self.assertTrue( active_list[2] == 100)
        self.assertTrue( active_list[7] == 105)
        
        active_list = IntVector.active_list("1,10,100-105X")
        self.assertFalse( active_list )



    def test_activeMask(self):
        active_list = BoolVector.active_mask("1 , 4 - 7 , 10")
        self.assertTrue( len(active_list) == 11 )
        self.assertTrue( active_list[1])
        self.assertTrue( active_list[4])
        self.assertTrue( active_list[10])
        self.assertFalse( active_list[9])
        self.assertFalse( active_list[8])

        active_list = BoolVector.active_mask("1,4-7,10X")
        self.assertFalse( active_list )
        

        
    def test_true(self):
        l = IntVector()
        self.assertFalse( l )    # Will invoke the __len__ function; could override with __nonzero__
        l[0] = 1
        self.assertTrue( l )



def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( TVectorTest( 'test_activeList' ))
    suite.addTest( TVectorTest( 'test_activeMask' ))
    suite.addTest( TVectorTest( 'test_true' ))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )


