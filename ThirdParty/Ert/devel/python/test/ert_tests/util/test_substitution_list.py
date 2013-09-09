from ert.util import SubstitutionList
from ert_tests import ExtendedTestCase


class SubstitutionListTest(ExtendedTestCase):
    def test_substitution_list(self):
        subst_list = SubstitutionList()

        subst_list.addItem("Key", "Value", "Doc String")

        self.assertEqual(len(subst_list), 1)

        with self.assertRaises(IndexError):
            item = subst_list["1"]

        with self.assertRaises(IndexError):
            item = subst_list[2]

        key, value, doc_string = subst_list[0]

        self.assertEqual(key, "Key")
        self.assertEqual(value, "Value")
        self.assertEqual(doc_string, "Doc String")

