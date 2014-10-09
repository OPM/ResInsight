from ert.enkf.data import GenKw, GenKwConfig
from ert.test import TestAreaContext, ExtendedTestCase


class GenKwTest(ExtendedTestCase):

    def test_gen_kw_get_set(self):
        with TestAreaContext("enkf/data/gen_kwt"):
            parameter_file ="MULTFLT.txt"
            with open(parameter_file, "w") as f:
                f.write("MULTFLT  NORMAL  0   1")

            gen_kw_config = GenKwConfig("MULTFLT", "%s", "MULTFLT.txt")

            gen_kw = GenKw(gen_kw_config)
            self.assertIsInstance(gen_kw, GenKw)

            gen_kw[0] = 3.0
            self.assertEqual(gen_kw[0], 3.0)

            gen_kw["MULTFLT"] = 4.0
            self.assertEqual(gen_kw["MULTFLT"], 4.0)
            self.assertEqual(gen_kw[0], 4.0)

            self.assertEqual(len(gen_kw), 1)

            with self.assertRaises(IndexError):
                gen_kw[1]

            with self.assertRaises(TypeError):
                gen_kw[1.5]

            with self.assertRaises(KeyError):
                gen_kw["MULTFLT_2"]

            self.assertTrue("MULTFLT" in gen_kw )





































