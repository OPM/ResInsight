import datetime
from operator import attrgetter
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

from opm.io.parser import Parser

# This is from the TimeMap.cpp implementation in opm
ecl_month = {"JAN" : 1,
             "FEB" : 2,
             "MAR" : 3,
             "APR" : 4,
             "MAI" : 5,
             "MAY" : 5,
             "JUN" : 6,
             "JLY" : 7,
             "JUL" : 7,
             "AUG" : 8,
             "SEP" : 9,
             "OCT" : 10,
             "OKT" : 10,
             "NOV" : 11,
             "DEC" : 12,
             "DES" : 12}

inv_ecl_month = {1 : "JAN",
                 2 : "FEB",
                 3 : "MAR",
                 4 : "APR",
                 5 : "MAY",
                 6 : "JUN",
                 7 : "JUL",
                 8 : "AUG",
                 9 : "SEP",
                 10: "OCT",
                 11 : "NOV",
                 12 : "DEC"}

def _make_datetime(dates_record):
    day = dates_record[0].get_int(0)
    month = dates_record[1].get_str(0)
    year = dates_record[2].get_int(0)

    return datetime.datetime(year, ecl_month[month], day)

class TimeStep(object):

    def __init__(self, dt, keywords):
        """The TimeStep class consist of a list of keywords and a corresponding date.

        Observe that the date value corresponds to a DATES / TSTEP keyword
        following *after* the keywords; i.e. if the TimeStep instance contains
        a WCONHIST keyword the settings in that keyword should apply *until*
        the date specified is reached. See the documentation of the TimeVector
        class for more details of the relationship between TimeVector and
        TimeStep.

        """
        self.dt = dt
        self.keywords = keywords
        self.tstep = None
        self.is_start = False


    @classmethod
    def create_first(cls, dt):
        ts = cls(dt, [])
        ts.is_start = True
        return ts

    def add_keyword(self, kw):
        self.keywords.append(kw)



    def __len__(self):
        return len(self.keywords)

    def __contains__(self, arg):
        for kw in self.keywords:
            if arg == kw.name:
                return True
        return False


    def __str__(self):
        string = StringIO()

        if not self.is_start:
            day = self.dt.day
            month = self.dt.month
            year = self.dt.year
            string.write("DATES\n  {day} '{month}' {year}/\n/\n\n".format( day=day, month = inv_ecl_month[month], year=year))

        for kw in self.keywords:
            string.write(str(kw))
            string.write("\n")

        return string.getvalue()


