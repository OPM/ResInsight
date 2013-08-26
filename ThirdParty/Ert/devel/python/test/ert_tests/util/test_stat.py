import random
from ert.util import DoubleVector, quantile, quantile_sorted
from ert_tests import ExtendedTestCase


class StatTest(ExtendedTestCase):
    def test_stat_quantiles(self):
        v = DoubleVector()
        for i in range(100000):
            v.append(random.random())

        self.assertAlmostEqual(quantile(v, 0.1), 0.1, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.2), 0.2, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.3), 0.3, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.4), 0.4, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.5), 0.5, 2)
        # print quantile( v , 0.10 )
        # print quantile_sorted( v , 0.20 )
        # print quantile_sorted( v , 0.30 )
        # print quantile_sorted( v , 0.40 )
        # print quantile_sorted( v , 0.50 )