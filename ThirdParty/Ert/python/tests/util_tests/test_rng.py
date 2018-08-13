from ecl.util.enums import RngAlgTypeEnum, RngInitModeEnum
from ecl.util.util import RandomNumberGenerator
from ecl.util.test import TestAreaContext
from tests import EclTest


class RngTest(EclTest):

    def test_enums(self):
        self.assertEnumIsFullyDefined(RngAlgTypeEnum, "rng_alg_type", "lib/include/ert/util/rng.hpp")
        self.assertEnumIsFullyDefined(RngInitModeEnum, "rng_init_mode", "lib/include/ert/util/rng.hpp")

    def test_rng_default(self):
        rng = RandomNumberGenerator()
        self.assertIsInstance(rng.getDouble(), float)

    def test_rng_state(self):
        rng = RandomNumberGenerator()
        with self.assertRaises(ValueError):
            rng.setState("12")

        rng.setState("0123456789ABCDEF")
        val1 = rng.getInt()
        val2 = rng.getInt()

        self.assertFalse( val1 == val2 )
        rng.setState("0123456789ABCDEF")
        self.assertEqual( rng.getInt() , val1)
        self.assertEqual( rng.getInt() , val2)



    def test_load_save(self):
        rng = RandomNumberGenerator()
        with self.assertRaises(IOError):
            rng.loadState("does/not/exist")

        with TestAreaContext("rng_state") as t:
            rng.saveState( "rng.txt" )
            t.sync()
            val1 = rng.getInt()
            val2 = rng.getInt()
            rng.loadState( "rng.txt" )
            self.assertEqual( rng.getInt() , val1 )
            self.assertEqual( rng.getInt() , val2 )

