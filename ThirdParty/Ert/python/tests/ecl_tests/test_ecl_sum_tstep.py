from datetime import datetime
import random
from ecl.ecl import EclSumTStep, EclSum
from tests import EclTest


class EclSumTStepTest(EclTest):

    def test_creation(self):
        ecl_sum = EclSum.writer("TEST", datetime(2010, 1, 1), 10, 10, 10)
        ecl_sum.addVariable("FOPT")
        ecl_sum.addVariable("FOPR")

        smspec = ecl_sum.cNamespace().get_smspec(ecl_sum)

        test_data = [(1, 0, 10), (1, 1, 20), (1, 2, 30), (2, 0, 40)]

        for report_step, mini_step, sim_days in test_data:
            ecl_sum_tstep = EclSumTStep(report_step, mini_step, sim_days, smspec)

            self.assertEqual(ecl_sum_tstep.getSimDays(), sim_days)
            self.assertEqual(ecl_sum_tstep.getReport(), report_step)
            self.assertEqual(ecl_sum_tstep.getMiniStep(), mini_step)

            self.assertTrue("FOPT" in ecl_sum_tstep)
            self.assertTrue("FOPR" in ecl_sum_tstep)
            self.assertFalse("WWCT" in ecl_sum_tstep)

            random_float = random.random()
            ecl_sum_tstep["FOPT"] = random_float
            ecl_sum_tstep["FOPR"] = random_float + 1

            self.assertAlmostEqual(random_float, ecl_sum_tstep["FOPT"], places=5)
            self.assertAlmostEqual(random_float + 1, ecl_sum_tstep["FOPR"], places=5)

            with self.assertRaises(KeyError):
                ecl_sum_tstep["FROPR"] = 2

            with self.assertRaises(KeyError):
                value = ecl_sum_tstep["FROPR"]

