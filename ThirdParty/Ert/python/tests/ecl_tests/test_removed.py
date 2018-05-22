import time
import datetime

from ecl.util.test import TestAreaContext
from tests import EclTest
from ecl import EclDataType
from ecl.eclfile import EclFile, EclKW, openFortIO, FortIO



class Removed_2_1_Test(EclTest):
    def test_ecl_file_block(self):

        with TestAreaContext("name") as t:
            kw = EclKW("TEST", 3, EclDataType.ECL_INT)
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                kw.fwrite( f )

            t.sync()
            
            f = EclFile( "TEST" )
            with self.assertRaises(NotImplementedError):
                f.select_block( "KW" , 100 )

            with self.assertRaises(NotImplementedError):
                f.select_global( )

            with self.assertRaises(NotImplementedError):
                f.select_restart_section( index = None , report_step = None , sim_time = None)

            with self.assertRaises(NotImplementedError):
                f.select_restart_section( )

            with self.assertRaises(NotImplementedError):
                EclFile.restart_block( "TEST" )
                
