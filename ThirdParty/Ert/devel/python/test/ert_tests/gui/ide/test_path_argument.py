from ert_gui.ide.keywords.definitions import PathArgument
from ert.test import ExtendedTestCase


class PathArgumentTest(ExtendedTestCase):

    def test_path_argument(self):

        path_arg = PathArgument()

        validation_status = path_arg.validate("/non_existing_file")
        self.assertFalse(validation_status)

        validation_status = path_arg.validate(__file__)
        self.assertTrue(validation_status)
