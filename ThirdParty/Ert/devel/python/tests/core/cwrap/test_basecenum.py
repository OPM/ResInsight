from ert.cwrap import BaseCEnum
from ert.test import ExtendedTestCase



class BaseCEnumTest(ExtendedTestCase):
    def test_base_c_enum(self):
        class enum(BaseCEnum):
            pass

        enum.addEnum("ONE", 1)
        enum.addEnum("TWO", 2)
        enum.addEnum("THREE", 3)
        enum.addEnum("FOUR", 4)

        class enum2(BaseCEnum):
            pass

        enum2.addEnum("ONE", 1)
        enum2.addEnum("TWO", 4)

        self.assertEqual(enum.ONE, 1)
        self.assertEqual(enum.TWO, 2)
        self.assertEqual(enum.FOUR, 4)

        self.assertListEqual(enum.enums(), [enum.ONE, enum.TWO, enum.THREE, enum.FOUR])

        self.assertEqual(enum(4), enum.FOUR)

        self.assertNotEqual(enum2(4), enum.FOUR)
        self.assertEqual(enum2(4), enum2.TWO)

        self.assertEqual(str(enum.ONE), "ONE")


        self.assertEqual(enum.ONE + enum.TWO, enum.THREE)
        self.assertEqual(enum.ONE + enum.FOUR, 5)

        with self.assertRaises(ValueError):
            e = enum(5)


        self.assertEqual(enum.THREE & enum.ONE, enum.ONE)
        self.assertEqual(enum.ONE | enum.TWO, enum.THREE)
        self.assertEqual(enum.THREE ^ enum.TWO, enum.ONE)


        with self.assertRaises(AssertionError):
            e = enum.ONE + enum2.ONE

        with self.assertRaises(AssertionError):
            e = enum.ONE & enum2.ONE

        with self.assertRaises(AssertionError):
            e = enum.ONE | enum2.ONE

        with self.assertRaises(AssertionError):
            e = enum.ONE ^ enum2.ONE


    def test_in_operator(self):
        class PowerOf2(BaseCEnum):
            pass

        PowerOf2.addEnum("ONE", 1)
        PowerOf2.addEnum("TWO", 2)
        PowerOf2.addEnum("FOUR", 4)

        three = PowerOf2.ONE | PowerOf2.TWO

        self.assertEqual(int(three), 3)

        self.assertIn(PowerOf2.TWO, three)
        self.assertIn(PowerOf2.ONE, three)
        self.assertNotIn(PowerOf2.FOUR, three)



