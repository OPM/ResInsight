import ecl
from cwrap import Prototype
from ecl.test.extended_testcase import ExtendedTestCase

# Local copies so that the real ones don't get changed
class TestUtilPrototype(Prototype):
    lib = ecl.load("libecl")

    def __init__(self, prototype, bind=False):
        super(TestUtilPrototype, self).__init__(TestUtilPrototype.lib, prototype, bind=bind)

alloc_string_copy = TestUtilPrototype("char* util_alloc_string_copy(char*)")

class CStringTest(ExtendedTestCase):

    def test_get(self):
        # this test can safely be deleted, as the type it used to test,
        # cstring_obj, has been replaced with char*
        s1 = "String123"
        print('test_get.s1=' + s1)
        s2 = alloc_string_copy(s1)
        print('test_get.s2=' + s2)
        self.assertEqual(s1, s2)
        print('test_get.s1==s2: %s' % (s1 == s2))
