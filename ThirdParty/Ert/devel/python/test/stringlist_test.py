#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'stringlist_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import unittest
import stat
import math
import ert
from   ert.util.stringlist import StringList
import sys
from   test_util import *
import ert.ecl.ecl as ecl

initList = ["S1" , "SABC" , "S33"]    

base = "ECLIPSE"
path = "test-data/Statoil/ECLIPSE/Gurbat"
case = "%s/%s" % (path , base)


def pop_empty():
    s = StringList( initial = initList )
    s.pop()
    s.pop()
    s.pop()
    s.pop()

def last_empty():
    s = StringList( initial = initList )
    s.pop()
    s.pop()
    s.pop()
    s.last


class StringListTest( unittest.TestCase ):
    
    def setUp( self ):
        pass


    def create(self):
        s = StringList( initial = initList )
        st = s.strings
        del s
        return st

    def test_pop( self ):
        s = StringList( initial = initList )
        s1 = s.pop()
        self.assertTrue( len(s) == 2 )
        self.assertTrue( s1 == "S33")

        s1 = s.pop()
        self.assertTrue( len(s) == 1 )
        self.assertTrue( s1 == "SABC")

        s1 = s.pop()
        self.assertTrue( len(s) == 0 )
        self.assertTrue( s1 == "S1")

        self.assertRaises( IndexError , pop_empty )


    def test_create( self ):
        s = StringList()
        
        s = StringList( initial = initList )
        self.assertTrue( len(s) == 3 )
        for i in range(len(s)):
            self.assertTrue( s[i] == initList[i] )
            
        s2 = s.strings
        for i in range(len(s)):
            self.assertTrue( s2[i] == initList[i] )
    
        s3 = self.create()
        for i in range(len(s)):
            self.assertTrue( s3[i] == initList[i] )


    def test_last( self ):
        s = StringList( initial = initList )
        l = s.last
        self.assertTrue( "S33" == l )
        self.assertRaises( IndexError , last_empty)



        


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( StringListTest( 'test_create' ))
    suite.addTest( StringListTest( 'test_pop' ))
    suite.addTest( StringListTest( 'test_last' ))
    return suite

                   
def test_suite(argv):
    return fast_suite()


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )

