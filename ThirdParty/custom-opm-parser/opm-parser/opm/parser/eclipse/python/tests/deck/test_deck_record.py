from unittest import TestCase

from ert.test import TestAreaContext

from opm.parser import Parser
from opm.deck import Deck, DeckKeyword, DeckRecord



class DeckRecordTest(TestCase):
        
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
        self.assertTrue( isinstance( record , DeckRecord ))
        self.assertEqual( len(record) , 4 )

        with self.assertRaises(IndexError):
            record[100]

        with self.assertRaises(KeyError):
            record["NO-NOT-THAT-ITEM"]

        self.assertTrue( "DAY" in record )
        self.assertFalse( "XXX" in record )
        
