from unittest2 import TestCase
from ert.util import LookupTable
import numpy

class LookupTableTest(TestCase):

    def test_lookup_table_no_values(self):
        lookup = LookupTable()

        self.assertTrue(numpy.isnan(lookup.max))
        self.assertTrue(numpy.isnan(lookup.min))
        self.assertTrue(numpy.isnan(lookup.arg_max))
        self.assertTrue(numpy.isnan(lookup.arg_min))
        self.assertEqual(len(lookup), 0)


        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

    def test_lookup_table_min_and_max(self):
        lookup = LookupTable()

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.max, 10.0)
        self.assertEqual(lookup.min, 0.0)
        self.assertEqual(lookup.arg_max, 1.0)
        self.assertEqual(lookup.arg_min, 0.0)
        self.assertEqual(len(lookup), 2)


    def test_lookup_table_interpolation(self):
        lookup = LookupTable()

        lookup.append(0.0, 0.0)
        lookup.append(1.0, 10.0)

        self.assertEqual(lookup.interp(0.5), 5.0)

