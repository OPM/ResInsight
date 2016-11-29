from ert.enkf.export import MisfitCollector
from ert.test import ErtTestContext, ExtendedTestCase


class MisfitCollectorTest(ExtendedTestCase):
    def setUp(self):
        self.config = self.createTestPath("local/snake_oil/snake_oil.ert")

    def test_misfit_collector(self):
        with ErtTestContext("python/enkf/export/misfit_collector", self.config) as context:
            ert = context.getErt()
            data = MisfitCollector.loadAllMisfitData(ert, "default_0")

            self.assertFloatEqual(data["MISFIT:FOPR"][0], 798.378619)
            self.assertFloatEqual(data["MISFIT:FOPR"][24], 1332.219633)

            self.assertFloatEqual(data["MISFIT:TOTAL"][0], 826.651491)
            self.assertFloatEqual(data["MISFIT:TOTAL"][24], 1431.305646)

            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_60 = data.loc[60]
