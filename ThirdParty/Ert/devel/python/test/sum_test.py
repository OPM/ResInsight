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

import os
import datetime
import unittest
import ert
import ert.ecl.ecl as ecl
from   test_util import approx_equal, approx_equalv

base = "ECLIPSE"
path = "test-data/Statoil/ECLIPSE/Gurbat"
case = "%s/%s" % (path , base)


def sum_get(*args):
    sum = args[0]
    key = args[1]
    vec = sum[key]




class SumTest( unittest.TestCase ):

    def setUp(self):
        self.case = case
        self.sum = ecl.EclSum( self.case )
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )

    def test_load(self):
        self.assertTrue( self.sum , "Load failed")
        

    def test_interp(self):
        sum = self.sum
        self.assertTrue( approx_equal( sum.get_interp( "WWCT:OP_3" , days = 750 ) , 0.11719122))
        self.assertTrue( approx_equal( sum.get_interp( "WWCT:OP_3" , date = datetime.date( 2004,1,1)) , 0.603358387947 ))
        
        v = sum.get_interp_vector( "WOPT:OP_1" , days_list = [100 , 200 , 400 , 800] )
        self.assertTrue( approx_equalv( [805817.11875 , 1614955.34677419 , 3289267.67857143 , 6493021.6218035 ] , v))
        
        v = sum.get_interp_vector( "WGPT:OP_2" , date_list = [datetime.date(2002,1,1) , datetime.date(2003,1,1) , datetime.date(2004 , 1 , 1)])
        self.assertTrue( approx_equalv( v ,  [ 8.20773632e+08  , 9.68444032e+08  , 1.02515213e+09]) )

        
    def test_wells(self):
        sum = self.sum
        wells = sum.wells()
        wells.sort()
        self.assertTrue( approx_equalv( wells , ["OP_1" , "OP_2" , "OP_3" , "OP_4" , "OP_5" , "WI_1" , "WI_2" , "WI_3"]))
        
        wells = sum.wells( pattern = "*_3")
        wells.sort()
        self.assertTrue( approx_equalv( wells , ["OP_3" , "WI_3"]))

        groups = sum.groups()
        groups.sort()
        self.assertTrue( approx_equalv( groups , ['GMWIN','OP','WI']))
        

    def test_last( self ):
        sum = self.sum
        last = sum.get_last("FOPT")
        self.assertTrue( approx_equal( last.value , 38006336.0 ))
        self.assertTrue( approx_equal( last.days , 1826.0 ))
        self.assertEqual( last.date , datetime.datetime( 2004 , 12 , 31, 0,0,0))

        self.assertTrue( approx_equal( sum.get_last_value("FGPT") , 6605249024.0)) 


    def test_dates( self ):
        sum = self.sum
        d = sum.dates

        self.assertEqual( len(d) , 63 )
        self.assertEqual( d[25] , datetime.datetime( 2001 , 12 , 1 , 0 , 0 , 0))
        self.assertEqual( sum.iget_date( 25 ) , datetime.datetime( 2001 , 12 , 1 , 0 , 0 , 0))
        
        mpl_dates = sum.mpl_dates
        self.assertTrue( approx_equal( mpl_dates[25] , 730820 ))
        
        days = sum.days
        self.assertTrue( approx_equal( days[50] , 1461 ))


    def test_keys(self):
        sum = self.sum
        self.assertRaises( KeyError , sum.__getitem__ , "BJARNE" )
        
        v = sum["FOPT"]
        self.assertEqual( len(v) , 63 )


    def test_index(self):
        sum = self.sum
        index = sum.get_key_index( "TCPUDAY")
        self.assertEqual( index , 10239 ) 


    def test_report(self):
        sum = self.sum
        self.assertEqual( sum.get_report( date = datetime.date( 2000,10,1) ) , 10)
        self.assertEqual( sum.get_report( date = datetime.date( 2000,10,3) ) , -1)
        self.assertEqual( sum.get_report( date = datetime.date( 1980,10,3) ) , -1)
        self.assertEqual( sum.get_report( date = datetime.date( 2012,10,3) ) , -1)

        self.assertEqual( sum.get_report( days = 91 ) , 3)
        self.assertEqual( sum.get_report( days = 92 ) , -1)
        self.assertTrue( approx_equal( sum.get_interp( "FOPT" , days = 91 ) , sum.get_from_report( "FOPT" , 3 )) )

        self.assertEqual( sum.first_report , 1 )
        self.assertEqual( sum.last_report  , 62 )

        self.assertEqual( sum.get_report_time( 10 ) , datetime.date( 2000 , 10 , 1))
        self.assertTrue(  approx_equal( sum.get_from_report( "FOPT" , 10 ) , 6.67447e+06) )


    def test_fwrite(self):
        self.sum.fwrite(ecl_case = "/tmp/CASE" )
        self.assertTrue( True )


    def test_block(self):
        sum = self.sum
        index_ijk = sum.get_key_index("BPR:15,28,1")
        index_num = sum.get_key_index("BPR:1095")
        self.assertEqual( index_ijk , index_num )


    def test_restart(self):
        hist = ecl.EclSum( "test-data/ECLIPSE/sum-restart/history/T07-4A-W2011-18-P1" )
        base = ecl.EclSum( "test-data/ECLIPSE/sum-restart/prediction/BASECASE" )
        pred = ecl.EclSum( "test-data/ECLIPSE/sum-restart/prediction/BASECASE" , include_restart = False)

        self.assertTrue( True )


    def test_case1(self ):
        self.assertTrue( self.sum.path     == path )
        self.assertTrue( self.sum.base     == base )
        self.assertTrue( self.sum.case     == case )
        self.assertTrue( self.sum.abs_path == os.path.realpath(os.path.join( os.getcwd() , path )))


    def test_case2( self ):
        cwd = os.getcwd()
        os.chdir( path )
        sum = ecl.EclSum( base )
        self.assertTrue( sum.path is None )
        self.assertTrue( sum.base     == base )
        self.assertTrue( sum.case     == base )
        self.assertTrue( sum.abs_path == os.path.realpath(os.path.join( cwd , path )))
        os.chdir( cwd )


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( SumTest( 'test_load' ))
    suite.addTest( SumTest( 'test_case1' ))
    suite.addTest( SumTest( 'test_case2' ))
    suite.addTest( SumTest( 'test_interp' ))
    suite.addTest( SumTest( 'test_wells' ))
    suite.addTest( SumTest( 'test_last' ))
    suite.addTest( SumTest( 'test_dates' ))
    suite.addTest( SumTest( 'test_keys' ))
    suite.addTest( SumTest( 'test_index' ))
    suite.addTest( SumTest( 'test_report' ))
    suite.addTest( SumTest( 'test_fwrite' ))
    suite.addTest( SumTest( 'test_block' ))
    suite.addTest( SumTest( 'test_restart' ))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
