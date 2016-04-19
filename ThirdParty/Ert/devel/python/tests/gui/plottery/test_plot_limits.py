import datetime

from ert.test import ExtendedTestCase
from ert_gui.plottery import PlotLimits


class PlotLimitsTest(ExtendedTestCase):

    def test_plot_limits_construction(self):
        plot_limits = PlotLimits()
        self.assertEqual(plot_limits.value_minimum, None)
        self.assertEqual(plot_limits.value_maximum, None)
        self.assertEqual(plot_limits.value_limits, (None, None))

        self.assertEqual(plot_limits.index_minimum, None)
        self.assertEqual(plot_limits.index_maximum, None)
        self.assertEqual(plot_limits.index_limits, (None, None))

        self.assertEqual(plot_limits.count_minimum, None)
        self.assertEqual(plot_limits.count_maximum, None)
        self.assertEqual(plot_limits.count_limits, (None, None))

        self.assertEqual(plot_limits.density_minimum, None)
        self.assertEqual(plot_limits.density_maximum, None)
        self.assertEqual(plot_limits.density_limits, (None, None))

        self.assertEqual(plot_limits.depth_minimum, None)
        self.assertEqual(plot_limits.depth_maximum, None)
        self.assertEqual(plot_limits.depth_limits, (None, None))

        self.assertEqual(plot_limits.date_minimum, None)
        self.assertEqual(plot_limits.date_maximum, None)
        self.assertEqual(plot_limits.date_limits, (None, None))


    def test_plot_limits(self):
        plot_limits = PlotLimits()
        limit_names = ["value", "index", "count", "density", "depth", "date"]

        non_numbers = ["string", datetime.date(2001, 1, 1), "3.0", "1e-5", "-5.5", "-.5"]

        positive_floats = [1.0, 1.5, 3.1415, 1e10, 5.2e-7]
        negative_floats = [-1.0, -1.5, -3.1415, -1e10, -5.2e-7]
        positive_ints = [1, 5, 1000]
        negative_ints = [-1, -5, -1000]

        non_dates = ["string", "3.4", "2001-01-01", datetime.time()]
        dates = [datetime.date(2001, 1, 1), datetime.datetime(2010, 3, 3)]

        setter_should_fail_values = {
            "value": non_numbers + dates,
            "index": non_numbers + positive_floats + negative_floats + dates,
            "depth": non_numbers + negative_floats + negative_ints + negative_ints,
            "count": non_numbers + negative_ints + negative_floats + positive_floats,
            "density": non_numbers + negative_floats + negative_ints,
            "date": non_dates
        }

        setter_should_succeed_values = {
            "value": positive_floats + negative_floats + positive_ints + negative_ints,
            "index": positive_ints,
            "depth": positive_floats + positive_ints,
            "count": positive_ints,
            "density": positive_floats + positive_ints,
            "date": dates
        }

        for attribute_name in limit_names:
            self.assertIsNone(getattr(plot_limits, "%s_minimum" % attribute_name))
            self.assertIsNone(getattr(plot_limits, "%s_maximum" % attribute_name))
            self.assertTupleEqual(getattr(plot_limits, "%s_limits" % attribute_name), (None, None))

            with self.assertNotRaises():
                setattr(plot_limits, "%s_minimum" % attribute_name, None)
                setattr(plot_limits, "%s_maximum" % attribute_name, None)
                setattr(plot_limits, "%s_limits" % attribute_name, (None, None))

            with self.assertRaises(TypeError):
                setattr(plot_limits, "%s_limits" % attribute_name, None)

            for value in setter_should_fail_values[attribute_name]:
                with self.assertRaises((TypeError, ValueError)):
                    setattr(plot_limits, "%s_minimum" % attribute_name, value)

                with self.assertRaises((TypeError, ValueError)):
                    setattr(plot_limits, "%s_maximum" % attribute_name, value)

                self.assertTupleEqual(getattr(plot_limits, "%s_limits" % attribute_name), (None, None))

            for value in setter_should_succeed_values[attribute_name]:
                with self.assertNotRaises():
                    setattr(plot_limits, "%s_minimum" % attribute_name, value)
                    setattr(plot_limits, "%s_maximum" % attribute_name, value)

                minimum = getattr(plot_limits, "%s_minimum" % attribute_name)
                maximum = getattr(plot_limits, "%s_maximum" % attribute_name)

                self.assertEqual(minimum, value)
                self.assertEqual(maximum, value)

                self.assertTupleEqual(getattr(plot_limits, "%s_limits" % attribute_name), (minimum, maximum))


    def test_copy_plot_limits(self):
        plot_limits = PlotLimits()
        plot_limits.value_limits = 1, 2
        plot_limits.index_limits = 3, 4
        plot_limits.count_limits = 5, 6
        plot_limits.depth_limits = 7, 8
        plot_limits.density_limits = 9, 10
        plot_limits.date_limits = datetime.date(1999, 1, 1), datetime.date(1999, 12, 31)

        copy_of_plot_limits = PlotLimits()

        copy_of_plot_limits.copyLimitsFrom(plot_limits)

        self.assertEqual(copy_of_plot_limits, plot_limits)

        self.assertEqual(copy_of_plot_limits.value_limits, (1, 2))
        self.assertEqual(copy_of_plot_limits.index_limits, (3, 4))
        self.assertEqual(copy_of_plot_limits.count_limits, (5, 6))
        self.assertEqual(copy_of_plot_limits.depth_limits, (7, 8))
        self.assertEqual(copy_of_plot_limits.density_limits, (9, 10))
        self.assertEqual(copy_of_plot_limits.date_limits, (datetime.date(1999, 1, 1), datetime.date(1999, 12, 31)))
