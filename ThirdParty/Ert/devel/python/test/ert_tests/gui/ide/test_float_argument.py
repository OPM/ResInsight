from ert_gui.ide.keywords.definitions import FloatArgument
from ert.test import ExtendedTestCase


class FloatArgumentTest(ExtendedTestCase):

    def test_default_float_argument(self):
        f = FloatArgument()

        validation_status = f.validate("45.0")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), 45.0)
        self.assertEqual(validation_status.message(), "")


        validation_status = f.validate("-45.0")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), -45)


        validation_status = f.validate("45.0 ")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")
        self.assertIsNone(validation_status.value())

        validation_status = f.validate("gx")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")


    def test_float_range_argument_from(self):
        from_value = 9.9
        f = FloatArgument(from_value=from_value)

        validation_status = f.validate("%f" % from_value)
        self.assertTrue(validation_status)

        value = 9.85
        validation_status = f.validate("%f" % value)
        self.assertFalse(validation_status)

        range_string = "%f <= %f" % (from_value, value)
        self.assertEqual(validation_status.message(), FloatArgument.NOT_IN_RANGE % range_string)


    def test_float_range_argument_to(self):
        to_value = 9.9
        f = FloatArgument(to_value=to_value)

        validation_status = f.validate("%f" % to_value)
        self.assertTrue(validation_status)

        value = 9.91
        validation_status = f.validate("%f" % value)
        self.assertFalse(validation_status)

        range_string = "%f <= %f" % (value, to_value)
        self.assertEqual(validation_status.message(), FloatArgument.NOT_IN_RANGE % range_string)


    def test_float_range_argument(self):
        from_value = 1.0
        to_value = 1.1
        f = FloatArgument(from_value=from_value, to_value=to_value)

        validation_status = f.validate("%f" % to_value)
        self.assertTrue(validation_status)

        validation_status = f.validate("%f" % from_value)
        self.assertTrue(validation_status)

        validation_status = f.validate("%f" % 1.05)
        self.assertTrue(validation_status)

        value = 0.9
        validation_status = f.validate("%f" % value)
        self.assertFalse(validation_status)

        range_string = "%f <= %f <= %f" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), FloatArgument.NOT_IN_RANGE % range_string)

        value = 1.15
        validation_status = f.validate("%f" % value)
        self.assertFalse(validation_status)

        range_string = "%f <= %f <= %f" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), FloatArgument.NOT_IN_RANGE % range_string)








