from unittest import TestCase

from ert.test import TestAreaContext

from opm.parser import Parser,ParseContext



class ParserTest(TestCase):
    def test_parse(self):
        p = Parser()
        pm = ParseContext()
        with self.assertRaises(IOError):
            p.parseFile("does/not/exist" , pm)

        with TestAreaContext("parse-test"):
            with open("test.DATA", "w") as fileH:
                fileH.write("RUNSPEC\n")
                fileH.write("DIMENS\n")
                fileH.write(" 10 10 10 /\n")
        
            deck = p.parseFile( "test.DATA" , pm)
            self.assertEqual( len(deck) , 2 )

            deck = Parser.parseFile( "test.DATA" )
            
            
    def test_add_keyword(self):
        p = Parser()
        schema = {'name' : 'KEYWORD' , 'size' : 1 , 'items' : [{'name' : 'File' , 'value_type' : 'STRING'}] , 'sections' : ["SCHEDULE"]}
        p.addKeyword( schema )
        self.assertTrue("KEYWORD" in p)


        with TestAreaContext("parse-test2"):
            with open("test.DATA", "w") as fileH:
                fileH.write("KEYWORD\n")
                fileH.write(" ARG /\n")
                

            deck = p.parseFile( "test.DATA" )
            self.assertTrue( "KEYWORD" in deck )
        
            
