import time
from ecl import EclDataType
from ecl.eclfile import EclKW, EclFile, openFortIO, FortIO
from ecl.grid import EclGrid
from ecl.gravimetry import EclGrav
from ecl.util.test import TestAreaContext
from tests import EclTest


class EclGravTest(EclTest):


    def setUp(self):
        self.grid = EclGrid.createRectangular( (10,10,10) , (1,1,1))
        
        
    def test_create(self):
        # The init file created here only contains a PORO field. More
        # properties must be added to this before it can be used for
        # any usefull gravity calculations.
        poro = EclKW( "PORO" , self.grid.getGlobalSize() , EclDataType.ECL_FLOAT )
        with TestAreaContext("grav_init"):
            with openFortIO( "TEST.INIT" , mode = FortIO.WRITE_MODE ) as f:
                poro.fwrite( f )
            self.init = EclFile( "TEST.INIT")

            grav = EclGrav( self.grid , self.init )


