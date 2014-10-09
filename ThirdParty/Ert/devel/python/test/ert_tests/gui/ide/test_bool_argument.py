from ert_gui.ide.keywords.definitions import BoolArgument
from ert.test import ExtendedTestCase


class BoolArgumentTest(ExtendedTestCase):



    def test_bool_argument(self):
        bool_arg = BoolArgument()

        validation_status = bool_arg.validate("TRUE")
        self.assertTrue(validation_status)
        self.assertTrue(validation_status.value())
        self.assertEqual(validation_status.message(), "")

        validation_status = bool_arg.validate("FALSE")
        self.assertTrue(validation_status)
        self.assertFalse(validation_status.value())
        self.assertEqual(validation_status.message(), "")

        validation_status = bool_arg.validate("True")
        self.assertTrue(validation_status)

        validation_status = bool_arg.validate("False")
        self.assertTrue(validation_status)

        validation_status = bool_arg.validate(" FALSE")
        self.assertFalse(validation_status)
        self.assertEqual(validation_status.message(), bool_arg.NOT_BOOL)

        self.assertTrue(bool_arg.validate("T"))
        self.assertTrue(bool_arg.validate("F"))
        self.assertTrue(bool_arg.validate("0"))
        self.assertTrue(bool_arg.validate("1"))
        self.assertTrue(bool_arg.validate("TrUe"))
        self.assertTrue(bool_arg.validate("FaLsE"))

        self.assertFalse(bool_arg.validate("t"))
        self.assertFalse(bool_arg.validate("f"))
        self.assertFalse(bool_arg.validate("Tr"))

        self.assertTrue(bool_arg.validate("T").value())
        self.assertFalse(bool_arg.validate("F").value())









