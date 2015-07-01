from ert.test import ErtTestContext
from ert.test import ExtendedTestCase

from ert.util import BoolVector,IntVector
from ert.enkf import ActiveMode, EnsembleConfig
from ert.enkf import ObsVector , LocalObsdata


class EnsembleConfigTest(ExtendedTestCase):

    def test_create(self):
        conf = EnsembleConfig( )
        self.assertEqual( len(conf) , 0 )
        self.assertFalse( "XYZ" in conf )

        with self.assertRaises(KeyError):
            node = conf["KEY"]
