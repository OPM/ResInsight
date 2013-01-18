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
    

class RestartTest( unittest.TestCase ):

    def setUp(self):
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )


    def test_report(self):
        self.assertTrue( ecl.EclFile.contains_report_step( file , 4 ))
        self.assertTrue( ecl.EclFile.contains_report_step( file , 0 ))
        self.assertTrue( ecl.EclFile.contains_report_step( file , 62 ))
        self.assertFalse( ecl.EclFile.contains_report_step( file , -1 ))
        self.assertFalse( ecl.EclFile.contains_report_step( file , 100 ))
        
        f = ecl.EclFile( file )
        self.assertTrue( f.has_report_step( 4 ))
        self.assertTrue( f.has_report_step( 0 ))
        self.assertTrue( f.has_report_step( 62 ))

        self.assertFalse( f.has_report_step( -1 ))
        self.assertFalse( f.has_report_step( 100 ))




    def test_dates(self):
        f = ecl.EclFile( file )
        self.assertTrue( f.has_sim_time( datetime.datetime( 2001 , 6 , 1) ))
        self.assertFalse( f.has_sim_time( datetime.datetime( 2005 , 6 , 1) ))
        self.assertFalse( f.has_sim_time( datetime.datetime( 1999 , 6 , 1) ))
        self.assertFalse( f.has_sim_time( datetime.datetime( 2001 , 6 , 11) ))

        self.assertTrue(  ecl.EclFile.contains_sim_time( file , datetime.datetime( 2001 , 6 , 1) ))
        self.assertFalse( ecl.EclFile.contains_sim_time( file , datetime.datetime( 2005 , 6 , 1) ))
        self.assertFalse( ecl.EclFile.contains_sim_time( file , datetime.datetime( 1999 , 6 , 1) ))
        self.assertFalse( ecl.EclFile.contains_sim_time( file , datetime.datetime( 2001 , 6 , 11) ))

        
    def test_kw( self ):
        f = ecl.EclFile( file )
        kw1 = f["SWAT"][40]
        kw2 = f.restart_get_kw( "SWAT" , datetime.datetime( 2003 , 3 , 1 ))
        kw3 = f.restart_get_kw( "SWAT" , datetime.datetime( 2003 , 3 , 1 ) , copy = True)

        self.assertTrue( kw1.equal( kw2 ))
        self.assertTrue( kw1.equal( kw3 ))

    
        kw4 = f.restart_get_kw( "SWAT" , datetime.datetime( 2009 , 3 , 1 ))
        self.assertTrue( kw4 is None )


                     

def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( RestartTest( 'test_report' )) 
    suite.addTest( RestartTest( 'test_dates' )) 
    suite.addTest( RestartTest( 'test_kw' )) 
    return suite



if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
