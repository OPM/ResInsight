#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'test_analysis_iter_config.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import AnalysisIterConfig
from ert.test import ExtendedTestCase


class AnalysisIterConfigTest(ExtendedTestCase):

    def test_set(self):
        c = AnalysisIterConfig()
        
        self.assertFalse( c.caseFormatSet() )
        c.setCaseFormat("case%d")
        self.assertTrue( c.caseFormatSet() )

        self.assertFalse( c.numIterationsSet() )
        c.setNumIterations(1)
        self.assertTrue( c.numIterationsSet() )

        

