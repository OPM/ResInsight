from ert.enkf.export import GenKwCollector
from ert.test import ErtTestContext, ExtendedTestCase

class GenKwCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")


    def test_gen_kw_collector(self):

        with ErtTestContext("python/enkf/export/gen_kw_collector", self.config) as context:
            ert = context.getErt()

            data = GenKwCollector.loadAllGenKwData(ert, "default")

            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][0], 0.295136)
            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][24], 0.177833)

            self.assertFloatEqual(data["GRID_PARAMS:MULTPV3"][0], 0.423297)
            self.assertFloatEqual(data["GRID_PARAMS:MULTPV3"][12], 2.278845)

            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][0], -2.742916)
            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][24], -3.459867)

            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_21 = data.loc[21]



            data = GenKwCollector.loadAllGenKwData(ert, "default", ["FLUID_PARAMS:SGCR", "LOG10_MULTFLT:F3"])

            self.assertFloatEqual(data["FLUID_PARAMS:SGCR"][0], 0.295136)
            self.assertFloatEqual(data["LOG10_MULTFLT:F3"][0], -2.742916)

            with self.assertRaises(KeyError):
                data["GRID_PARAMS:MULTPV3"]