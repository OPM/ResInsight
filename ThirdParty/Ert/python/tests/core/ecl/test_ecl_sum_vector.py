#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'test_ecl_sum_vector.py' is part of ERT - Ensemble based Reservoir Tool. 
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

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import warnings

from ert.ecl import EclSumVector, EclSum
from ert.test import ExtendedTestCase


class EclSumVectorTest(ExtendedTestCase):
    def setUp(self):
        self.test_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.SMSPEC")
        self.ecl_sum = EclSum(self.test_file)

    def test_reportOnly_warns(self):
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")

            vector = EclSumVector(self.ecl_sum, "FOPT", True)
            assert len(w) == 1
            assert issubclass(w[-1].category, DeprecationWarning)

    def test_basic(self):
        self.assertEqual(512, len(self.ecl_sum.keys()))
        pfx = 'EclSum(name'
        self.assertEqual(pfx, repr(self.ecl_sum)[:len(pfx)])
        it = iter(self.ecl_sum)
        t = self.ecl_sum[it.next()] # EclSumVector
        self.assertEqual(63, len(t))
        self.assertEqual('BARSA', t.unit)
        pfx = 'EclSumVector(key = '
        self.assertEqual(pfx, repr(t)[:len(pfx)])
