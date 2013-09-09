from ert.cwrap import BaseCClass
from ert_tests import ExtendedTestCase


class BaseCClassTest(ExtendedTestCase):

    def test_none_assertion(self):
        self.assertFalse(None > 0)

    def test_creation(self):
        with self.assertRaises(ValueError):
            obj = BaseCClass(0)