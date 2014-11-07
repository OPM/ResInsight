from datetime import datetime, date

from ert.util import CTime


try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase


class CTimeTest(TestCase):

    def test_creation(self):
        t0 = CTime(-60 * 60)

        t1 = CTime(t0)
        self.assertEqual(t0, t1)

        t2 = CTime(datetime(1970, 1, 1, 0))
        self.assertEqual(t0, t2)

        t3 = CTime(date(1970, 1, 1))
        self.assertEqual(t0, t3)

        with self.assertRaises(NotImplementedError):
            CTime("string")


    def test_c_time(self):
        c_time = CTime(0)
        self.assertEqual(str(c_time), "1970-01-01 01:00:00")

        date_time = CTime(datetime(1970, 1, 1, 1, 0, 0))
        self.assertEqual(c_time, date_time)

        date_time_after = CTime(datetime(1970, 1, 1, 1, 0, 5))

        self.assertTrue(date_time_after > date_time)


    def test_math(self):
        c1 = CTime(date(2000, 1, 1))
        c2 = CTime(date(2000, 1, 1))
        c3 = CTime(date(2000, 1, 1))

        c3 += c1
        self.assertTrue(isinstance(c3, CTime))

        c4 = c1 * 1.0
        self.assertTrue(isinstance(c4, CTime))
        self.assertTrue(isinstance(c1 + c2, CTime))

        self.assertEqual((c1 + c2) * 0.5, date(2000, 1, 1))


    def test_comparison(self):
        t0 = CTime(0)
        t1 = CTime(0)
        t2 = CTime(1)

        self.assertTrue(t0 == t1)
        self.assertFalse(t0 != t1)
        with self.assertRaises(TypeError):
            t0 != 0.5

        self.assertFalse(t0 < t1)
        self.assertTrue(t0 < t2)
        with self.assertRaises(TypeError):
            t0 < 0.5

        self.assertTrue(t0 <= t1)
        self.assertTrue(t0 <= t2)
        with self.assertRaises(TypeError):
            t0 <= 0.5

        self.assertFalse(t0 > t1)
        self.assertFalse(t0 > t2)
        with self.assertRaises(TypeError):
            t0 > 0.5

        self.assertTrue(t0 >= t1)
        self.assertFalse(t0 >= t2)
        with self.assertRaises(TypeError):
            t0 >= 0.5

        t3 = CTime(date(2050 , 1 , 1 ))
        t4 = CTime(date(2060 , 1 , 1 ))
        self.assertTrue( t1 < t3 )
        self.assertTrue( t3 < t4 )

        t5 = CTime(t4)
        self.assertTrue( t4 == t5 )



    def test_range(self):
        d1 = date(2000, 1, 1)
        dt1 = datetime(2000, 1, 1, 0, 0, 0)
        c1 = CTime(d1)

        d0 = date(1999, 1, 1)
        dt0 = datetime(1999, 1, 1, 0, 0, 0)
        c0 = CTime(d0)

        d2 = date(2001, 1, 1)
        dt2 = datetime(2001, 1, 1, 0, 0, 0)
        c2 = CTime(d2)

        self.assertTrue(d0 <= c1 < dt2)
        self.assertTrue(c0 <= c1 < d2)
        self.assertTrue(dt0 <= c1 < c2)

        self.assertFalse(d1 <= c0 < dt2)
        self.assertFalse(c1 <= c0 < d2)
        self.assertFalse(dt1 <= c0 < c2)

        self.assertTrue(d0 <= c0 < dt2)
        self.assertTrue(c0 <= c0 < d2)
        self.assertTrue(dt0 <= c0 < c2)

        self.assertFalse(d0 <= c2 < dt2)
        self.assertFalse(c0 <= c2 < d2)
        self.assertFalse(dt0 <= c2 < c2)

        self.assertTrue(d0 <= c2 <= dt2)
        self.assertTrue(c0 <= c2 <= d2)
        self.assertTrue(dt0 <= c2 <= c2)

