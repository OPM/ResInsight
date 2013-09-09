from ctypes import c_void_p
from ert.util import Hash, StringHash, DoubleHash, IntegerHash
from ert_tests import ExtendedTestCase


class HashTest(ExtendedTestCase):
    def test_string_hash(self):
        hash = StringHash()

        self.assertEqual(len(hash), 0)

        hash["hipp"] = ""

        self.assertEqual(len(hash), 1)

        with self.assertRaises(ValueError):
            hash["hopp"] = 55

        with self.assertRaises(KeyError):
            hopp = hash["hopp"]



        self.assertTrue("hipp" in hash)

        self.assertEqual(list(hash.keys()), ["hipp"])

    def test_int_hash(self):
        hash = IntegerHash()

        with self.assertRaises(ValueError):
            hash["one"] = "ein"

        with self.assertRaises(ValueError):
            hash["one"] = 1.0

        hash["two"] = 2

        self.assertEqual(hash["two"], 2)


    def test_double_hash(self):
        hash = DoubleHash()

        with self.assertRaises(ValueError):
            hash["one"] = "ein"

        hash["two"] = 2
        hash["three"] = 3.0

        self.assertEqual(hash["two"], 2)
        self.assertEqual(hash["three"], 3.0)



    def test_c_void_p_hash(self):
        hash = Hash()

        cp = c_void_p(512)
        hash["1"] = cp

        self.assertEqual(hash["1"], cp.value)

    def test_for_in_hash(self):
        hash = StringHash()

        hash["one"] = "one"
        hash["two"] = "two"
        hash["three"] = "three"

        for key in hash:
            self.assertTrue(key in hash)



