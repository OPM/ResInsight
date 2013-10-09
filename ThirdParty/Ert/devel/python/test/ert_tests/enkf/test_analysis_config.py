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
from ert_tests import ExtendedTestCase


class AnalysisConfigTest(ExtendedTestCase):

    def test_keywords_for_monitoring_simulation_runtime(self):
        ac = AnalysisConfig()
        ac.set_min_realisations( 100 )
        self.assertEqual( 100 , ac.get_min_realisations() )
        
        ac.set_max_runtime( 50 )
        self.assertEqual( 50 , ac.get_max_runtime() )

        ac.set_stop_long_running( True )
        self.assertTrue( ac.get_stop_long_running() )


    def test_analysis_modules(self):
        ac = AnalysisConfig()
        self.assertIsNone( ac.activeModuleName() )
       
