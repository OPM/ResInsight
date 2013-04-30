#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import filecmp
import datetime
import unittest
import shutil
import time
import os
import os.path
import ert
import ert.ecl.ecl as ecl
from   test_util import approx_equal, approx_equalv, file_equal


file     = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST"
fmt_file = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.FUNRST"

def load_missing():
    ecl.EclFile( "No/Does/not/exist")
    

class FileTest( unittest.TestCase ):

    def setUp(self):
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )
            

    def testIOError(self):
        self.assertRaises( IOError , load_missing)

    
    def test_fwrite( self ):
        self.addFile( "/tmp/ECLIPSE.UNRST" )
        rst_file = ecl.EclFile( file )
        fortio = ecl.FortIO.writer("/tmp/ECLIPSE.UNRST")
        rst_file.fwrite( fortio )
        fortio.close()
        rst_file.close()
        self.assertTrue( file_equal( "/tmp/ECLIPSE.UNRST" , file ) ) 


    def test_save(self):
        self.addFile( "/tmp/ECLIPSE.UNRST" )
        shutil.copyfile( file , "/tmp/ECLIPSE.UNRST" )
        rst_file = ecl.EclFile( "/tmp/ECLIPSE.UNRST" , flags = ecl.ECL_FILE_WRITABLE )
        swat0 = rst_file["SWAT"][0]
        swat0.assign( 0.75 )
        rst_file.save_kw( swat0 )
        rst_file.close( )
        self.assertFalse( file_equal( "/tmp/ECLIPSE.UNRST" , file ) )
        
        rst_file1 = ecl.EclFile( file )
        rst_file2 = ecl.EclFile( "/tmp/ECLIPSE.UNRST" , flags = ecl.ECL_FILE_WRITABLE)
        
        swat1 = rst_file1["SWAT"][0]
        swat2 = rst_file2["SWAT"][0]
        swat2.assign( swat1 )

        rst_file2.save_kw( swat2 )
        self.assertTrue( swat1.equal( swat2 ))
        rst_file1.close()
        rst_file2.close()

        # Random failure ....
        self.assertTrue( file_equal( "/tmp/ECLIPSE.UNRST" , file ) ) 
        

    def test_save_fmt(self):
        self.addFile( "/tmp/ECLIPSE.FUNRST" )
        shutil.copyfile( fmt_file , "/tmp/ECLIPSE.FUNRST" )
        rst_file = ecl.EclFile( "/tmp/ECLIPSE.FUNRST" , flags = ecl.ECL_FILE_WRITABLE)
        swat0 = rst_file["SWAT"][0]
        swat0.assign( 0.75 )
        rst_file.save_kw( swat0 )
        rst_file.close( )
        self.assertFalse( file_equal( "/tmp/ECLIPSE.FUNRST" , fmt_file ) )
        
        rst_file1 = ecl.EclFile( fmt_file )
        rst_file2 = ecl.EclFile( "/tmp/ECLIPSE.FUNRST" , flags = ecl.ECL_FILE_WRITABLE)
        
        swat1 = rst_file1["SWAT"][0]
        swat2 = rst_file2["SWAT"][0]

        swat2.assign( swat1 )
        rst_file2.save_kw( swat2 )
        self.assertTrue( swat1.equal( swat2 ))
        rst_file1.close()
        rst_file2.close()

        # Random failure ....
        self.assertTrue( file_equal( "/tmp/ECLIPSE.FUNRST" , fmt_file ) ) 
        


def slow_suite():
    suite = unittest.TestSuite()
    suite.addTest( FileTest( 'test_save' ))
    suite.addTest( FileTest( 'test_save_fmt' ))
    return suite


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( FileTest( 'testIOError' )) 
    suite.addTest( FileTest( 'test_fwrite' ))
    return suite




def test_suite( argv ):
    test_list = fast_suite()
    if argv:
        if argv[0][0] == "T":
            for t in slow_suite():
                test_list.addTest( t )
    return test_list

            

if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
    unittest.TextTestRunner().run( slow_suite() )
