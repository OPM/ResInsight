from ert_gui.ide.keywords.definitions import ProperNameArgument
from ert.test import ExtendedTestCase


class ProperNameArgumentTest(ExtendedTestCase):

    def test_proper_name_argument(self):

        argument = ProperNameArgument()

        self.assertTrue(argument.validate("NAME"))
        self.assertTrue(argument.validate("__NAME__"))
        self.assertTrue(argument.validate("<NAME>"))
        self.assertTrue(argument.validate("-NAME-"))

        self.assertFalse(argument.validate("-NA ME-"))
        self.assertFalse(argument.validate("NAME*"))

