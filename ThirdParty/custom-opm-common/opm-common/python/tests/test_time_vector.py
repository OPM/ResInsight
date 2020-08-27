import unittest
import datetime
from opm.tools import *

from opm.io.parser import Parser
try:
    from tests.utils import test_path, tmp
except ImportError:
    from utils import test_path, tmp

class TestTimeVector(unittest.TestCase):

    def setUp(self):
        pass


    def test_create(self):
        start_date = datetime.date(2018,1,1)
        start_datetime = datetime.datetime(2018,1,1)
        with self.assertRaises(ValueError):
            tv = TimeVector(start_date, base_string = "string", base_file="XYZ")

        tv = TimeVector( start_date )
        self.assertEqual(len(tv), 1)
        with self.assertRaises(IndexError):
            tv[1]

        passed_date = datetime.datetime(2000,1,1)
        with self.assertRaises(ValueError):
            tv.add_keywords(passed_date, [])

        next_date = datetime.datetime(2018,2,1)
        tv.add_keywords(next_date, ["KEY1"])
        self.assertEqual(len(tv), 2)

        middle_date = datetime.datetime(2018,1,15)
        tv.add_keywords(middle_date,[])
        self.assertEqual(len(tv) ,3)

        ts1 = tv[0]
        self.assertEqual(ts1.dt, start_datetime)

        tv.add_keywords(next_date, ["KEY2"])
        self.assertEqual(len(tv),3)

        ts = tv[-1]
        self.assertEqual(ts.keywords, ["KEY1", "KEY2"])

        self.assertIn(middle_date, tv)
        self.assertEqual(tv.dates, [start_datetime, middle_date, next_date])


        with self.assertRaises(KeyError):
            tv[datetime.datetime(1980,1,1)]

        ts1 = tv[next_date]
        ts2 = tv[datetime.date(next_date.year, next_date.month, next_date.day)]
        self.assertEqual(ts1,ts2)



    def test_load(self):
        tv = TimeVector(datetime.date(1997, 11, 6), base_file = test_path("data/schedule/part1.sch"))
        tv.load(test_path("data/schedule/part3.sch"))
        tv.load(test_path("data/schedule/fragment_dates.sch"))
        tv.load(test_path("data/schedule/part2.sch"))

        self.assertEqual(tv.dates, [datetime.datetime(1997, 11,  6),
                                    datetime.datetime(1997, 11, 14),
                                    datetime.datetime(1997, 12,  1),
                                    datetime.datetime(1997, 12, 17),
                                    datetime.datetime(1998,  1,  1),
                                    datetime.datetime(1998,  2,  1),
                                    datetime.datetime(1998,  3,  1),
                                    datetime.datetime(1998,  3, 29),
                                    datetime.datetime(1998,  3, 30),
                                    datetime.datetime(1998,  4,  1),
                                    datetime.datetime(1998,  4, 23),
                                    datetime.datetime(1998,  5,  1),
                                    datetime.datetime(1998,  5, 26),
                                    datetime.datetime(1998,  5, 27),
                                    datetime.datetime(1998,  6,  1),
                                    datetime.datetime(1998,  8,  1)])

    def test_str(self):
        tv = TimeVector(datetime.date(1997, 11, 6), base_string = open(test_path("data/schedule/part1.sch")).read())
        tv.load(test_path("data/schedule/part3.sch"))
        tv.load(test_path("data/schedule/part2.sch"))

        s = str(tv)
        tv2 = TimeVector(datetime.date(1997, 11, 6))
        tv2.load_string(s, date=datetime.datetime(1997, 11, 6))

        for ts1,ts2 in zip(tv,tv2):
            self.assertEqual(ts1.dt, ts2.dt)


    def test_optional(self):
        tv = TimeVector(datetime.date(1997, 11, 6), base_file = test_path("data/schedule/part1.sch"))

        # Must have a starting date, either as first keyword in loaded file,
        # or alternatively as the optional date argument.
        with self.assertRaises(ValueError):
            tv.load(test_path("data/schedule/fragment.sch"))

        with self.assertRaises(ValueError):
            tv.load(test_path("data/schedule/fragment_dates.sch"), date = datetime.datetime(1998, 1,1))

        tv.load(test_path("data/schedule/fragment.sch"), date = datetime.datetime(1998, 1, 10))
        ts = tv[-1]
        self.assertEqual(ts.dt, datetime.datetime(1998, 1 , 10))
        self.assertEqual(ts.keywords[0].name, "WCONINJE")



    def test_user_test(self):
       tv=TimeVector(datetime.date(1999,12,31))
       tv.load(test_path('data/schedule/TEMPLATE.SCH'), date=datetime.datetime(1999,12,31))
       self.assertListEqual(tv.dates, [datetime.datetime(1999,12,31),
                                       datetime.datetime(2000,1,1),
                                       datetime.datetime(2000,2,1),
                                       datetime.datetime(2000,3,1)])

    def test_no_leading_DATES(self):
        tv = TimeVector(datetime.date(1997, 11, 6), base_file=test_path("data/schedule/part1.sch"))
        s = str(tv)
        d = Parser().parse_string(s)
        kw0 = d[0]
        self.assertEqual(kw0.name, "WELSPECS")

        tv2 = TimeVector(datetime.date(2000,1,1))
        self.assertEqual("", str(tv2))

    def test_drop_dates(self):
        tv = TimeVector(datetime.date(1997, 11, 6), base_file=test_path("data/schedule/part1.sch"))
        with self.assertRaises(KeyError):
            tv.delete(datetime.datetime(2019,1,1))

        ts = tv[datetime.datetime(1997,11,14)]
        self.assertTrue("WTEST" in ts)
        tv.delete(datetime.datetime(1997,11,14))

        with self.assertRaises(KeyError):
            tv.delete(datetime.datetime(1997,11,14))

        for ts in tv:
            self.assertFalse("WTEST" in ts)

    def test_drop_dates2(self):
        tv = TimeVector(datetime.datetime(2017,1,1))
        tv.add_keywords(datetime.datetime(2018,1,1), ['FOO18'])
        tv.add_keywords(datetime.datetime(2019,1,1), ['FOO19'])
        tv.add_keywords(datetime.datetime(2020,1,1), ['FOO20'])
        tv.delete(datetime.datetime(2019,1,1))
        tv.delete(datetime.datetime(2020,1,1))


if __name__ == "__main__":
    unittest.main()

