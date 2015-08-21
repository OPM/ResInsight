from ert.enkf.export import SummaryObservationCollector
from ert.test import ErtTestContext, ExtendedTestCase

class SummaryObservationCollectorTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")


    def test_summary_observation_collector(self):

        with ErtTestContext("python/enkf/export/summary_observation_collector", self.config) as context:

            ert = context.getErt()

            self.assertTrue(SummaryObservationCollector.summaryKeyHasObservations(ert, "FOPT"))
            self.assertFalse(SummaryObservationCollector.summaryKeyHasObservations(ert, "FGIR"))

            keys = SummaryObservationCollector.getAllObservationKeys(ert)
            self.assertTrue("FOPT" in keys)
            self.assertTrue("RPR:2" in keys)

            data = SummaryObservationCollector.loadObservationData(ert, "default")

            self.assertFloatEqual(data["RPR:2"]["2001-01-01"], 278)
            self.assertFloatEqual(data["STD_RPR:2"]["2001-01-01"], 41.7)

            self.assertFloatEqual(data["FOPT"]["2000-02-01"], 619907.0)
            self.assertFloatEqual(data["STD_FOPT"]["2000-02-01"], 61990.7)


            with self.assertRaises(KeyError):
                fgir = data["FGIR"]


            data = SummaryObservationCollector.loadObservationData(ert, "default", ["RPR:3"])

            self.assertFloatEqual(data["RPR:3"]["2001-01-01"], 283.5)
            self.assertFloatEqual(data["STD_RPR:3"]["2001-01-01"], 2)

            with self.assertRaises(KeyError):
                data["FOPR"]
