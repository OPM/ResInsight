import datetime 

from ert.util import BoolVector
from ert.test import TestAreaContext
from ert.test import ExtendedTestCase
from ert.enkf import ObsBlock



class ObsBlockTest(ExtendedTestCase):


    def test_create(self):
        block = ObsBlock("OBS" , 1000)
        self.assertTrue( isinstance( block , ObsBlock ))
        self.assertEqual( 1000 , block.totalSize())
        self.assertEqual( 0 , block.activeSize())
        
        
        
    def test_access(self):
        obs_size = 10
        block = ObsBlock("OBS" , obs_size)

        with self.assertRaises(IndexError):
            block[100] = (1,1)

        with self.assertRaises(IndexError):
            block[-100] = (1,1)

        with self.assertRaises(TypeError):
            block[4] = 10

        with self.assertRaises(TypeError):
            block[4] = (1,1,9)

        #------

        with self.assertRaises(IndexError):
            v = block[100]

        with self.assertRaises(IndexError):
            v = block[-100]

        block[0] = (10,1)
        v = block[0]
        self.assertEqual( v , (10,1))
        self.assertEqual( 1 , block.activeSize())

        block[-1] = (17,19)
        self.assertEqual( block[-1], (17,19))
