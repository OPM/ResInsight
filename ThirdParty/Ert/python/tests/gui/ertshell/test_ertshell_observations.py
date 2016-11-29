from ert.test import ExtendedTestCase
from tests.gui.ertshell.ert_shell_test_context import ErtShellTestContext

class ErtShellObservationsTest(ExtendedTestCase):

    def test_load_observations(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtShellTestContext("python/ertshell/observations", test_config) as shell:
            ert = shell.shellContext().ert()
            original_observation_count = len(ert.getObservations())

            self.assertTrue(shell.invokeCommand("observations list"))

            self.assertTrue(shell.invokeCommand("observations clear"))
            self.assertEqual(0, len(ert.getObservations()))

            self.assertTrue(shell.invokeCommand("observations load Observations/observation_1"))
            self.assertEqual(1, len(ert.getObservations()))

            self.assertTrue(shell.invokeCommand("observations load Observations/observation_2"))
            self.assertEqual(2, len(ert.getObservations()))

            self.assertTrue(shell.invokeCommand("observations load Observations/observation_3"))
            self.assertEqual(original_observation_count, len(ert.getObservations()))

            self.assertTrue(shell.invokeCommand("observations reload Observations/observation_1"))
            self.assertEqual(1, len(ert.getObservations()))