class TimeVector(object):

    def __init__(self, start_date, base_string = None, base_file = None):
        """The TimeVector class is a simple vector class with DATES/TSTEP blocks.

        The TimeVector class is a basic building block for tools designed to
        update schedule files. A schedule file consists of a list of keywords
        related to the dynamic properties of the field, like opening and
        closing wells, specifiying rates and so on. The temporal advancement of
        the simulator is controlled by DATES and TSTEP keywords. A typical
        schedule section can look like this:

        --- Step 1 -----------------------

        WELSPECS
           'C1'   'G1'   10 10 10 'OIL' /
        /

        COMPDAT
           'C1' 15 20 10 16 'OPEN' /
           'C1' 15 21 16 16 'OPEN' /
        /

        WCONHIST
           'C1' 'OPEN'  'ORAT' 1000 /
        /

        --- Step 2 ----------------------

        DATES
           10  'MAY' 2016 /
        /

        WCONHIST
           'C1'  'OPEN'  'ORAT' 2000 /
        /

       --- Step 3 ----------------------

        TSTEP
            10 /

        WELSPECS
           'W2'  'G1'  5 5 5 'OIL' /
        /

        COMPDAT
           'W2' 10 10 7 10 'OPEN' /
        /

        WCONHIST
           'C1' 'OPEN' 'ORAT' 3000 /
           'W2' 'OPEN' 'ORAT' 1500 /
        /

        --- Step 4 ----------------------

        DATES
           30 'MAY' 2016 /
        /

        As indicated above the DATES and TSTEP keywords act as delimiters in
        the schedule file. In the TimeVector class the fundamental unit is
        TimeStep instance which consists of a list of keywords, and a
        terminating DATES or TSTEP keyword, the example above would correspond
        to a TimeVector with three TimeStep instances.

        Basic usage example:

           #!/usr/bin/env python
           from opm.tools import TimeVector

           # Create vector and load history.
           tv = TimeVector( start )
           tv.load("history.sch")


           # Load predictions from another file
           tv.load("prediction.sch")


           # Insert the definition of one particular well at
           # a specifed date.
           tv.load("extra_wll.sch", date = datetime.datetime(2018,10,1))


           # Check if we have a certain timestep:
           if datetime.datetime(2017,1,1) in tv:
               print("We have it!")
           else:
               print("No such date")


           # Dump the updated schedule content to a file:
           with open("schedule","w") as f:
                f.write(str(tv))


        """
        if base_string and base_file:
            raise ValueError("Can only supply one of base_string and base_file arguments")

        self.start_date = datetime.datetime( start_date.year, start_date.month, start_date.day)
        self.time_steps_dict = {}
        self.time_steps_list = []

        ts = TimeStep.create_first(self.start_date)

        self._add_dates_block(ts)
        start_dt = datetime.datetime(start_date.year, start_date.month, start_date.day)
        if base_file:
            deck = Parser().parse(base_file)
            self._add_deck(deck, start_dt)

        if base_string:
            deck = Parser().parse_string(base_string)
            self._add_deck(deck, start_dt)


    def __len__(self):
        """
        The number of timesteps in the vector.
        """
        return len(self.time_steps_dict)

    def __contains__(self, dt):
        """
        Will return true if the vector contains a timestep at date dt.
        """
        if isinstance(dt, datetime.date):
            dt = datetime.datetime(dt.year, dt.month, dt.day)
        return dt in self.time_steps_dict


    def __getitem__(self, index):
        """Will look up a timestep in the vector.

        The index argument can either be an integer or a datetime instance.

        """
        if isinstance(index,int):
            return self.time_steps_list[index]
        else:
            if isinstance(index,datetime.date):
                index = datetime.datetime(index.year, index.month, index.day)
            return self.time_steps_dict[index]


    def _add_dates_block(self, ts):
        self.time_steps_dict[ts.dt] = ts
        self.time_steps_list.append(ts)

    def delete(self, dt):
        del self.time_steps_dict[dt]
        for (index,ts) in enumerate(self.time_steps_list):
            if ts.dt == dt:
                del self.time_steps_list[index]
                break


    def add_keywords(self, dt, keywords):
        if dt < self.start_date:
            raise ValueError("Invalid datetime argument: {}".format(dt))

        if dt in self.time_steps_dict:
            ts = self[dt]
            for kw in keywords:
                ts.add_keyword(kw)
        else:
            ts = TimeStep(dt, keywords)
            self._add_dates_block(ts)
            self.time_steps_list.sort( key = attrgetter("dt"))


    def _add_deck(self, deck, start_date):
        first_kw = deck[0]
        if start_date is None:
            if first_kw.name != "DATES":
                raise ValueError("When loading you must *either* specify date - or file must start with DATES keyword")
            dt = _make_datetime(first_kw[len(first_kw) - 1])
        else:
            if first_kw.name == "DATES":
                raise ValueError("When loading you must *either* specify date - or file must start with DATES keyword")
            dt = start_date

        keywords = []
        for kw in deck:

            if kw.name == "DATES":
                self.add_keywords(dt, keywords)

                for index in range(len(kw)-1):
                    dt = _make_datetime(kw[index])
                    self.add_keywords(dt, [])

                dt = _make_datetime(kw[len(kw)-1])

                keywords = []
                continue

            #if kw.name == "TSTEP":
            #raise ValueError("Must block the ranges with active TSTEP - getting a DATES in there is ERROR")

            keywords.append(kw)

        self.add_keywords(dt, keywords)


    def load(self, filename, date = None):
        """Will parse a Schedule file and add the keywords to the current TimeVector.

        You can call the load() method repeatedly, the different timesteps will
        be ordered chronologically. If a timestep is already present the
        keywords will be appended.

        The optional date argument can be used to insert schedule file
        fragments which do not have any DATES / TSTEP keywords. Assuming you
        have a base file 'base.sch' and a small fragment 'well.sch' with the
        WELSPECS and COMPDAT keywords to create one well, then the new well can
        be added 1.st of April 2017 as this:

            tv = TimeVector( start )
            tv.load("base.sch")
            tv.load("well.sch", date = datetime.datetime(2017, 4, 1))

        """
        deck = Parser().parse(filename)
        self._add_deck(deck, date)


    def load_string(self, deck_string, date = None):
        """
        Like load() - but load from a string literal instead of file.
        """
        deck = Parser().parse_string(deck_string)
        self._add_deck(deck, date)


    def __str__(self):
        """Will return a string representation of the vector.

        The output from this method should be valid Schedule input which can be
        passed to a simulator.

        """

        string = StringIO()
        for ts in self:
            string.write(str(ts))

        return string.getvalue()




    @property
    def dates(self):
        """
        Will return a list of all the dates in the vector.
        """
        return [ x.dt for x in self.time_steps_list ]
