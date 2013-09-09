from unittest2 import TestCase
from ert.util import StringList


class StringListTest(TestCase):
    def test_del(self):
        s = StringList( initial = ["A", "list"] )
        internal_list_of_strings = s.strings
        python_list_of_strings = list(s)

        self.assertEqual(internal_list_of_strings, python_list_of_strings)

        del s

        self.assertEqual(python_list_of_strings, ["A", "list"])

    def test_iterate(self):
        s = ["A", "list", "of", "strings"]
        s1 = StringList(initial=s)
        s2 = s1.strings
        s3 = list(s1)

        for index in range(len(s1)):
            self.assertEqual(s[index], s1[index])

        for index in range(len(s2)):
            self.assertEqual(s[index], s2[index])

        for index in range(len(s3)):
            self.assertEqual(s[index], s3[index])


    def test_pop( self ):
        s = StringList(initial=["A", "list", "of", "strings"])
        s1 = s.pop()
        self.assertTrue(len(s) == 3)
        self.assertTrue(s1 == "strings")

        s1 = s.pop()
        self.assertTrue(len(s) == 2)
        self.assertTrue(s1 == "of")

        s1 = s.pop()
        self.assertTrue(len(s) == 1)
        self.assertTrue(s1 == "list")

        s1 = s.pop()
        self.assertTrue(len(s) == 0)
        self.assertTrue(s1 == "A")

        with self.assertRaises(IndexError):
            s.pop()

    def test_last(self):
        s = StringList(initial=["A", "list", "of", "strings"])
        self.assertEqual(s.last, "strings")

        with self.assertRaises(IndexError):
            s.pop()
            s.pop()
            s.pop()
            s.pop()
            s.last


    def test_in_and_not_in(self):
        s = StringList(["A", "list", "of", "strings"])

        self.assertTrue("A" in s)
        self.assertTrue("Bjarne" not in s)

    def test_append(self):
        s = StringList(["A", "B"])
        s.append("C")
        self.assertEqual(list(s), ["A", "B", "C"])

    def test_negative_index(self):
        s = StringList(["A", "B", "C"])

        self.assertEqual(s[-1], "C")
        self.assertEqual(s[-3], "A")

        with self.assertRaises(LookupError):
            s = s[-4]