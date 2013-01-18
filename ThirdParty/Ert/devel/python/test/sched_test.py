#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'sched_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import ert.sched.sched as sched
from   test_util import approx_equal, approx_equalv, file_equal


src_file = "test-data/Statoil/ECLIPSE/Gurbat/target.SCH"
start_time = datetime.date(2000 , 1, 1)

class SchedFileTest( unittest.TestCase ):
    def setUp(self):
        self.sched_file = sched.SchedFile( src_file , start_time )
        self.file_list = []

    def addFile( self , file ):
        self.file_list.append( file )

    def tearDown(self):
        for file in self.file_list:
            if os.path.exists( file ):
                os.unlink( file )

    def test_load(self):
        self.assertTrue( self.sched_file , "Load failed")


    def test_length(self):
        self.assertEqual( self.sched_file.length , 63 )


    def test_write_loop(self):
        self.sched_file.write( "/tmp/schedule1" , 62)
        sched_file2 = sched.SchedFile( "/tmp/schedule1" , start_time)
        sched_file2.write( "/tmp/schedule2" , 62)
        self.assertTrue( file_equal( "/tmp/schedule1" , "/tmp/schedule2") ) 

        self.addFile( "/tmp/schedule1" )
        self.addFile( "/tmp/schedule2" )


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( SchedFileTest( 'test_load' ))
    suite.addTest( SchedFileTest( 'test_length' ))
    suite.addTest( SchedFileTest( 'test_write_loop' ))
    return suite
        

if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )
