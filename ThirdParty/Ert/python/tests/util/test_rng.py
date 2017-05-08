from ecl.util.enums import RngAlgTypeEnum, RngInitModeEnum
from ecl.util.rng import RandomNumberGenerator
from ecl.test import ExtendedTestCase,TestAreaContext


class RngTest(ExtendedTestCase):

    def test_enums(self):
        self.assertEnumIsFullyDefined(RngAlgTypeEnum, "rng_alg_type", "libert_util/include/ert/util/rng.h")
        self.assertEnumIsFullyDefined(RngInitModeEnum, "rng_init_mode", "libert_util/include/ert/util/rng.h")

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

