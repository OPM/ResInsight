#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'test_region.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.ecl import EclFile, EclGrid, EclRegion
from ert_tests import ExtendedTestCase


class RegionTest(ExtendedTestCase):
    def setUp(self):
        case = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE")
        self.grid = EclGrid(case)
        self.rst_file = EclFile("%s.UNRST" % case)
        self.init_file = EclFile("%s.INIT" % case)


    def test_kw_imul(self):
        P = self.rst_file["PRESSURE"][5]
        fipnum = self.init_file["FIPNUM"][0]
        fipnum_copy = fipnum.deep_copy()

        reg = EclRegion(self.grid, False)
        reg.select_more(P, 260)
        fipnum.mul(-1, mask=reg)
        self.assertFalse(fipnum.equal(fipnum_copy))

        fipnum.mul(-1, mask=reg)
        self.assertTrue(fipnum.equal(fipnum_copy))


    def test_kw_idiv(self):
        P = self.rst_file["PRESSURE"][5]
        fipnum = self.init_file["FIPNUM"][0]
        fipnum_copy = fipnum.deep_copy()

        reg = EclRegion(self.grid, False)
        reg.select_more(P, 260)
        fipnum.div(-1, mask=reg)
        self.assertFalse(fipnum.equal(fipnum_copy))

        fipnum.div(-1, mask=reg)
        self.assertTrue(fipnum.equal(fipnum_copy))


    def test_kw_iadd(self):
        P = self.rst_file["PRESSURE"][5]
        fipnum = self.init_file["FIPNUM"][0]
        fipnum_copy = fipnum.deep_copy()

        reg = EclRegion(self.grid, False)
        reg.select_more(P, 260)
        fipnum.add(1, mask=reg)
        self.assertFalse(fipnum.equal(fipnum_copy))

        reg.invert()
        fipnum.add(1, mask=reg)

        fipnum.sub(1)
        self.assertTrue(fipnum.equal(fipnum_copy))


    def test_kw_isub(self):
        P = self.rst_file["PRESSURE"][5]
        fipnum = self.init_file["FIPNUM"][0]
        fipnum_copy = fipnum.deep_copy()

        reg = EclRegion(self.grid, False)
        reg.select_more(P, 260)
        fipnum.sub(1, mask=reg)
        self.assertFalse(fipnum.equal(fipnum_copy))
        fipnum.add(1, mask=reg)
        self.assertTrue(fipnum.equal(fipnum_copy))


    def test_slice(self):
        reg = EclRegion(self.grid, False)
        reg.select_islice(0, 5)
        OK = True

        global_list = reg.global_list
        self.assertEqual(global_list.parent(), reg)

        for gi in global_list:
            (i, j, k) = self.grid.get_ijk(global_index=gi)
            if i > 5:
                OK = False
        self.assertTrue(OK)
        self.assertTrue(self.grid.ny * self.grid.nz * 6 == len(reg.global_list))

        reg.select_jslice(7, 8, intersect=True)
        OK = True
        for gi in reg.global_list:
            (i, j, k) = self.grid.get_ijk(global_index=gi)
            if i > 5:
                OK = False

            if j < 7 or j > 8:
                OK = False

        self.assertTrue(OK)
        self.assertTrue(2 * self.grid.nz * 6 == len(reg.global_list))

        reg2 = EclRegion(self.grid, False)
        reg2.select_kslice(3, 5)
        reg &= reg2
        OK = True
        for gi in reg.global_list:
            (i, j, k) = self.grid.get_ijk(global_index=gi)
            if i > 5:
                OK = False

            if j < 7 or j > 8:
                OK = False

            if k < 3 or k > 5:
                OK = False

        self.assertTrue(OK)
        self.assertTrue(2 * 3 * 6 == len(reg.global_list))


