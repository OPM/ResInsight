#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'rft_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import ert.ecl.ecl as ecl
from   test_util import approx_equal, approx_equalv


RFT_file = "test-data/Statoil/ECLIPSE/Gurbat/ECLIPSE.RFT"
PLT_file = "test-data/Statoil/ECLIPSE/RFT/TEST1_1A.RFT"




def out_of_range():
    rftFile = ecl.EclRFTFile( RFT_file )
    rft = rftFile[100]



class RFTTest( unittest.TestCase ):

    def loadRFT( self ):
        rftFile = ecl.EclRFTFile( RFT_file )

        rft = rftFile[0]
        cell = rft.ijkget( (32 , 53 , 0) )
        self.assertTrue( isinstance( cell , ecl.EclRFTCell ))

        self.assertEqual( 2   , rftFile.size( ) )
        self.assertEqual( 0   , rftFile.size( well = "OP*"))
        self.assertEqual( 0   , rftFile.size( well = "XXX"))
        self.assertEqual( 1   , rftFile.size( date = datetime.date( 2000 , 6  , 1 )))
        self.assertEqual( 0   , rftFile.size( date = datetime.date( 2000 , 6  , 17 )))
                          
        cell = rft.ijkget( (30 , 20 , 1880) )
        self.assertTrue( cell is None )

        for rft in rftFile:
            self.assertTrue( rft.is_RFT() )
            self.assertFalse( rft.is_SEGMENT( ))
            self.assertFalse( rft.is_PLT( ))
            self.assertFalse( rft.is_MSW( ))
            
            for cell in rft:
                self.assertTrue( isinstance( cell , ecl.EclRFTCell ))
                
            cell0 = rft.iget_sorted( 0 )
            self.assertTrue( isinstance( cell , ecl.EclRFTCell ))
            rft.sort()


                
    def loadPLT( self ):
        pltFile = ecl.EclRFTFile( PLT_file )
        plt = pltFile[11]
        self.assertTrue( plt.is_PLT() )
        self.assertFalse( plt.is_SEGMENT( ))
        self.assertFalse( plt.is_RFT( ))
        self.assertFalse( plt.is_MSW( ))
        for cell in plt:
            self.assertTrue( isinstance( cell , ecl.EclPLTCell ))


    def exceptions( self ):
        self.assertRaises( IndexError , out_of_range )
    

        

def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( RFTTest( 'loadRFT' ))
    suite.addTest( RFTTest( 'loadPLT' ))
    suite.addTest( RFTTest( 'exceptions' ))
    return suite


def test_suite( argv ):
    return fast_suite()


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )


        
