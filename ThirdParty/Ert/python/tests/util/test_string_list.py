from __future__ import absolute_import, division, print_function, unicode_literals

try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from ecl.util import StringList


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
        s1 = StringList(["A", "B"])
        s1.append("C")

        s2 = StringList(["A","B","C"])
        self.assertEqual(s1, ["A", "B", "C"])
        self.assertEqual(s1, s2)
        self.assertFalse(s1 == ["A","B","D"])
        self.assertFalse(s1 == ["A","B","C" , "D"])

        pfx = 'StringList(size' # __repr__
        self.assertEqual(pfx, repr(s2)[:len(pfx)])

    def test_append_not_string(self):
        s = StringList()
        s.append(10)
        self.assertEqual( len(s) , 1)
        self.assertEqual(s[0] , "10")


    def test_negative_index(self):
        s = StringList(["A", "B", "C"])

        self.assertEqual(s[-1], "C")
        self.assertEqual(s[-3], "A")

        with self.assertRaises(LookupError):
            s = s[-4]

    def test_find_first(self):
        s = StringList(["A", "B", "C"])

        self.assertEqual(s.index("A"), 0)
        self.assertEqual(s.index("B"), 1)
        self.assertEqual(s.index("C"), 2)
        self.assertEqual(s.index("D"), -1)



    def test_front_back(self):
        s = StringList()
        with self.assertRaises(LookupError):
            s.front()

        with self.assertRaises(LookupError):
            s.back()

        s.append("S1")
        s.append("S2")
        s.append("S3")

        self.assertEqual( "S1" , s.front() )
        self.assertEqual( "S3" , s.back() )


    def test_iadd(self):
        s1 = StringList( initial = ["A","B","C"])
        with self.assertRaises(TypeError):
             s3 = s1 + 10
        

        s2 = StringList( initial = ["A","B","C"])
        s3 = s1 + s2
        self.assertEqual( s3 , ["A","B","C","A","B","C"])

        s1 += s2
        self.assertEqual( s1 , ["A","B","C","A","B","C"])
        with self.assertRaises(TypeError):
             s3 += "b"
        

    def test_ior(self):
        s1 = StringList( initial = ["A","B","C"])
        s2 = StringList( initial = ["A","B","C"])

        s3 = s1 | s2
        self.assertEqual( s3 , ["A","B","C"])
        s1 |= s2
        self.assertEqual( s1 , ["A","B","C"])

        with self.assertRaises(TypeError):
            s1 |= 26
