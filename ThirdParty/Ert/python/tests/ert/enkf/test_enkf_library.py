import os
from ert.ecl import EclSum
from ert.enkf import AnalysisConfig, EclConfig, GenKwConfig, EnkfConfigNode, SiteConfig, ObsVector
from ert.enkf import GenDataConfig, FieldConfig, EnkfFs, EnkfObs, EnKFState, EnsembleConfig
from ert.enkf import ErtTemplate, ErtTemplates, LocalConfig, ModelConfig, PlotConfig
from ert.enkf.enkf_main import EnKFMain

from ert.enkf.util import TimeMap
from ert.test import ExtendedTestCase , TestAreaContext


class EnKFLibraryTest(ExtendedTestCase):
    def setUp(self):
        self.case_directory = self.createTestPath("local/simple_config/")

    def test_failed_class_creation(self):
        classes = [EnkfConfigNode, EnKFState,
                   ErtTemplate, ErtTemplates, LocalConfig, ModelConfig, PlotConfig, SiteConfig]

        for cls in classes:
            with self.assertRaises(NotImplementedError):
                temp = cls()


    def test_ecl_config_creation(self):
        with TestAreaContext("enkf_library_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config")

            self.assertIsInstance(main.analysisConfig(), AnalysisConfig)
            self.assertIsInstance(main.eclConfig(), EclConfig)

            with self.assertRaises(AssertionError): # Null pointer!
                self.assertIsInstance(main.eclConfig().getRefcase(), EclSum)

            file_system = main.getEnkfFsManager().getCurrentFileSystem()
            self.assertEqual(file_system.getCaseName(), "default")
            time_map = file_system.getTimeMap()
            self.assertIsInstance(time_map, TimeMap)

            main.free()


    def test_enkf_state(self):
        with TestAreaContext("enkf_library_test") as work_area:
            work_area.copy_directory(self.case_directory)

            main = EnKFMain("simple_config/minimum_config")
            state = main.getRealisation( 0 )
            
            with self.assertRaises(TypeError):
                state.addSubstKeyword( "GEO_ID" , 45)
            
            state.addSubstKeyword("GEO_ID" , "45")
            
