from ert_gui.ide.keywords.definitions import IntegerArgument
from ert.test import ExtendedTestCase


class IntegerArgumentTest(ExtendedTestCase):

    def test_default_integer_argument(self):
        integer = IntegerArgument()

        validation_status = integer.validate("45")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), 45)
        self.assertEqual(validation_status.message(), "")


        validation_status = integer.validate("-45")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), -45)


        validation_status = integer.validate("45 ")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")
        self.assertIsNone(validation_status.value())

        validation_status = integer.validate("gx")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")


    def test_integer_range_argument_from(self):
        from_value = 99
        integer = IntegerArgument(from_value=from_value)

        validation_status = integer.validate("%d" % from_value)
        self.assertTrue(validation_status)

        value = 98
        validation_status = integer.validate("%d" % value)
        self.assertFalse(validation_status)

        range_string = "%d <= %d" % (from_value, value)
        self.assertEqual(validation_status.message(), IntegerArgument.NOT_IN_RANGE % range_string)


    def test_integer_range_argument_to(self):
        to_value = 99
        integer = IntegerArgument(to_value=to_value)

        validation_status = integer.validate("%d" % to_value)
        self.assertTrue(validation_status)

        value = 100
        validation_status = integer.validate("%d" % value)
        self.assertFalse(validation_status)

        range_string = "%d <= %d" % (value, to_value)
        self.assertEqual(validation_status.message(), IntegerArgument.NOT_IN_RANGE % range_string)


    def test_integer_range_argument(self):
        from_value = 10
        to_value = 20
        integer = IntegerArgument(from_value=from_value, to_value=to_value)

        validation_status = integer.validate("%d" % to_value)
        self.assertTrue(validation_status)

        validation_status = integer.validate("%d" % from_value)
        self.assertTrue(validation_status)

        validation_status = integer.validate("%d" % 15)
        self.assertTrue(validation_status)

        value = 9
        validation_status = integer.validate("%d" % value)
        self.assertFalse(validation_status)

        range_string = "%d <= %d <= %d" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), IntegerArgument.NOT_IN_RANGE % range_string)

        value = 21
        validation_status = integer.validate("%d" % value)
        self.assertFalse(validation_status)

        range_string = "%d <= %d <= %d" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), IntegerArgument.NOT_IN_RANGE % range_string)








