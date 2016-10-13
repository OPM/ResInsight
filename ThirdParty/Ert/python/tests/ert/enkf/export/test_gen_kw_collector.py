from ert.enkf.export import GenKwCollector
from ert.test import ErtTestContext, ExtendedTestCase


class GenKwCollectorTest(ExtendedTestCase):
    def setUp(self):
        self.config = self.createTestPath("local/snake_oil/snake_oil.ert")

    def test_gen_kw_collector(self):
        with ErtTestContext("python/enkf/export/gen_kw_collector", self.config) as context:
            ert = context.getErt()

            data = GenKwCollector.loadAllGenKwData(ert, "default_0")

            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_PERSISTENCE"][0], 0.047517)
            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_PERSISTENCE"][24], 0.160907)

            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_OFFSET"][0], 0.054539)
            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_OFFSET"][12], 0.057807)

            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_60 = data.loc[60]

            data = GenKwCollector.loadAllGenKwData(ert, "default_0", ["SNAKE_OIL_PARAM:OP1_PERSISTENCE", "SNAKE_OIL_PARAM:OP1_OFFSET"])

            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_PERSISTENCE"][0], 0.047517)
            self.assertFloatEqual(data["SNAKE_OIL_PARAM:OP1_OFFSET"][0], 0.054539)

            with self.assertRaises(KeyError):
                data["SNAKE_OIL_PARAM:OP1_DIVERGENCE_SCALE"]
