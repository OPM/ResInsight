from __future__ import absolute_import, division, print_function, unicode_literals
from six import string_types
import ctypes

import ert
from cwrap import BaseCClass, Prototype, PrototypeError
from ert.test import ExtendedTestCase


# Local copies so that the real ones don't get changed
class TestUtilPrototype(Prototype):
    lib = ert.load("libert_util")
    def __init__(self, prototype, bind=False):
        super(TestUtilPrototype, self).__init__(TestUtilPrototype.lib, prototype, bind=bind)

class BoundTestUtilPrototype(TestUtilPrototype):
    def __init__(self, prototype):
        super(BoundTestUtilPrototype, self).__init__(prototype, bind=True)


class StringList(BaseCClass):
    TYPE_NAME = "test_stringlist"

    __len__ = BoundTestUtilPrototype("int stringlist_get_size(test_stringlist)")
    free    = BoundTestUtilPrototype("void stringlist_free(test_stringlist)")

    _alloc  = TestUtilPrototype("void* stringlist_alloc_new()", bind = False)
    _iget   = TestUtilPrototype("char* stringlist_iget(test_stringlist, int)")
    _append = TestUtilPrototype("void  stringlist_append_copy(test_stringlist, char*)")

    def __init__(self, initial=None):
        c_ptr = self._alloc()
        super(StringList, self).__init__(c_ptr)

        if initial:
            for s in initial:
                if isinstance(s, bytes):
                    s.decode('ascii')
                if isinstance(s, string_types):
                    self.append(s)
                else:
                    raise TypeError("Item: %s not a string" % s)

    def __getitem__(self, index):
        if isinstance(index, int):
            length = len(self)
            if index < 0:
                index += length
            if index < 0 or index >= length:
                raise IndexError("index must be in range %d <= %d < %d" % (0, index, len(self)))
            else:
                return self._iget(self, index)
        else:
            raise TypeError("Index should be integer type")

    def append(self, string):
        if isinstance(string, bytes):
            s.decode('ascii')
        if isinstance(string, string_types):
            self._append(self, string)
        else:
            self._append(self, str(string))



class MetaWrapTest(ExtendedTestCase):

    def test_stringlist_wrap(self):
        items = ["A", "C", "E"]
        stringlist = StringList(items)
        self.assertEqual(len(stringlist), len(items))

        self.assertIn("free", StringList.__dict__)
        self.assertEqual(StringList.free.__name__, "stringlist_free")

        for index, item in enumerate(items):
            self.assertEqual(item, stringlist[index])


    def test_already_registered(self):
        with self.assertRaises(PrototypeError):
            Prototype.registerType("test_stringlist", None)

    def test_error_in_prototype_illegal_return_type(self):
        func = TestUtilPrototype("test_stringlist util_alloc_date_stamp_utc()")

        with self.assertRaises(PrototypeError):
            func()

    def test_prototype_known_return_type(self):
        stringlist = StringList(["B", "D", "F"])
        func = TestUtilPrototype("test_stringlist_ref stringlist_alloc_shallow_copy(stringlist)")
        result = func(stringlist)
        self.assertIsInstance(result, StringList)

        for index, value in enumerate(stringlist):
            self.assertEqual(result[index], value)


    def test_invalid_function(self):
        func = TestUtilPrototype("void stringlist_missing_function()")
        with self.assertRaises(PrototypeError):
            func()


    def test_invalid_prototype(self):
        func = TestUtilPrototype("void util_alloc_date_stamp_utc(")
        with self.assertRaises(PrototypeError):
            func()


    def test_function_type(self):
        def stringObj(c_ptr):
            char_ptr = ctypes.c_char_p(c_ptr)
            python_string = char_ptr.value
            TestUtilPrototype.lib.free(c_ptr)
            return python_string.decode('ascii')

        Prototype.registerType("string_obj", stringObj)

        dateStamp  = TestUtilPrototype("string_obj util_alloc_date_stamp_utc()")
        date_stamp = dateStamp()
        self.assertIsInstance(date_stamp, string_types)
