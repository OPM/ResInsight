from ert.enkf.export import MisfitCollector
from ert.test import ErtTestContext, ExtendedTestCase

class MisfitCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")

    def test_misfit_collector(self):

        with ErtTestContext("python/enkf/export/misfit_collector", self.config) as context:
            ert = context.getErt()
            data = MisfitCollector.loadAllMisfitData(ert, "default")

            self.assertFloatEqual(data["MISFIT:WWCT:OP_2"][0], 3.552157351038322)
            self.assertFloatEqual(data["MISFIT:WWCT:OP_2"][24], 25.318572860839158)

            self.assertFloatEqual(data["MISFIT:TOTAL"][0], 1621.755076130249)
            self.assertFloatEqual(data["MISFIT:TOTAL"][24], 2783.7582107191383)


            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_21 = data.loc[21]

