from ert.test.extended_testcase import ExtendedTestCase
from .ert_shell_test_context import ErtShellTestContext


class ErtShellAnalysisModuleTest(ExtendedTestCase):

    def test_analysis_module(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtShellTestContext("python/ertshell/analysis_module", test_config) as shell:

            self.assertTrue(shell.invokeCommand("analysis_module current"))
            self.assertTrue(shell.invokeCommand("analysis_module list"))

            modules = shell.shellContext().ert().analysisConfig().getModuleList()

            for analysis_module in modules:
                self.assertTrue(shell.invokeCommand("analysis_module select %s" % analysis_module))
                active_module_name = shell.shellContext().ert().analysisConfig().activeModuleName()
                self.assertEqual(analysis_module, active_module_name)

                self.assertTrue(shell.invokeCommand("analysis_module variables"))

                analysis_module = shell.shellContext().ert().analysisConfig().getActiveModule()
                variable_names = analysis_module.getVariableNames()

                for variable_name in variable_names:
                    value = analysis_module.getVariableValue(variable_name)

                    variable_type = analysis_module.getVariableType(variable_name)

                    if variable_type is float:
                        new_value = value + 0.5
                    elif variable_type is int:
                        new_value = value + 2
                    elif variable_type is str:
                        new_value = "New String Value"
                    else:
                        new_value = not value

                    self.assertTrue(shell.invokeCommand("analysis_module set %s %s" % (variable_name, new_value)))

                    updated_value = analysis_module.getVariableValue(variable_name)

                    self.assertEqual(new_value, updated_value)

