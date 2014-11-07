from ert_gui.ide.keywords.definitions import PercentArgument
from ert.test import ExtendedTestCase


class PercentArgumentTest(ExtendedTestCase):

    def test_default_percent_argument(self):
        percent = PercentArgument()

        validation_status = percent.validate("45%")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), 0.45)
        self.assertEqual(validation_status.message(), "")


        validation_status = percent.validate("-45%")

        self.assertTrue(validation_status)
        self.assertEqual(validation_status.value(), -0.45)


        validation_status = percent.validate("45")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")
        self.assertIsNone(validation_status.value())

        validation_status = percent.validate("gx")

        self.assertFalse(validation_status)
        self.assertNotEqual(validation_status.message(), "")


    def test_percent_range_argument_from(self):
        from_value = 99
        percent = PercentArgument(from_value=from_value)

        validation_status = percent.validate("%d%%" % from_value)
        self.assertTrue(validation_status)

        value = 98
        validation_status = percent.validate("%d%%" % value)
        self.assertFalse(validation_status)

        range_string = "%g %% <= %g %%" % (from_value, value)
        self.assertEqual(validation_status.message(), PercentArgument.NOT_IN_RANGE % range_string)


    def test_percent_range_argument_to(self):
        to_value = 99
        percent = PercentArgument(to_value=to_value)

        validation_status = percent.validate("%d%%" % to_value)
        self.assertTrue(validation_status)

        value = 100
        validation_status = percent.validate("%d%%" % value)
        self.assertFalse(validation_status)

        range_string = "%g%% <= %g%%" % (value, to_value)
        self.assertEqual(validation_status.message(), PercentArgument.NOT_IN_RANGE % range_string)


    def test_percent_range_argument(self):
        from_value = 10
        to_value = 20
        percent = PercentArgument(from_value=from_value, to_value=to_value)

        validation_status = percent.validate("%d%%" % to_value)
        self.assertTrue(validation_status)

        validation_status = percent.validate("%d%%" % from_value)
        self.assertTrue(validation_status)

        validation_status = percent.validate("%d%%" % 15)
        self.assertTrue(validation_status)

        value = 9
        validation_status = percent.validate("%d%%" % value)
        self.assertFalse(validation_status)

        range_string = "%g%% <= %g%% <= %g%%" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), PercentArgument.NOT_IN_RANGE % range_string)

        value = 21
        validation_status = percent.validate("%d%%" % value)
        self.assertFalse(validation_status)

        range_string = "%g%% <= %g%% <= %g%%" % (from_value, value, to_value)
        self.assertEqual(validation_status.message(), PercentArgument.NOT_IN_RANGE % range_string)








