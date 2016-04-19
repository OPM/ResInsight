from ert.enkf.export import SummaryObservationCollector
from ert.test import ErtTestContext, ExtendedTestCase
import os

class SummaryObservationCollectorTest(ExtendedTestCase):

    def setUp(self):
        os.environ["TZ"] = "CET" # The ert_statoil case was generated in CET
        self.config = self.createTestPath("local/snake_oil/snake_oil.ert")


    def test_summary_observation_collector(self):

        with ErtTestContext("python/enkf/export/summary_observation_collector", self.config) as context:

            ert = context.getErt()

            self.assertTrue(SummaryObservationCollector.summaryKeyHasObservations(ert, "FOPR"))
            self.assertFalse(SummaryObservationCollector.summaryKeyHasObservations(ert, "FOPT"))

            keys = SummaryObservationCollector.getAllObservationKeys(ert)
            self.assertTrue("FOPR" in keys)
            self.assertTrue("WOPR:OP1" in keys)
            self.assertFalse("WOPR:OP2" in keys)

            data = SummaryObservationCollector.loadObservationData(ert, "default_0")

            self.assertFloatEqual(data["FOPR"]["2010-01-10"],  0.001696887)
            self.assertFloatEqual(data["STD_FOPR"]["2010-01-10"], 0.1)

            self.assertFloatEqual(data["WOPR:OP1"]["2010-03-31"], 0.1)
            self.assertFloatEqual(data["STD_WOPR:OP1"]["2010-03-31"], 0.05)


            with self.assertRaises(KeyError):
                fgir = data["FGIR"]


            data = SummaryObservationCollector.loadObservationData(ert, "default_0", ["WOPR:OP1"])

            self.assertFloatEqual(data["WOPR:OP1"]["2010-03-31"], 0.1)
            self.assertFloatEqual(data["STD_WOPR:OP1"]["2010-03-31"], 0.05)

            with self.assertRaises(KeyError):
                data["FOPR"]
