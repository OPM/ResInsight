from __future__ import print_function
import os
from ert.enkf.export import GenKwCollector
from ert.test.extended_testcase import ExtendedTestCase
from tests.gui.ertshell.ert_shell_test_context import ErtShellTestContext

class ErtShellSmootherTest(ExtendedTestCase):

    def test_smoother(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")
    
        with ErtShellTestContext("python/ertshell/smoother", test_config) as shell:
            print(os.getcwd())
            shell.invokeCommand("case select test_run")
    
            self.assertTrue(shell.invokeCommand("smoother update test_run_update"))
    
            shell.invokeCommand("case select test_run_update")
    
            ert = shell.shellContext().ert()
            data = GenKwCollector.loadAllGenKwData(ert, "test_run", keys=["PERLIN_PARAM:SCALE"])
            update_data = GenKwCollector.loadAllGenKwData(ert, "test_run_update", keys=["PERLIN_PARAM:SCALE"])
    
            self.assertTrue(data["PERLIN_PARAM:SCALE"].std() > update_data["PERLIN_PARAM:SCALE"].std())


    def test_config(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")
    
        with ErtShellTestContext("python/ertshell/smoother_config", test_config) as shell:
    
            analysis_config = shell.shellContext().ert().analysisConfig()
    
            self.assertTrue(shell.invokeCommand("smoother overlap_alpha"))
            self.assertTrue(shell.invokeCommand("smoother overlap_alpha 3.1415"))
            self.assertAlmostEqual(3.1415, analysis_config.getEnkfAlpha())
            self.assertFalse(shell.invokeCommand("smoother overlap_alpha threepointfourteen"))
    
            self.assertTrue(shell.invokeCommand("smoother std_cutoff"))
            self.assertTrue(shell.invokeCommand("smoother std_cutoff 0.1"))
            self.assertAlmostEqual(0.1, analysis_config.getStdCutoff())
            self.assertTrue(shell.invokeCommand("smoother std_cutoff -0.1"))
            self.assertAlmostEqual(0.0, analysis_config.getStdCutoff())
            self.assertFalse(shell.invokeCommand("smoother std_cutoff zeropointthreefourteen"))
    
    
            self.assertTrue(shell.invokeCommand("smoother global_std_scaling"))
            self.assertTrue(shell.invokeCommand("smoother global_std_scaling 0.5"))
            self.assertAlmostEqual(0.5, analysis_config.getGlobalStdScaling())
            self.assertTrue(shell.invokeCommand("smoother global_std_scaling -0.5"))
            self.assertAlmostEqual(0.0, analysis_config.getGlobalStdScaling())
            self.assertFalse(shell.invokeCommand("smoother global_std_scaling zeropointfour"))

