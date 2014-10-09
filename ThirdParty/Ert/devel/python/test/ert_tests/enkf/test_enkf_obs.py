from ert.enkf import ObsVector
from ert.util import BoolVector
from ert.test import ErtTestContext
from ert.test import ExtendedTestCase


class EnKFObsTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/obs_testing/config")

    def testObs(self):
        with ErtTestContext("obs_test", self.config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()

            self.assertEqual(31, len(obs))
            for v in obs:
                self.assertTrue(isinstance(v, ObsVector))

            with self.assertRaises(IndexError):
                v = obs[-1]

            with self.assertRaises(IndexError):
                v = obs[40]

            with self.assertRaises(KeyError):
                v = obs["No-this-does-not-exist"]

            v1 = obs["WWCT:OP_3"]
            v2 = obs["GOPT:OP"]
            mask = BoolVector(True, ert.getEnsembleSize())
            current_fs = ert.getEnkfFsManager().getCurrentFileSystem()

            self.assertTrue(v1.hasData(mask, current_fs))
            self.assertFalse(v2.hasData(mask, current_fs))
            
