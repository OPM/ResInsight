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
from ert.enkf import EnsConfig, AnalysisConfig, ModelConfig, SiteConfig, EclConfig, PlotConfig, EnkfObs, ErtTemplates, EnkfFs, EnKFState, EnkfStateType, EnkfRunEnum, EnkfVarType, ObsVector
from ert.enkf.data import EnkfConfigNode
from ert.enkf.enkf_main import EnKFMain
from ert.enkf.enums import EnkfObservationImplementationType, LoadFailTypeEnum
from ert.enkf.observations.summary_observation import SummaryObservation
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

        self.assertEnumIsFullyDefined(EnkfVarType, "enkf_var_type", "libenkf/include/ert/enkf/enkf_types.h")
        self.assertEnumIsFullyDefined(EnkfStateType, "state_enum", "libenkf/include/ert/enkf/enkf_types.h")
        self.assertEnumIsFullyDefined(EnkfObservationImplementationType, "obs_impl_type", "libenkf/include/ert/enkf/obs_vector.h")
        self.assertEnumIsFullyDefined(LoadFailTypeEnum, "load_fail_type", "libenkf/include/ert/enkf/summary_config.h")

        self.assertEqual(EnkfRunEnum.ENKF_ASSIMILATION, 1)
        self.assertEqual(EnkfRunEnum.ENSEMBLE_EXPERIMENT, 2)


    def test_observations(self):
        with TestAreaContext("enkf_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config", self.site_config_file)

            count = 10
            summary_key = "test_key"
            observation_key = "test_obs_key"
            summary_observation_node = EnkfConfigNode.createSummaryConfigNode(summary_key, LoadFailTypeEnum.LOAD_FAIL_EXIT)
            observation_vector = ObsVector(EnkfObservationImplementationType.SUMMARY_OBS, observation_key, summary_observation_node, count)

            main.getObservations().addObservationVector(observation_key, observation_vector)

            values = []
            for index in range(0, count):
                value = index * 10.5
                std = index / 10.0
                summary_observation_node = SummaryObservation(summary_key, observation_key, value, std)
                observation_vector.installNode(index, summary_observation_node)
                self.assertEqual(observation_vector.getNode(index), summary_observation_node)
                self.assertEqual(value, summary_observation_node.getValue())
                values.append((index, value, std))


            observations = main.getObservations()
            test_vector = observations.getObservationsVector(observation_key)

            self.assertEqual(observation_vector, test_vector)
            for index, value, std in values:
                self.assertTrue(test_vector.isActive(index))

                summary_observation_node = test_vector.getNode(index)
                """@type: SummaryObservation"""

                self.assertEqual(value, summary_observation_node.getValue())
                self.assertEqual(std, summary_observation_node.getStandardDeviation())
                self.assertEqual(summary_key, summary_observation_node.getSummaryKey())






    def test_config( self ):
        with TestAreaContext("enkf_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config", self.site_config_file)

            self.assertIsInstance(main.ensembleConfig(), EnsConfig)
            self.assertIsInstance(main.analysis_config(), AnalysisConfig)
            self.assertIsInstance(main.getModelConfig(), ModelConfig)
            #self.assertIsInstance(main.local_config(), LocalConfig) #warn: Should this be None?
            self.assertIsInstance(main.siteConfig(), SiteConfig)
            self.assertIsInstance(main.ecl_config(), EclConfig)
            self.assertIsInstance(main.plot_config(), PlotConfig)

            # self.main.load_obs(obs_config_file)
            self.assertIsInstance(main.getObservations(), EnkfObs)
            self.assertIsInstance(main.get_templates(), ErtTemplates)
            self.assertIsInstance(main.getFileSystem(), EnkfFs)
            # self.assertIsInstance(main.iget_member_config(0), MemberConfig)
            self.assertIsInstance(main.getMemberRunningState(0), EnKFState)

            del main
