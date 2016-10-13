from ert.test import ExtendedTestCase
from ert.util import UIReturn
from ert.util.enums import UIReturnStatusEnum


class UIReturnTest(ExtendedTestCase):
    def test_create(self):
        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_OK)
        self.assertTrue(ui_return)

        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_FAIL)
        self.assertFalse(ui_return)

        self.assertEqual(0, len(ui_return))

    def test_help(self):
        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_OK)
        self.assertEqual("", ui_return.help_text())

        ui_return.add_help("Help1")
        self.assertEqual("Help1", ui_return.help_text())

        ui_return.add_help("Help2")
        self.assertEqual("Help1 Help2", ui_return.help_text())

    def test_error_raises_OK(self):
        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_OK)
        with self.assertRaises(ValueError):
            ui_return.add_error("Error1")

        with self.assertRaises(ValueError):
            ui_return.last_error()

        with self.assertRaises(ValueError):
            ui_return.first_error()

    def test_add_error(self):
        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_FAIL)
        ui_return.add_error("Error1")
        ui_return.add_error("Error2")
        ui_return.add_error("Error3")
        self.assertEqual(3, len(ui_return))

        self.assertEqual("Error1", ui_return.first_error())
        self.assertEqual("Error3", ui_return.last_error())

    def test_iget_error(self):
        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_FAIL)
        ui_return.add_error("Error1")
        ui_return.add_error("Error2")
        ui_return.add_error("Error3")

        errorList = []
        for index in range(len(ui_return)):
            errorList.append(ui_return.iget_error(index))
        self.assertEqual(errorList, ["Error1", "Error2", "Error3"])

        with self.assertRaises(TypeError):
            ui_return.iget_error("XX")

        ui_return = UIReturn(UIReturnStatusEnum.UI_RETURN_OK)
        errorList = []
        for index in range(len(ui_return)):
            errorList.append(ui_return.iget_error(index))
        self.assertEqual(errorList, [])

    def test_status_enum(self):
        source_file_path = "libert_util/include/ert/util/ui_return.h"
        self.assertEnumIsFullyDefined(UIReturnStatusEnum, "ui_return_status_enum", source_file_path)
