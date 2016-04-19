from ert.cwrap import Prototype, CFILE, clib
from ert.test.extended_testcase import ExtendedTestCase
from ert.test.test_area import TestAreaContext


# Local copies so that the real ones don't get changed
class TestUtilPrototype(Prototype):
    lib = clib.ert_load("libert_util")
    def __init__(self, prototype, bind=False):
        super(TestUtilPrototype, self).__init__(TestUtilPrototype.lib, prototype, bind=bind)

fileno = TestUtilPrototype("int fileno(FILE)")


class CFILETest(ExtendedTestCase):

    def test_cfile(self):
        with TestAreaContext("cfile_tests") as test_area:

            with open("test", "w") as f:
                f.write("some content")

            with open("test", "r") as f:
                cfile = CFILE(f)

                self.assertEqual(fileno(cfile), f.fileno())

    def test_cfile_error(self):
        with self.assertRaises(TypeError):
            cfile = CFILE("some text")
