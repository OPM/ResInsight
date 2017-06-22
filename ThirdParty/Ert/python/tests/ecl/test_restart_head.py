#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'test_ecl_init_file.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ecl.test import ExtendedTestCase
from ecl.ecl import Ecl3DKW , EclKW, EclRestartFile , EclFile, FortIO, EclFileFlagEnum , EclGrid

class RestartHeadTest(ExtendedTestCase):
    def setUp(self):
        self.grid_file =   self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")
        self.unrst_file   = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.xrst_file0   = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0000")

    def test_headers(self):
        g = EclGrid( self.grid_file )
        f = EclRestartFile( g , self.unrst_file )
        
        headers = f.headers( )
        self.assertEqual( len(headers) , 63 )
        
        with self.assertRaises(IndexError):
            f.get_header(1000)

        header = f.get_header( 10 )
        details = header.well_details( )
        self.assertTrue( "NXCONZ" in details )
        self.assertTrue( "NCWMAX" in details )
        
