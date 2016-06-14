from unittest import TestCase

from ert.test import TestAreaContext

from opm.parser import Parser
from opm.deck import Deck



class DeckTest(TestCase):
    def test_empty_deck(self):
        deck = Deck()
        self.assertEqual( len(deck) , 0 )

        with self.assertRaises(IndexError):
            deck[9]

        with self.assertRaises(KeyError):
            l = deck["EQUIL"]

        self.assertEqual( 0 , deck.numKeywords("EQUIL"))

        
    def test_deck(self):
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
                fileH.write("/\n")
                
            p = Parser( )
            deck = p.parseFile("test.DATA")
        self.assertEqual( len(deck) , 6 )
        self.assertEqual( 2 , deck.numKeywords("TSTEP"))
        tstep = deck["TSTEP"]
        self.assertEqual( len(tstep) , 2)

        slice_list = deck[0:2:6]
        self.assertEqual( len(slice_list) , 3)
        self.assertEqual( slice_list[0].name() , "RUNSPEC")
        self.assertEqual( slice_list[1].name() , "SCHEDULE")
        self.assertEqual( slice_list[2].name() , "TSTEP")

        dates_kw = deck[-1]
        self.assertEqual( dates_kw.name() , "DATES")
        
        with self.assertRaises(IndexError):
            deck["TSTEP" , 2]

        with self.assertRaises(TypeError):
            deck["TSTEP" , "X"]
        
        t0 = deck["TSTEP",0]
        t1 = deck["TSTEP",1]
        t1 = deck["TSTEP",-1]
        
            

        
