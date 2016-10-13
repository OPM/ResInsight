from unittest import TestCase
from ert.test import TestAreaContext

from opm.parser import Parser
from opm.ecl_state.tables import TableManager



class TableManagerTest(TestCase):
    def test_create(self):
        with TestAreaContext("table-manager"):
            p = Parser()
            with open("test.DATA", "w") as fileH:
                fileH.write("""
TABDIMS
 2 /

SWOF
 1 2 3 4
 5 6 7 8 /
 9 10 11 12 /
""")
            
            deck = p.parseFile( "test.DATA")
            table_manager = TableManager( deck )

            
            self.assertTrue(  table_manager.hasTable( "SWOF" ))
            self.assertFalse( table_manager.hasTable( "SGLF" ))
            self.assertTrue( "SWOF" in table_manager )
            self.assertFalse( "SGLF" in table_manager )

            with self.assertRaises(KeyError):
                table_manager.getTable("SGFL")

            self.assertEqual( table_manager.numTables("SLGF") , 0 )
            self.assertEqual( table_manager.numTables("SWOF") , 2 )

            with self.assertRaises(IndexError):
                table = table_manager.getTable("SWOF", num = 10)

            


    def test_table(self):
        with TestAreaContext("table-manager"):
            p = Parser()
            with open("test.DATA", "w") as fileH:
                fileH.write("""
TABDIMS
 2 /

SWOF
 1 2 3 4
 5 6 7 8 /
 9 10 11 12 /
""")
            
            deck = p.parseFile( "test.DATA")
            table_manager = TableManager( deck )
            table = table_manager.getTable("SWOF")

            self.assertTrue("SW" in table )
            self.assertFalse(table.hasColumn("JOE"))

            with self.assertRaises(KeyError):
                table.getValue("NO_NOT_THIS" , 1)

            with self.assertRaises(IndexError):
                table.getValue("SW" , 100)

            self.assertEqual( table.getValue("SW" , 0 ) , 1)
            self.assertEqual( table.getValue("SW" , 1 ) , 5)

            self.assertEqual( table.evaluate("KRW" , 1) , 2)

            index = table.lookup( 1 )
            self.assertEqual( table.evaluate("KRW" , index) , 2)
            
