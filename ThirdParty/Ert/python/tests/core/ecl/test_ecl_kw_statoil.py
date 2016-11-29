#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_kw.py' is part of ERT - Ensemble based Reservoir Tool.
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
import random
from ert.ecl import EclKW, EclTypeEnum, EclFile, FortIO, EclFileFlagEnum

from ert.test import ExtendedTestCase , TestAreaContext


def copy_long():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(0, 2000)


def copy_offset():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(200, 100)


class KWTest(ExtendedTestCase):
    def test_fortio_size( self ):
        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        size = 0
        for kw in unrst_file:
            size += kw.fortio_size

        stat = os.stat(unrst_file_path)
        self.assertTrue(size == stat.st_size)




    def test_sub_copy(self):
        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        swat = unrst_file["SWAT"][0]

        swat1 = swat.sub_copy(0, -1)
        swat2 = swat.sub_copy(0, swat.size)

        self.assertTrue(swat.equal(swat1))
        self.assertTrue(swat.equal(swat2))

        swat3 = swat.sub_copy(20000, 100, new_header="swat")
        self.assertTrue(swat3.name == "swat")
        self.assertTrue(swat3.size == 100)
        equal = True
        for i in range(swat3.size):
            if swat3[i] != swat[i + 20000]:
                equal = False
        self.assertTrue(equal)

        self.assertRaises(IndexError, copy_long)
        self.assertRaises(IndexError, copy_offset)


    def test_equal(self):
        kw1 = EclKW("TEST", 3, EclTypeEnum.ECL_CHAR_TYPE)
        kw1[0] = "Test1"
        kw1[1] = "Test13"
        kw1[2] = "Test15"

        kw2 = EclKW("TEST", 3, EclTypeEnum.ECL_CHAR_TYPE)
        kw2[0] = "Test1"
        kw2[1] = "Test13"
        kw2[2] = "Test15"

        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw2[2] = "Test15X"
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2))

        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        kw1 = unrst_file["PRESSURE"][0]
        kw2 = kw1.deep_copy()

        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw2 *= 1.00001
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2, epsilon=1e-8))
        self.assertTrue(kw1.equal_numeric(kw2, epsilon=1e-2))

        kw1 = unrst_file["ICON"][10]
        kw2 = kw1.deep_copy()
        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw1[-1] += 1
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2))

            
