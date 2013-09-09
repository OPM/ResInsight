from ert.config import CONFIG_LIB
from ert.cwrap import BaseCEnum
from ert_tests import ExtendedTestCase



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


    def test_enum_populate_from_c(self):
        class ContentTypeEnum(BaseCEnum):
            pass

        ContentTypeEnum.populateEnum(CONFIG_LIB, "config_schema_item_type_enum_iget")

        # CONFIG_STRING        = 1,
        # CONFIG_INT           = 2,
        # CONFIG_FLOAT         = 4,
        # CONFIG_PATH          = 8,
        # CONFIG_EXISTING_PATH = 16,
        # CONFIG_BOOL          = 32,
        # CONFIG_CONFIG        = 64,
        # CONFIG_BYTESIZE      = 128,
        # CONFIG_EXECUTABLE    = 256 ,
        # CONFIG_INVALID       = 512

        self.assertEqual(ContentTypeEnum.CONFIG_STRING, 1)
        self.assertEqual(ContentTypeEnum.CONFIG_EXISTING_PATH, 16)
        self.assertEqual(ContentTypeEnum.CONFIG_BYTESIZE, 128)
        self.assertEqual(ContentTypeEnum.CONFIG_INVALID, 512)

