import os
from ert.enkf import ErtLog
from ert.test import ExtendedTestCase, TestAreaContext


class ErtLogTest(ExtendedTestCase):

    def test_log(self):
        with TestAreaContext("python/ert_log/log") as work_area:
            test_log_filename = "test_log"
            ErtLog.init(1, test_log_filename, True)
            message = "This is fun"
            ErtLog.log(1, message)

            self.assertTrue(os.path.isfile(test_log_filename))

            with open(test_log_filename, "r") as f:
                text = f.readlines()
                self.assertTrue(len(text) > 0)
                self.assertTrue(message in text[-1])


    def test_getFilename(self):
        with TestAreaContext("python/ert_log/log") as work_area:
            test_log_filename = "log_test_file.txt"
            ErtLog.init(1, test_log_filename, True)
            message = "This is fun"
            ErtLog.log(1, message)

            self.assertEqual(ErtLog.getFilename(), test_log_filename)

