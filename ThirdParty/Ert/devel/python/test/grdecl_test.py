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

import datetime
import unittest
import ert
import os
import ert.ecl.ecl as ecl
from   test_util import approx_equal, approx_equalv, file_equal


src_file = "test-data/Statoil/ECLIPSE/Gurbat/include/example_permx.GRDECL"


class GRDECLTest( unittest.TestCase ):

    def setUp(self):
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )



    def testLoad( self ):
        kw = ecl.EclKW.read_grdecl( open( src_file , "r") , "PERMX")
        self.assertTrue( kw )


    def testReLoad( self ):
        kw = ecl.EclKW.read_grdecl( open( src_file , "r") , "PERMX")
        tmp_file1 = "/tmp/permx1.grdecl"
        tmp_file2 = "/tmp/permx2.grdecl"
        self.addFile( tmp_file1 )
        self.addFile( tmp_file2 )

        fileH = open( tmp_file1 , "w")
        kw.write_grdecl( fileH )
        fileH.close()

        kw1 = ecl.EclKW.read_grdecl( open( tmp_file1 , "r") , "PERMX") 
        
        fileH = open( tmp_file2 , "w")
        kw1.write_grdecl( fileH )
        fileH.close()

        self.assertTrue( file_equal( tmp_file1 , tmp_file2 ))



    def test_fseek( self ):
        file = open( src_file , "r")
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "PERMX" ) )
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMY" ) )  
        file.close()

        file = open( src_file , "r")
        kw1 = ecl.EclKW.read_grdecl( file , "PERMX") 
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMX" ) )
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "PERMX" , rewind = True) )
        file.close()

        
        
    def test_fseek2(self):
        test_src = "test-data/local/ECLIPSE/grdecl-test/test.grdecl"
        # Test kw at the the very start
        file = open( test_src , "r")
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "PERMX" ) )
        
        # Test commented out kw:
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMY" ) )
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMZ" ) )

        # Test ignore not start of line:
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "MARKER" ) )
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMXYZ" ) )

        # Test rewind 
        self.assertFalse(ecl.EclKW.fseek_grdecl( file , "PERMX" , rewind = False) )
        self.assertTrue(ecl.EclKW.fseek_grdecl( file , "PERMX" , rewind = True) )

        # Test multiline comments + blanks
        self.assertTrue(ecl.EclKW.fseek_grdecl( file , "LASTKW" ) )


    def test_fseek_dos(self):
        test_src = "test-data/local/ECLIPSE/grdecl-test/test.grdecl_dos"  # File formatted with \r\n line endings.
        # Test kw at the the very start
        file = open( test_src , "r")
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "PERMX" ) )
        
        # Test commented out kw:
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMY" ) )
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMZ" ) )

        # Test ignore not start of line:
        self.assertTrue( ecl.EclKW.fseek_grdecl( file , "MARKER" ) )
        self.assertFalse( ecl.EclKW.fseek_grdecl( file , "PERMXYZ" ) )

        # Test rewind 
        self.assertFalse(ecl.EclKW.fseek_grdecl( file , "PERMX" , rewind = False) )
        self.assertTrue(ecl.EclKW.fseek_grdecl( file , "PERMX" , rewind = True) )

        # Test multiline comments + blanks
        self.assertTrue(ecl.EclKW.fseek_grdecl( file , "LASTKW" ) )




def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( GRDECLTest( 'testLoad' ))
    suite.addTest( GRDECLTest( 'testReLoad' ))
    suite.addTest( GRDECLTest( 'test_fseek' ))
    suite.addTest( GRDECLTest( 'test_fseek_dos' ))
    return suite

if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
