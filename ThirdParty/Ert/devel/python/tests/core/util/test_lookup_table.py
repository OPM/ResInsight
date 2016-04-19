try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from ert.util import LookupTable


class LookupTableTest(TestCase):
    def test_lookup_table_no_values(self):
        lookup = LookupTable()

        self.assertEqual(len(lookup), 0)

        with self.assertRaises(ValueError):
            lookup.getMaxValue()

        with self.assertRaises(ValueError):
            lookup.getMinValue()

        with self.assertRaises(ValueError):
            lookup.getMaxArg()

        with self.assertRaises(ValueError):
            lookup.getMinArg()

        with self.assertRaises(ValueError):
            lookup.interp(0.25)

    def test_lookup_table_one_value(self):
        lookup = LookupTable()
        lookup.append(0, 0)
        with self.assertRaises(ValueError):
            lookup.interp(0.25)

    def test_lookup_table_min_and_max(self):
        lookup = LookupTable()

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.getMaxValue(), 10.0)
        self.assertEqual(lookup.getMinValue(), 0.0)
        self.assertEqual(lookup.getMaxArg(), 1.0)
        self.assertEqual(lookup.getMinArg(), 0.0)
        self.assertEqual(len(lookup), 2)

    def test_lookup_out_of_bounds(self):
        lookup = LookupTable()

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.interp(0), 0)
        self.assertEqual(lookup.interp(1), 10)

        with self.assertRaises(ValueError):
            lookup.interp(-1)

        with self.assertRaises(ValueError):
            lookup.interp(2)

        lookup.setLowerLimit(-1)
        self.assertEqual(lookup.interp(-0.25), -1)

        with self.assertRaises(ValueError):
            lookup.interp(2)

        lookup.setUpperLimit(88)
        self.assertEqual(lookup.interp(1.25), 88)

    def test_lookup_table_interpolation(self):
        lookup = LookupTable()

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.interp(0.5), 5.0)

    def test_optional_arg(self):
        lookup = LookupTable(lower_limit=-1, upper_limit=100)

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.interp(-1), -1.0)
        self.assertEqual(lookup.interp(0.5), 5.0)
        self.assertEqual(lookup.interp(2), 100.0)
