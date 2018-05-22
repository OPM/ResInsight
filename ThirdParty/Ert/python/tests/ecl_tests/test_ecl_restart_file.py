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

from tests import EclTest, statoil_test
from ecl import EclFileFlagEnum
from ecl.eclfile import Ecl3DKW , EclKW, EclRestartFile , EclFile, FortIO
from ecl.grid import EclGrid

@statoil_test()
class RestartFileTest(EclTest):
    def setUp(self):
        self.grid_file =   self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")
        self.unrst_file   = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.xrst_file0   = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0000")
        self.xrst_file10  = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0010")
        self.xrst_file20  = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0020")


    def test_load(self):
        g = EclGrid( self.grid_file )
        f = EclRestartFile( g , self.unrst_file )

        head = f["INTEHEAD"][0]
        self.assertTrue( isinstance( head , EclKW ))

        swat = f["SWAT"][0]
        self.assertTrue( isinstance( swat , Ecl3DKW ))

        pressure = f["PRESSURE"][0]
        self.assertTrue( isinstance( pressure , Ecl3DKW ))


    def test_type(self):
        g = EclGrid( self.grid_file )
        with self.assertRaises(ValueError):
            f = EclRestartFile( g , "NOT_A_RESTART_FILE")


    def test_unified(self):
        g = EclGrid( self.grid_file )
        f_unrst = EclRestartFile( g , self.unrst_file )
        f_x0 = EclRestartFile( g , self.xrst_file0 )
        f_x10 = EclRestartFile( g , self.xrst_file10 )
        f_x20 = EclRestartFile( g , self.xrst_file20 )

        self.assertTrue( f_unrst.unified() )
        self.assertFalse( f_x0.unified() )
        self.assertFalse( f_x10.unified() )
        self.assertFalse( f_x20.unified() )

        self.assertEqual( [(10 , datetime.datetime( 2000 , 10 , 1 , 0 , 0 , 0 ) , 274.0)] , f_x10.timeList())

        unrst_timeList = f_unrst.timeList()
        self.assertEqual( len(unrst_timeList) , 63 )
        self.assertEqual( (62 , datetime.datetime( 2004 , 12 , 31 , 0 , 0 , 0 ) , 1826.0) , unrst_timeList[62]);

