# !/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway.
#   
#  The file 'test_ecl_sum.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.ecl import EclSum
from ert.test import ExtendedTestCase

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

class EclSumTest(ExtendedTestCase):
    def setUp(self):
        self.test_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.SMSPEC")
        self.ecl_sum = EclSum(self.test_file)

    def test_time_range_year(self):
        real_range = self.ecl_sum.timeRange(interval="1y", extend_end = False)
        extended_range = self.ecl_sum.timeRange(interval="1y", extend_end = True)
        assert real_range[-1] < extended_range[-1]

    def test_time_range_day(self):
        real_range = self.ecl_sum.timeRange(interval = "1d", extend_end = False)
        extended_range = self.ecl_sum.timeRange(interval = "1d", extend_end = True)
        assert real_range[-1] == extended_range[-1]

    def test_time_range_month(self):
        real_range = self.ecl_sum.timeRange(interval = "1m", extend_end = False)
        extended_range = self.ecl_sum.timeRange(interval = "1m", extend_end = True)
        assert real_range[-1] < extended_range[-1]