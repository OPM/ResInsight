#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_rft_cell.py' is part of ERT - Ensemble based Reservoir Tool.
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
from unittest2 import TestCase
from ert.ecl import EclTypeEnum, EclFileFlagEnum, EclPhaseEnum

import ert.ecl.ecl as ecl
import ert.ecl as new_ecl
import ert.ecl.ecl_default as ecl_default


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

        self.assertEqual(ecl.ecl_default.default.ecl_version, new_ecl.EclDefault.ecl_version())

        self.assertEqual(ecl.EclQueue, new_ecl.EclQueue)

    def test_enums(self):
        self.assertEqual(ecl.ECL_FLOAT_TYPE, EclTypeEnum.ECL_FLOAT_TYPE)
        self.assertEqual(ecl.ECL_CHAR_TYPE, EclTypeEnum.ECL_CHAR_TYPE)
        self.assertEqual(ecl.ECL_DOUBLE_TYPE, EclTypeEnum.ECL_DOUBLE_TYPE)
        self.assertEqual(ecl.ECL_BOOL_TYPE, EclTypeEnum.ECL_BOOL_TYPE)
        self.assertEqual(ecl.ECL_CHAR_TYPE, EclTypeEnum.ECL_CHAR_TYPE)

        self.assertEqual(ecl.ECL_FILE_WRITABLE, EclFileFlagEnum.ECL_FILE_WRITABLE)

        self.assertEqual(ecl.ECL_OIL_PHASE, EclPhaseEnum.ECL_OIL_PHASE)
        self.assertEqual(ecl.ECL_GAS_PHASE, EclPhaseEnum.ECL_GAS_PHASE)
        self.assertEqual(ecl.ECL_WATER_PHASE, EclPhaseEnum.ECL_WATER_PHASE)



    def test_ecl_defaults(self):
        self.assertIsNotNone(ecl_default.default.ecl_version)
        self.assertIsNotNone(ecl_default.default.driver_options)
        self.assertIsNotNone(ecl_default.default.ecl_cmd)
        self.assertIsNotNone(ecl_default.default.driver_type)
        self.assertIsNotNone(ecl_default.default.lsf_resource_request)