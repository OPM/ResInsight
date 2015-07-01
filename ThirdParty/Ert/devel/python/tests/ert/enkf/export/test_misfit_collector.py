from ert.enkf.export import MisfitCollector
from ert.test import ErtTestContext, ExtendedTestCase

class MisfitCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")

    def test_misfit_collector(self):

        with ErtTestContext("python/enkf/export/misfit_collector", self.config) as context:
            ert = context.getErt()
            data = MisfitCollector.loadAllMisfitData(ert, "default")

            self.assertFloatEqual(data["MISFIT:WWCT:OP_2"][0], 0.617793)
            self.assertFloatEqual(data["MISFIT:WWCT:OP_2"][24], 0.256436)

            self.assertFloatEqual(data["MISFIT:TOTAL"][0],7236.322836)
            self.assertFloatEqual(data["MISFIT:TOTAL"][24], 2261.726621)


            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_60 = data.loc[60]

