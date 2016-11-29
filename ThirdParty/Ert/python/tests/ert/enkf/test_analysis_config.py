#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_analysis_config.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import AnalysisConfig
from ert.test import ExtendedTestCase


class AnalysisConfigTest(ExtendedTestCase):

    def test_keywords_for_monitoring_simulation_runtime(self):
        ac = AnalysisConfig()

        # Unless the MIN_REALIZATIONS is set in config, one is required to have "all" realizations.
        self.assertFalse(ac.haveEnoughRealisations(5, 10))
        self.assertTrue(ac.haveEnoughRealisations(10, 10))

        ac.set_max_runtime( 50 )
        self.assertEqual( 50 , ac.get_max_runtime() )

        ac.set_stop_long_running( True )
        self.assertTrue( ac.get_stop_long_running() )


    def test_analysis_modules(self):
        ac = AnalysisConfig()
        self.assertIsNone( ac.activeModuleName() )
        self.assertIsNotNone( ac.getModuleList() )

    def test_analysis_config_global_std_scaling(self):
        ac = AnalysisConfig()
        self.assertFloatEqual(ac.getGlobalStdScaling(), 1.0)
        ac.setGlobalStdScaling(0.77)
        self.assertFloatEqual(ac.getGlobalStdScaling(), 0.77)
