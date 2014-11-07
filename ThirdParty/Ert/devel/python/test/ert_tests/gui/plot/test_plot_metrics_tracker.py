from ert.test.extended_testcase import ExtendedTestCase
from ert_gui.tools.plot.plot_metrics_tracker import PlotMetricsTracker


class PlotMetricTrackerTest(ExtendedTestCase):

    def test_creation(self):
        pmt = PlotMetricsTracker()

        self.assertIsNone(pmt.getType(None))

        with self.assertRaises(KeyError):
            pmt.getType("type")


        self.assertIsNone(pmt.getDataTypeKey())

        pmt.setDataTypeKey("test data type")
        self.assertEqual(pmt.getDataTypeKey(), "test data type")



    def test_scale_types(self):
        pmt = PlotMetricsTracker()

        pmt.addScaleType("index", int)
        self.assertEqual(pmt.getType("index"), int)

        pmt.addScaleType("value", float)
        self.assertEqual(pmt.getType("value"), float)


        with self.assertRaises(KeyError):
            pmt.getScalesForType("index")


        pmt.setDataTypeKey("TestData1")
        self.assertEqual(pmt.getScalesForType("index"), (None, None))
        self.assertEqual(pmt.getScalesForType("value"), (None, None))

        pmt.setDataTypeKey("TestData2")
        self.assertEqual(pmt.getScalesForType("index"), (None, None))
        self.assertEqual(pmt.getScalesForType("value"), (None, None))

        pmt.setScalesForType("index", 1, 2)
        pmt.setScalesForType("value", 1.5, 2.9)
        self.assertEqual(pmt.getScalesForType("index"), (1, 2))
        self.assertEqual(pmt.getScalesForType("value"), (1.5, 2.9))

        pmt.setDataTypeKey("TestData1")
        self.assertEqual(pmt.getScalesForType("index"), (None, None))
        self.assertEqual(pmt.getScalesForType("value"), (None, None))

        pmt.setScalesForType("index", 10, 20)
        pmt.setScalesForType("value", 2.5, 3.9)
        self.assertEqual(pmt.getScalesForType("index"), (10, 20))
        self.assertEqual(pmt.getScalesForType("value"), (2.5, 3.9))

        with self.assertRaises(KeyError):
            pmt.setScalesForType("time", 99, 999)
