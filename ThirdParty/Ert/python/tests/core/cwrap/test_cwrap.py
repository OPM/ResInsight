import ctypes
import ert
from cwrap import CWrapper, BaseCClass, CWrapError
from ert.test  import ExtendedTestCase

test_lib  = ert.load("libert_util") # create a local namespace (so we don't overwrite StringList)
cwrapper =  CWrapper(test_lib)

class StringListTest(BaseCClass):
    def __init__(self):
        c_pointer = self.cNamespace().alloc()
        super(StringListTest, self).__init__(c_pointer)

    def free(self):
        StringListTest.cNamespace().free(self)

CWrapper.registerObjectType("stringlisttest", StringListTest)

StringListTest.cNamespace().alloc = cwrapper.prototype("c_void_p stringlist_alloc_new( )")
StringListTest.cNamespace().free  = cwrapper.prototype("void stringlist_free(stringlisttest )")


class CWrapTest(ExtendedTestCase):

    def test_return_type(self):
        stringlist_alloc = cwrapper.prototype("c_void_p stringlist_alloc_new( )")
        string_list1 = StringListTest()

        stringlist_alloc = cwrapper.prototype("stringlisttest_obj stringlist_alloc_new( )")
        string_list2 = stringlist_alloc()

        stringlist_alloc = cwrapper.prototype("stringlisttest_ref stringlist_alloc_new( )")
        string_list3 = stringlist_alloc()

        self.assertIsInstance(string_list1, StringListTest)
        self.assertIsInstance(string_list2, StringListTest)
        self.assertIsInstance(string_list3, StringListTest)

        self.assertFalse(string_list1.isReference())
        self.assertFalse(string_list2.isReference())
        self.assertTrue(string_list3.isReference())

        self.assertNotEqual(BaseCClass.from_param(string_list1), BaseCClass.from_param(string_list2))
        self.assertNotEqual(BaseCClass.from_param(string_list2), BaseCClass.from_param(string_list3))
        self.assertNotEqual(BaseCClass.from_param(string_list1), BaseCClass.from_param(string_list3))




    def test_class_variables(self):
        BaseCClass.cNamespace().hello = "BooYa!"
        StringListTest.cNamespace()["pow"] = "Badonkadonk!"

        with self.assertRaises(AssertionError):
            StringListTest.cNamespace().pow = "Badonkadonka!"

        with self.assertRaises(LookupError):
            self.assertFalse(BaseCClass.cNamespace()["pow"])

        with self.assertRaises(AttributeError):
            self.assertFalse(StringListTest.cNamespace().hello)

        self.assertEqual(StringListTest.cNamespace().pow, StringListTest.cNamespace()["pow"])


    def test_invalid_function(self):
        with self.assertRaises(CWrapError):
            func = cwrapper.prototype("void stringlist_missing_function( )")
    

    def test_invalid_prototype(self):
        with self.assertRaises(CWrapError):
            stringlist_alloc = cwrapper.prototype("c_void_p stringlist_alloc_new( ")


    def test_method_type(self):
        wrapper =  CWrapper(test_lib)
        def stringObj(c_ptr):
            char_ptr = ctypes.c_char_p( c_ptr )
            python_string = char_ptr.value
            test_lib.free(c_ptr)
            return python_string

        wrapper.registerType("string_obj", stringObj)

        dateStamp  = wrapper.prototype("string_obj util_alloc_date_stamp_utc()")
        date_stamp = dateStamp()
        self.assertIsInstance(date_stamp, str)




