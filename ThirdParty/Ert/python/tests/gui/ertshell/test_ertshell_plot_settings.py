from ert.test import ExtendedTestCase
from tests.gui.ertshell.ert_shell_test_context import ErtShellTestContext


class ErtShellPlotSettingsTest(ExtendedTestCase):
    def checkStringProperties(self, shell, command, testFunction, allow_multiple_arguments=True):
        self.assertTrue(shell.invokeCommand(command))

        spacey_string = "new value with spaces"
        if allow_multiple_arguments:
            self.assertTrue(shell.invokeCommand("%s %s" % (command, spacey_string)))
            self.assertEqual(testFunction(), spacey_string)
        else:
            self.assertFalse(shell.invokeCommand("%s %s" % (command, spacey_string)))
            self.assertNotEqual(testFunction(), spacey_string)

        no_spacey_string = "new_value_without_spaces"
        self.assertTrue(shell.invokeCommand("%s %s" % (command, no_spacey_string)))
        self.assertEqual(testFunction(), no_spacey_string)


    def checkBoolProperties(self, shell, command, testFunction):
        start_value = testFunction()
        self.assertTrue(shell.invokeCommand(command))
        self.assertTrue(shell.invokeCommand("%s %s" % (command, str(not start_value))))
        self.assertNotEqual(start_value, testFunction())
        self.assertFalse(shell.invokeCommand("%s not_a_bool" % command))


    def test_style_setting(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")
        with ErtShellTestContext("python/ertshell/plot_settings", test_config, load_config=True) as shell:
            plot_config = shell.shellContext()["plot_settings"].plotConfig()

            self.assertTrue(shell.invokeCommand("help plot_settings"))

            self.checkStringProperties(shell, "plot_settings title", plot_config.title)
            self.checkStringProperties(shell, "plot_settings x_label", plot_config.xLabel)
            self.checkStringProperties(shell, "plot_settings y_label", plot_config.yLabel)

            self.checkBoolProperties(shell, "plot_settings grid", plot_config.isGridEnabled)
            self.checkBoolProperties(shell, "plot_settings legend", plot_config.isLegendEnabled)
            self.checkBoolProperties(shell, "plot_settings refcase", plot_config.isRefcaseEnabled)
            self.checkBoolProperties(shell, "plot_settings observations", plot_config.isObservationsEnabled)

            self.checkStringProperties(shell, "plot_settings path", shell.shellContext()["plot_settings"].getPath, allow_multiple_arguments=False)

            default_plot_cases = shell.shellContext()["plot_settings"].getCurrentPlotCases()
            self.assertTrue(shell.invokeCommand("plot_settings current"))
            self.assertTrue(shell.invokeCommand("plot_settings select test_run default"))

            plot_cases = shell.shellContext()["plot_settings"].getCurrentPlotCases()
            self.assertSetEqual(set(plot_cases), {"test_run", "default"})

            self.assertTrue(shell.invokeCommand("plot_settings select"))
            plot_cases = shell.shellContext()["plot_settings"].getCurrentPlotCases()
            self.assertListEqual(plot_cases, default_plot_cases)

            self.assertFalse(shell.invokeCommand("plot_settings select unknown"))
            self.assertFalse(shell.invokeCommand("plot_settings select unknown1 unknown2"))
