#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_rft.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.ecl import EclRFTFile, EclRFTCell, EclPLTCell
from ert_tests import ExtendedTestCase


class RFTTest(ExtendedTestCase):
    def setUp(self):
        self.RFT_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.RFT")
        self.PLT_file = self.createTestPath("Statoil/ECLIPSE/RFT/TEST1_1A.RFT")


    def test_RFT_load( self ):
        rftFile = EclRFTFile(self.RFT_file)

        rft = rftFile[0]
        cell = rft.ijkget((32, 53, 0))
        self.assertIsInstance(cell, EclRFTCell)

        self.assertEqual(2, rftFile.size())
        self.assertEqual(0, rftFile.size(well="OP*"))
        self.assertEqual(0, rftFile.size(well="XXX"))
        self.assertEqual(1, rftFile.size(date=datetime.date(2000, 6, 1)))
        self.assertEqual(0, rftFile.size(date=datetime.date(2000, 6, 17)))

        cell = rft.ijkget((30, 20, 1880))
        self.assertIsNone(cell)

        for rft in rftFile:
            self.assertTrue(rft.is_RFT())
            self.assertFalse(rft.is_SEGMENT())
            self.assertFalse(rft.is_PLT())
            self.assertFalse(rft.is_MSW())

            for cell in rft:
                self.assertIsInstance(cell, EclRFTCell)

            cell0 = rft.iget_sorted(0)
            self.assertIsInstance(cell, EclRFTCell)
            rft.sort()


    def test_PLT_load( self ):
        pltFile = EclRFTFile(self.PLT_file)
        plt = pltFile[11]
        self.assertTrue(plt.is_PLT())
        self.assertFalse(plt.is_SEGMENT())
        self.assertFalse(plt.is_RFT())
        self.assertFalse(plt.is_MSW())

        for cell in plt:
            self.assertIsInstance(cell, EclPLTCell)


    def test_exceptions( self ):
        with self.assertRaises(IndexError):
            rftFile = EclRFTFile(self.RFT_file)
            rft = rftFile[100]
