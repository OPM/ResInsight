from ert.enkf.export import CustomKWCollector
from ert.test import ErtTestContext, ExtendedTestCase

class CustomKwCollectorTest(ExtendedTestCase):

    def test_custom_kw_collector_non_current_fs(self):
        config = self.createTestPath("local/custom_kw/mini_config")

        with ErtTestContext("python/enkf/export/custom_kw_collector_from_fs", config) as context:
            ert = context.getErt()

            data = CustomKWCollector.loadAllCustomKWData(ert, "test_run")

            self.assertTrue(len(data.columns) == 0)


    def test_custom_kw_collector_current_fs(self):
        config = self.createTestPath("local/custom_kw/mini_config")

        with ErtTestContext("python/enkf/export/custom_kw_collector_from_current_fs", config) as context:
            ert = context.getErt()
            fs = ert.getEnkfFsManager().getFileSystem("test_run")
            ert.getEnkfFsManager().switchFileSystem(fs)
            data = CustomKWCollector.loadAllCustomKWData(ert, "test_run")

            self.assertFloatEqual(data["AGGREGATED:PERLIN_1"][0], -0.167794)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_1"][8], -1.276058)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_1"][9], -0.137903)

            self.assertFloatEqual(data["AGGREGATED:PERLIN_2"][0], 1.00263)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_2"][8], -0.105634)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_2"][9], 1.032522)

            self.assertFloatEqual(data["AGGREGATED:PERLIN_3"][0], 0.190479)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_3"][8], -0.917785)
            self.assertFloatEqual(data["AGGREGATED:PERLIN_3"][9], 0.220371)

            self.assertEqual(data["AGGREGATED:STATE"][0], "Positive")
            self.assertEqual(data["AGGREGATED:STATE"][8], "Negative")
            self.assertEqual(data["AGGREGATED:STATE"][9], "Positive")
