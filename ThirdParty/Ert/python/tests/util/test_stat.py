from ert.test import ExtendedTestCase
from ert.util import DoubleVector, quantile, quantile_sorted, polyfit
from ert.util.rng import RandomNumberGenerator


class StatTest(ExtendedTestCase):
    def test_stat_quantiles(self):
        rng = RandomNumberGenerator()
        rng.setState("0123456789ABCDEF")
        v = DoubleVector()
        for i in range(100000):
            v.append(rng.getDouble( ))

        self.assertAlmostEqual(quantile(v, 0.1), 0.1, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.2), 0.2, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.3), 0.3, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.4), 0.4, 2)
        self.assertAlmostEqual(quantile_sorted(v, 0.5), 0.5, 2)

    def test_polyfit(self):
        x_list = DoubleVector()
        y_list = DoubleVector()
        S = DoubleVector()

        A = 7.25
        B = -4
        C = 0.025

        x = 0
        dx = 0.1
        for i in range(100):
            y = A + B * x + C * x * x
            x_list.append(x)
            y_list.append(y)

            x += dx
            S.append(1.0)

        beta = polyfit(3, x_list, y_list, None)

        self.assertAlmostEqual(A, beta[0])
        self.assertAlmostEqual(B, beta[1])
        self.assertAlmostEqual(C, beta[2])
