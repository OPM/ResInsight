#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from ert.ecl import EclTypeEnum, EclFileFlagEnum, EclPhaseEnum

import ert.ecl.ecl as ecl
import ert.ecl as new_ecl

class LegacyEclTest(TestCase):

    def test_classes(self):
        self.assertEqual(ecl.EclSum, new_ecl.EclSum)

        self.assertEqual(ecl.EclRFTFile, new_ecl.EclRFTFile)
        self.assertEqual(ecl.EclRFTCell, new_ecl.EclRFTCell)
        self.assertEqual(ecl.EclPLTCell, new_ecl.EclPLTCell)

        self.assertEqual(ecl.EclKW, new_ecl.EclKW)
        self.assertEqual(ecl.EclFile, new_ecl.EclFile)

        self.assertEqual(ecl.FortIO, new_ecl.FortIO)

        self.assertEqual(ecl.EclGrid, new_ecl.EclGrid)

        self.assertEqual(ecl.EclRegion, new_ecl.EclRegion)
