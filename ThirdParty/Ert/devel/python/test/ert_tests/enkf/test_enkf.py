#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway.
#
#  The file 'test_enkf.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import EnkfStateEnum, EnsConfig, AnalysisConfig, ModelConfig, SiteConfig, EclConfig, PlotConfig, EnkfObs, ErtTemplates, EnkfFs, EnKFState
from ert.enkf.local_config import LocalConfig
from ert.enkf.enkf_main import EnKFMain
from ert.util.test_area import TestAreaContext
from ert_tests import ExtendedTestCase



class EnKFTest(ExtendedTestCase):
    def setUp(self):
        self.case_directory = self.createTestPath("local/simple_config/")
        self.site_config_file = os.getenv("ERT_SITE_CONFIG")


    def test_bootstrap( self ):
        with TestAreaContext("enkf_test", store_area=True) as work_area:
            work_area.copy_directory(self.case_directory)
            main = EnKFMain("simple_config/minimum_config", self.site_config_file)
            self.assertTrue(main, "Load failed")
            del main

    def test_enum(self):
        self.assertEqual(EnkfStateEnum.FORECAST, 2)
        self.assertEqual(EnkfStateEnum.ANALYZED, 4)

    def test_config( self ):
        with TestAreaContext("enkf_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config", self.site_config_file)

            self.assertIsInstance(main.ensemble_config(), EnsConfig)
            self.assertIsInstance(main.analysis_config(), AnalysisConfig)
            self.assertIsInstance(main.model_config(), ModelConfig)
            #self.assertIsInstance(main.local_config(), LocalConfig) #warn: Should this be None?
            self.assertIsInstance(main.site_config(), SiteConfig)
            self.assertIsInstance(main.ecl_config(), EclConfig)
            self.assertIsInstance(main.plot_config(), PlotConfig)

            # self.main.load_obs(obs_config_file)
            self.assertIsInstance(main.get_obs(), EnkfObs)
            self.assertIsInstance(main.get_templates(), ErtTemplates)
            self.assertIsInstance(main.get_fs(), EnkfFs)
            # self.assertIsInstance(main.iget_member_config(0), MemberConfig)
            self.assertIsInstance(main.iget_state(0), EnKFState)

            del main
