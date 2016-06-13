from unittest import TestCase

from ert.test import TestAreaContext

from opm.parser import Parser
from opm.deck import Deck, DeckKeyword, DeckRecord



class DeckKeywordTest(TestCase):
    def test_empty_keyword(self):
        kw = DeckKeyword("MYKW")
        self.assertEqual( len(kw) , 0 )

        with self.assertRaises(IndexError):
            kw[9]

        self.assertEqual( kw.name() , "MYKW")
        
        
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

        self.assertEqual( len(deck) , 6 )
        dates_kw = deck[-1]
        self.assertEqual( dates_kw.name() , "DATES")
        self.assertEqual( len(dates_kw) , 2 )
        runspec_kw = deck["RUNSPEC"][0]
        self.assertEqual( len(runspec_kw) , 0 )

        with self.assertRaises( IndexError ):
            runspec_kw[0]

        with self.assertRaises( IndexError ):
            dates_kw[2]

        record = dates_kw[0]
        self.assertTrue( isinstance( record , DeckRecord ))
        
