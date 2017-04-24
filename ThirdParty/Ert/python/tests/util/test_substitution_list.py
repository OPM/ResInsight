from ert.test import ExtendedTestCase
from ert.util import SubstitutionList


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

        self.assertTrue("Key" in subst_list)
        self.assertEqual(subst_list.indexForKey("Key"), 0)

        with self.assertRaises(KeyError):
            subst_list.indexForKey("AnotherKey")
