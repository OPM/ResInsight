from datetime import datetime
from unittest2 import TestCase
from ert.util import ctime


class CTimeTest(TestCase):

    def test_c_time(self):
        c_time = ctime(0)
        self.assertEqual(str(c_time), "1970-01-01 01:00:00")

        date_time = ctime(datetime(1970, 1, 1, 1, 0, 0))
        self.assertEqual(c_time, date_time)

        date_time_after = ctime(datetime(1970, 1, 1, 1, 0, 5))

        self.assertTrue(date_time_after > date_time)