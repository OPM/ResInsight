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


def add_size_error():
    l1 = IntVector()
    l2 = IntVector()

    l1[100] = 10
    l2[10] = 1
    l1 += l2


def mul_size_error():
    l1 = IntVector()
    l2 = IntVector()

    l1[100] = 10
    l2[10] = 1
    l1 *= l2
    


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



    def test_default(self):
        l = IntVector( default_value = 100 )
        self.assertTrue( l.default == 100 )
        l[10] = 1
        self.assertTrue( l[0]  == 100 )
        self.assertTrue( l[9]  == 100 )
        self.assertTrue( l[10] == 1 )

        l.default = 200
        self.assertTrue( l.default == 200 )
        l[20] = 77
        self.assertTrue( l[19] == 200 )

    
    def test_copy(self):
        l1 = IntVector( default_value = 77 )
        l1[99] = 100
        
        l2 = l1.copy()
        self.assertTrue( l1.default == l2.default )
        self.assertTrue( len(l1) == len(l2))
        self.assertTrue( l1[99] == l2[99] )


    def test_div(self):
        l1 = IntVector( default_value = 100 )
        l1[99] = 100
        l2 = l1 / 10
        
        self.assertTrue( l1.default == l2.default )
        self.assertTrue( len(l1) == len(l2))
        self.assertTrue( l2[99] == 10 )
        self.assertTrue( l2[9] == 10 )
        self.assertTrue( l2[1] == 10 )


    def test_add(self):
        l1 = IntVector( default_value = 100 )
        l1[99] = 100
        l1 += 100
        
        self.assertTrue( l1[99] == 200 )
        self.assertTrue( l1[9] == 200 )
        self.assertTrue( l1[1] == 200 )

        l2 = IntVector( default_value = 75 )
        l2[99] = 75
        l1 += l2
        
        self.assertTrue( l1[99] == 275 )
        self.assertTrue( l1[9] == 275 )
        self.assertTrue( l1[1] == 275 )

        self.assertRaises( ValueError , add_size_error )


    def test_mul(self):
        l1 = IntVector( default_value = 100 )
        l1[99] = 100
        l1 *= 2
        
        self.assertTrue( l1[99] == 200 )
        self.assertTrue( l1[9] == 200 )
        self.assertTrue( l1[1] == 200 )

        l2 = IntVector( default_value = 2 )
        l2[99] = 2
        l1 *= l2
        
        self.assertTrue( l1[99] == 400 )
        self.assertTrue( l1[9] == 400 )
        self.assertTrue( l1[1] == 400 )

        self.assertRaises( ValueError , mul_size_error )






def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( TVectorTest( 'test_activeList' ))
    suite.addTest( TVectorTest( 'test_activeMask' ))
    suite.addTest( TVectorTest( 'test_true' ))
    suite.addTest( TVectorTest( 'test_default' ))
    suite.addTest( TVectorTest( 'test_copy' ))
    suite.addTest( TVectorTest( 'test_div' ))
    suite.addTest( TVectorTest( 'test_add' ))
    suite.addTest( TVectorTest( 'test_mul' ))
    return suite

def test_suite(argv):
    return fast_suite()



if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )


