import ctypes
from ert.cwrap import CWrapper, CWrapperNameSpace, CClass, BaseCClass
from ert.util import UTIL_LIB
from ert_tests import ExtendedTestCase




class StringListTest(BaseCClass):
    def __init__(self):
        c_pointer = self.cNamespace().stringlist_alloc()
        super(StringListTest, self).__init__(c_pointer)

    def free(self):
        StringListTest.cNamespace().free(self)


class CWRapTest(ExtendedTestCase):
    def test_return_type(self):
        stringlist_alloc = cwrapper.prototype("c_void_p stringlist_alloc_new( )")
        string_list1 = StringListTest()

        stringlist_alloc = cwrapper.prototype("StringListObj stringlist_alloc_new( )")
        string_list2 = stringlist_alloc()

        stringlist_alloc = cwrapper.prototype("StringListRef stringlist_alloc_new( )")
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


CWrapper.registerType("stringlist", StringListTest)
CWrapper.registerType("StringListObj", StringListTest.createPythonObject)
CWrapper.registerType("StringListRef", StringListTest.createCReference)

cwrapper = CWrapper(UTIL_LIB)

StringListTest.cNamespace().stringlist_alloc = cwrapper.prototype("c_void_p stringlist_alloc_new( )")
StringListTest.cNamespace().free = cwrapper.prototype("void stringlist_free(stringlist)")

