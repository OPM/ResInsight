from unittest import TestCase

from ert.test import TestAreaContext

from opm.parser import Parser
from opm.deck import Deck, DeckKeyword, DeckRecord , DeckItem



class DeckItemTest(TestCase):
        
    def test_from_deck(self):
        with TestAreaContext("parse-test"):
            with open("test.DATA", "w") as fileH:
                fileH.write("RUNSPEC\n")
                fileH.write("DIMENS\n")
                fileH.write(" 10 10 10 /\n")
                fileH.write("SCHEDULE\n")
                fileH.write("TSTEP\n")
                fileH.write("  10 10 /\n")
                fileH.write("TSTEP\n")
                fileH.write("  20 20 /\n")
                fileH.write("DATES\n")
                fileH.write("  10 'MAY' 2017 /\n")
                fileH.write("  15 'MAY' 2017 /\n")
                fileH.write("/\n")
                
            p = Parser( )
            deck = p.parseFile("test.DATA")

        dates_kw = deck[-1]
        record = dates_kw[0]
        day_item = record[0]
        year_item = record["YEAR"]
        month_item = record[1]
        
        self.assertTrue( isinstance( day_item , DeckItem ))
        self.assertTrue( isinstance( year_item , DeckItem ))

        self.assertEqual( day_item[0] , 10 )
        self.assertEqual( month_item[0] , "MAY")
        self.assertEqual( year_item[0] , 2017)

