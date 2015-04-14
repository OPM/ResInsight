from ert.enkf.export import SummaryCollector
from ert.test import ErtTestContext, ExtendedTestCase

class SummaryCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")


    def test_summary_collector(self):

        with ErtTestContext("python/enkf/export/summary_collector", self.config) as context:

            ert = context.getErt()

            data = SummaryCollector.loadAllSummaryData(ert, "default")

            self.assertFloatEqual(data["WWCT:OP_2"][0]["2000-02-01"], 0.0003561387420631945)
            self.assertFloatEqual(data["WWCT:OP_2"][24]["2000-02-01"], 0.0001338826841674745)

            self.assertFloatEqual(data["FOPR"][0]["2000-02-01"], 19997.0)
            self.assertFloatEqual(data["FOPR"][0]["2004-12-01"], 6998.982421875)


            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_21 = data.loc[21]


            data = SummaryCollector.loadAllSummaryData(ert, "default", ["WWCT:OP_2", "WWCT:OP_3"])

            self.assertFloatEqual(data["WWCT:OP_2"][0]["2000-02-01"], 0.0003561387420631945)
            self.assertFloatEqual(data["WWCT:OP_3"][0]["2000-02-01"], 0.000117796211271)

            with self.assertRaises(KeyError):
                data["FOPR"]
