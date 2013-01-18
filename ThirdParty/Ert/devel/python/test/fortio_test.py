#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'kw_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import ert.ecl.ecl as ecl
import sys


unrst_file = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST"

def open_noex():
    f = ecl.FortIO.reader("/tmp/does/notExist")


class FortIOTest( unittest.TestCase ):

    def setUp(self):
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )


    def test_open_read(self):
        f = ecl.FortIO.reader(unrst_file)
        self.assertTrue( f )


    def test_open_write(self):
        self.addFile( "/tmp/newfile" )
        f = ecl.FortIO.writer("/tmp/newfile")
        f.close()
        self.assertTrue( True )

    def test_noex(self):
        self.assertRaises( IOError , open_noex)

    def test_kw(self):
        self.addFile( "/tmp/test" )
        kw1 = ecl.EclKW.create( "KW1" , 2 , ecl.ECL_INT_TYPE )
        kw2 = ecl.EclKW.create( "KW2" , 2 , ecl.ECL_INT_TYPE )

        kw1[0] = 99
        kw1[1] = 77
        kw2[0] = 113
        kw2[1] = 335

        f = ecl.FortIO.writer( "/tmp/test" , fmt_file = False)
        kw1.fwrite( f )
        f.close()
        
        f = ecl.FortIO.open( "/tmp/test" , mode = "a")
        kw2.fwrite( f )
        f.close()
        
        f = ecl.FortIO.open( "/tmp/test" , fmt_file = False)
        k1 = ecl.EclKW.fread( f )
        k2 = ecl.EclKW.fread( f )
        f.close()
        
        self.assertTrue( k1.equal( kw1 ))
        self.assertTrue( k2.equal( kw2 ))



def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( FortIOTest('test_open_read'))
    suite.addTest( FortIOTest('test_open_write'))
    suite.addTest( FortIOTest('test_noex'))
    suite.addTest( FortIOTest('test_kw'))
    return suite




if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
