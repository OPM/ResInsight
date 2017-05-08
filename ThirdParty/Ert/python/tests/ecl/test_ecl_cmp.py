#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'test_ecl_cmp.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ecl.test import ExtendedTestCase , TestAreaContext
from ecl.test.ecl_mock import createEclSum
from ecl.ecl import EclCmp

class EclCmpTest(ExtendedTestCase):
    def setUp(self):
        self.root1 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE")
        self.root2 = self.createTestPath("Statoil/ECLIPSE/Oseberg/F8MLT/F8MLT-F4")

        
    def test_not_existing(self):
        with self.assertRaises(IOError):
            ecl_cmp = EclCmp( "missing/case1" , "missing/case2")

        with self.assertRaises(IOError):
            ecl_cmp = EclCmp( "missing/case1" , self.root1 )

        with self.assertRaises(IOError):
            ecl_cmp = EclCmp( self.root1 , "missing/case1")

        ecl_cmp = EclCmp( self.root1 , self.root1)
        ecl_cmp = EclCmp( self.root2 , self.root2)

        
    def test_different_start(self):
        with self.assertRaises(ValueError):
            ecl_cmp = EclCmp(self.root1 , self.root2)

            
    def test_summary_cmp(self):
        ecl_cmp = EclCmp( self.root1 , self.root1)
        self.assertEqual( (False , False) , ecl_cmp.hasSummaryVector("MISSING"))
        self.assertEqual( (True , True) , ecl_cmp.hasSummaryVector("FOPT"))
        
        with self.assertRaises(KeyError):
            diff = ecl_cmp.cmpSummaryVector("MISSING")

        diff_sum , ref_sum = ecl_cmp.cmpSummaryVector("FOPT")
        self.assertEqual( diff_sum , 0.0 )
        self.assertTrue( ecl_cmp.endTimeEqual( ) )

        
    def test_wells(self):
        ecl_cmp = EclCmp( self.root1 , self.root1)
        wells = ecl_cmp.testWells()

        well_set = set( ["OP_1" , "OP_2" , "OP_3" , "OP_4" , "OP_5" , "WI_1" , "WI_2" , "WI_3"] )
        self.assertEqual( len(wells) , len(well_set))
        for well in wells:
            self.assertTrue( well in well_set )
            

