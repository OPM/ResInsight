from ert.job_queue import ErtScript
from ert.test import ErtTestContext, ExtendedTestCase


class MDAEnsembleSmootherPluginTest(ExtendedTestCase):
    def getPlugin(self, ert, job_name):
        mda_es = self.createSharePath("workflows/jobs/internal-gui/config/%s" % job_name)

        ert.getWorkflowList().addJob("TEST_MDA_ES", mda_es)
        plugin_job = ert.getWorkflowList().getJob("TEST_MDA_ES")

        self.assertIsNotNone(plugin_job)

        script_obj = ErtScript.loadScriptFromFile(plugin_job.getInternalScriptPath())
        script = script_obj(ert)
        return script


    def test_weights(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtTestContext("python/workflow_jobs/mda_es_weights", test_config) as test_context:
            ert = test_context.getErt()
            plugin = self.getPlugin(ert, "MDA_ES")

            weights = plugin.parseWeights("iteration_weights/constant_4")
            self.assertAlmostEqualList([2, 2, 2, 2], weights)

            weights = plugin.parseWeights("iteration_weights/constant_2")
            self.assertAlmostEqualList([1.414213562373095, 1.414213562373095], weights)

            with self.assertRaises(ValueError):
                plugin.parseWeights("iteration_weights/error_in_weights")

            with self.assertRaises(ValueError):
                plugin.parseWeights("")

            weights = plugin.parseWeights("2, 2, 2, 2")
            self.assertAlmostEqualList([2, 2, 2, 2], weights)

            weights = plugin.parseWeights("1.414213562373095, 1.414213562373095")
            self.assertAlmostEqualList([1.414213562373095, 1.414213562373095], weights)

            with self.assertRaises(ValueError):
                plugin.parseWeights("2, error, 2, 2")


    def test_normalized_weights(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtTestContext("python/workflow_jobs/mda_es_weights_normalized", test_config) as test_context:
            ert = test_context.getErt()
            plugin = self.getPlugin(ert, "MDA_ES")
            weights = plugin.normalizeWeights([1])
            self.assertAlmostEqualList([1.0], weights)

            weights = plugin.normalizeWeights([1, 1])
            self.assertAlmostEqualList([1.414214, 1.414214], weights)

            weights = plugin.normalizeWeights([1, 1, 1])
            self.assertAlmostEqualList([1.732051, 1.732051, 1.732051], weights)

            weights = plugin.normalizeWeights([8, 4, 2, 1])
            self.assertAlmostEqualList([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109], weights)

            weights = plugin.normalizeWeights([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109])
            self.assertAlmostEqualList([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109], weights)
