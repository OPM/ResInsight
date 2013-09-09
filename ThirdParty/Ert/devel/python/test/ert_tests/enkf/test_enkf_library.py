import os
from ert.ecl import EclSum
from ert.enkf import BlockObs, AnalysisConfig, EclConfig, GenKwConfig, EnkfConfigNode, SiteConfig, EnkfStateEnum
from ert.enkf import GenDataConfig, FieldConfig, EnkfFs, EnkfObs, EnKFState, EnsConfig
from ert.enkf import ErtTemplate, ErtTemplates, LocalConfig, ModelConfig, PlotConfig
from ert.enkf.enkf_main import EnKFMain

from ert.enkf.util import ObsVector, TimeMap
from ert.util.test_area import TestAreaContext
from ert_tests import ExtendedTestCase


class EnKFLibraryTest(ExtendedTestCase):
    def setUp(self):
        self.case_directory = self.createTestPath("local/simple_config/")
        self.site_config = os.getenv("ERT_SITE_CONFIG")


    def test_failed_class_creation(self):
        classes = [AnalysisConfig, BlockObs, FieldConfig, GenKwConfig, GenDataConfig,
                   EnkfConfigNode, EnkfFs, EnkfObs, TimeMap, ObsVector, EnKFState, EnsConfig,
                   ErtTemplate, ErtTemplates, LocalConfig, ModelConfig, PlotConfig, SiteConfig]

        for cls in classes:
            with self.assertRaises(NotImplementedError):
                temp = cls()


    def test_enums(self):
        print(EnkfStateEnum.enum_names)

    def test_ecl_config_creation(self):
        with self.assertRaises(NotImplementedError):
            ecl_config = EclConfig()

        with TestAreaContext("enkf_library_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config", self.site_config)

            self.assertIsInstance(main.analysis_config(), AnalysisConfig)
            self.assertIsInstance(main.ecl_config(), EclConfig)

            with self.assertRaises(AttributeError):
                self.assertIsInstance(main.ecl_config().get_refcase(), EclSum)

            time_map = main.get_fs().get_time_map()
            self.assertIsInstance(time_map, TimeMap)

            del main



