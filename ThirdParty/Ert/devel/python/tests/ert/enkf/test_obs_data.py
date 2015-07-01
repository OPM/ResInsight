import datetime 

from ert.util import BoolVector , Matrix
from ert.test import TestAreaContext
from ert.test import ExtendedTestCase
from ert.enkf import ObsData , ObsBlock




class ObsDataTest(ExtendedTestCase):
    
    
    def test_create(self):
        obs_data = ObsData()
        obs_size = 10
        block = obs_data.addBlock("OBS" , obs_size)
        self.assertTrue( isinstance( block , ObsBlock ))

        block[0] = (10,10)
        block[1] = (12,12)
        D = obs_data.createDObs()
        self.assertTrue( isinstance(D , Matrix ))
        self.assertEqual( D.dims() , (2,1))
        
        self.assertEqual( D[0,0] , 10 )
        self.assertEqual( D[1,0] , 12 )
        
        obs_data.scaleMatrix( D )
        self.assertEqual( D[0,0] , 1 )
        self.assertEqual( D[1,0] , 1 )
        
        R = obs_data.createR()
        self.assertEqual( (2,2) , R.dims() )

        
        


