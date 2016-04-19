from ert.cwrap import clib, Prototype
from ert.test.extended_testcase import ExtendedTestCase

# Local copies so that the real ones don't get changed
class TestUtilPrototype(Prototype):
    lib = clib.ert_load("libert_util")

    def __init__(self, prototype, bind=False):
        super(TestUtilPrototype, self).__init__(TestUtilPrototype.lib, prototype, bind=bind)


alloc_string_copy = TestUtilPrototype("cstring_obj util_alloc_string_copy(char*)")


class CStringTest(ExtendedTestCase):
    def test_get(self):
        s1 = "String123"
        s2 = alloc_string_copy(s1)
        self.assertEqual(s1, s2)
