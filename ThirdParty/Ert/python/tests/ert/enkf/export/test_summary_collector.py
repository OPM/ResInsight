from ert.enkf.export import SummaryCollector
from ert.test import ErtTestContext, ExtendedTestCase
import os


class SummaryCollectorTest(ExtendedTestCase):
    def setUp(self):
        os.environ["TZ"] = "CET" # The ert_statoil case was generated in CET
        self.config = self.createTestPath("local/snake_oil/snake_oil.ert")

    def test_summary_collector(self):
        with ErtTestContext("python/enkf/export/summary_collector", self.config) as context:
            ert = context.getErt()

            data = SummaryCollector.loadAllSummaryData(ert, "default_0")

            self.assertFloatEqual(data["WWCT:OP2"][0]["2010-01-10"], 0.385549)
            self.assertFloatEqual(data["WWCT:OP2"][24]["2010-01-10"], 0.498331)

            self.assertFloatEqual(data["FOPR"][0]["2010-01-10"], 0.118963)
            self.assertFloatEqual(data["FOPR"][0]["2015-06-23"], 0.133601)

            realization_20 = data.loc[20]

            with self.assertRaises(KeyError):
                realization_60 = data.loc[60]

            data = SummaryCollector.loadAllSummaryData(ert, "default_0", ["WWCT:OP1", "WWCT:OP2"])

            self.assertFloatEqual(data["WWCT:OP1"][0]["2010-01-10"], 0.352953)
            self.assertFloatEqual(data["WWCT:OP2"][0]["2010-01-10"], 0.385549)

            with self.assertRaises(KeyError):
                data["FOPR"]
