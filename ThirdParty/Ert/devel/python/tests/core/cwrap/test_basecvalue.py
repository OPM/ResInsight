from ctypes import c_ubyte, c_long
from ert.cwrap import BaseCValue, clib, CWrapper
from ert.test import ExtendedTestCase


class UnsignedByteValue(BaseCValue):
    DATA_TYPE = c_ubyte


class TimeTValue(BaseCValue):
    DATA_TYPE = c_long


class BaseCValueTest(ExtendedTestCase):
    def setUp(self):
        ert = clib.ert_load("libert_util")

        self.ert_wrapper = CWrapper(ert)

        self.ert_wrapper.registerType("time_t", TimeTValue)
        self.make_date = self.ert_wrapper.prototype("time_t util_make_date(int, int, int)")


    def test_illegal_type(self):
        class ExceptionValueTest(BaseCValue):
            DATA_TYPE = str
            def __init__(self, value):
                super(ExceptionValueTest, self).__init__(value)

        with self.assertRaises(ValueError):
            test = ExceptionValueTest("Failure")


        class NoDataTypeTest(BaseCValue):
            def __init__(self, value):
                super(NoDataTypeTest, self).__init__(value)

        with self.assertRaises(ValueError):
            test = ExceptionValueTest(0)


    def test_creation(self):
        test_value = UnsignedByteValue(255)

        self.assertEqual(test_value.value(), 255)

        test_value.setValue(256)
        self.assertEqual(test_value.value(), 0)

        self.assertEqual(test_value.type(), c_ubyte)


    def test_from_param(self):
        test_value = UnsignedByteValue(127)

        self.assertEqual(UnsignedByteValue.from_param(test_value).value, 127)

        with self.assertRaises(AttributeError):
            UnsignedByteValue.from_param(None)

        with self.assertRaises(ValueError):
           UnsignedByteValue.from_param("exception")


    def test_time_t(self):
        future = self.make_date(1, 1, 2050)

        self.assertIsInstance(future, TimeTValue)
        self.assertEqual(future.value(), 2524604400)
