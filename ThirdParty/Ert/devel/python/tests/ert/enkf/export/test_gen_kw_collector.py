from ert.enkf.export import GenKwCollector
from ert.test import ErtTestContext, ExtendedTestCase

class GenKwCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")


    def test_gen_kw_collector(self):

        with ErtTestContext("python/enkf/export/gen_kw_collector", self.config) as context:
            ert = context.getErt()

            data = GenKwCollector.loadAllGenKwData(ert, "default")

            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][0], 0.018466)
            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][24], 0.221049)

            self.assertFloatEqual(data["GRID_PARAMS:MULTPV3"][0], 2.227307)
            self.assertFloatEqual(data["GRID_PARAMS:MULTPV3"][12], 5.899703)

            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][0], -3.008949)
            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][24], -1.051446)

            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_60 = data.loc[60]



            data = GenKwCollector.loadAllGenKwData(ert, "default", ["FLUID_PARAMS:SGCR", "LOG10_MULTFLT:F3"])

            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][0], 0.018466)
            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][0], -3.008949)

            with self.assertRaises(KeyError):
                data["GRID_PARAMS:MULTPV3"]
