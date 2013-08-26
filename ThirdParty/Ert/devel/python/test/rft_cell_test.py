#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'rft_cell_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import unittest
import ert.ecl.ecl as ecl
from   test_util import approx_equal, approx_equalv


RFT_file = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.RFT"
PLT_file = "test-data/Statoil/ECLIPSE/RFT/TEST1_1A.RFT"




def out_of_range():
    rftFile = ecl.EclRFTFile( RFT_file )
    rft = rftFile[100]



class RFTCellTest( unittest.TestCase ):

    def RFTCell(self):
        i = 10
        j = 8
        k = 100
        depth = 100
        pressure = 65
        swat = 0.56
        sgas = 0.10
        cell = ecl.EclRFTCell.new( i , j , k , depth , pressure , swat , sgas )
        
        self.assertTrue( i == cell.get_i() )
        self.assertTrue( j == cell.get_j() )
        self.assertTrue( k == cell.get_k() )
        
        self.assertTrue( approx_equal( pressure , cell.pressure) )
        self.assertTrue( approx_equal( depth    , cell.depth ))
        self.assertTrue( approx_equal( swat     , cell.swat) )
        self.assertTrue( approx_equal( sgas     , cell.sgas ))
        self.assertTrue( approx_equal( 1 - (sgas + swat)  , cell.soil ))





    def PLTCell(self):
        i = 2
        j = 16
        k = 100
        depth = 100
        pressure = 65
        orat = 0.78
        grat = 88
        wrat = 97213
        conn_start = 214
        flowrate = 111
        oil_flowrate = 12
        gas_flowrate = 132
        water_flowrate = 13344

        cell = ecl.EclPLTCell.new( i , j , k , depth , pressure , orat , grat , wrat , conn_start , flowrate , oil_flowrate , gas_flowrate , water_flowrate)
                                   
        
        self.assertTrue( i == cell.get_i() )
        self.assertTrue( j == cell.get_j() )
        self.assertTrue( k == cell.get_k() )
        
        self.assertTrue( cell.get_i() + 1 == cell.i )
        self.assertTrue( cell.get_j() + 1 == cell.j )
        self.assertTrue( cell.get_k() + 1 == cell.k )
        
        
        self.assertTrue( approx_equal( pressure , cell.pressure) )
        self.assertTrue( approx_equal( depth    , cell.depth ))
        self.assertTrue( approx_equal( orat     , cell.orat) )
        self.assertTrue( approx_equal( grat     , cell.grat ))
        self.assertTrue( approx_equal( wrat     , cell.wrat ))

        self.assertTrue( approx_equal( conn_start , cell.conn_start) )
        self.assertTrue( approx_equal( flowrate           , cell.flowrate      ))
        self.assertTrue( approx_equal( oil_flowrate       , cell.oil_flowrate  ))
        self.assertTrue( approx_equal( gas_flowrate       , cell.gas_flowrate  ))
        self.assertTrue( approx_equal( water_flowrate     , cell.water_flowrate))
    
        




def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( RFTCellTest( 'RFTCell' ))
    suite.addTest( RFTCellTest( 'PLTCell' ))
    return suite



def test_suite( argv ):
    return fast_suite()


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )


        
