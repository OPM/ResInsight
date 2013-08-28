#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'kw_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.ecl import FortIO, EclTypeEnum, EclKW
from ert_tests import ExtendedTestCase
from ert.util import TestAreaContext




class FortIOTest(ExtendedTestCase):
    def setUp(self):
        self.unrst_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.file_list = []


    def test_open_read(self):
        f = FortIO.reader(self.unrst_file)
        self.assertTrue(f)


    def test_open_write(self):
        with TestAreaContext("python/fortio/write"):
            f = FortIO.writer("newfile")
            f.close()
            self.assertTrue(True)

    def test_noex(self):
        with self.assertRaises(IOError):
            f = FortIO.reader("/tmp/does/notExist")

    def test_kw(self):
        kw1 = EclKW.create("KW1", 2, EclTypeEnum.ECL_INT_TYPE)
        kw2 = EclKW.create("KW2", 2, EclTypeEnum.ECL_INT_TYPE)

        kw1[0] = 99
        kw1[1] = 77
        kw2[0] = 113
        kw2[1] = 335

        with TestAreaContext("python/fortio/write-kw"):
            f = FortIO.writer("test", fmt_file=False)
            kw1.fwrite(f)
            f.close()

            f = FortIO.open("test", mode="a")
            kw2.fwrite(f)
            f.close()

            f = FortIO.open("test", fmt_file=False)
            k1 = EclKW.fread(f)
            k2 = EclKW.fread(f)
            f.close()

            self.assertTrue(k1.equal(kw1))
            self.assertTrue(k2.equal(kw2))



