import os.path

from ert.test import ExtendedTestCase, TestAreaContext

from ert.enkf import GenObservation,GenDataConfig,ActiveList




class GenObsTest(ExtendedTestCase):
    def setUp(self):
        pass


    def test_create(self):
        data_config = GenDataConfig("KEY")
        with self.assertRaises(ValueError):
            gen_obs = GenObservation("KEY" , data_config )

        with TestAreaContext("gen_obs/create"):
            with open("obs1.txt","w") as f:
                f.write("10  5  12 6\n")
                
            with self.assertRaises(ValueError):
                gen_obs = GenObservation("KEY" , data_config , scalar_value = (1,2) , obs_file = "obs1.txt")

            with self.assertRaises(TypeError):
                gen_obs = GenObservation("KEY" , data_config , scalar_value = 1 )

            with self.assertRaises(IOError):
                gen_obs = GenObservation("KEY" , data_config , obs_file = "does/not/exist" )
            
            gen_obs = GenObservation("KEY" , data_config , obs_file = "obs1.txt" , data_index = "10,20")
            self.assertEqual( len(gen_obs) , 2 )
            self.assertEqual( gen_obs[0] , (10,5) )
            self.assertEqual( gen_obs[1] , (12,6) )
            
            self.assertEqual( gen_obs.getValue(0) , 10 )
            self.assertEqual( gen_obs.getDataIndex(1) , 20 )
            self.assertEqual( gen_obs.getStdScaling(0) , 1 )
            self.assertEqual( gen_obs.getStdScaling(1) , 1 )

            active_list = ActiveList( )
            gen_obs.updateStdScaling( 0.25 , active_list )
            self.assertEqual( gen_obs.getStdScaling(0) , 0.25 )
            self.assertEqual( gen_obs.getStdScaling(1) , 0.25 )
            
            active_list.addActiveIndex( 1 )
            gen_obs.updateStdScaling( 2.00 , active_list )
            self.assertEqual( gen_obs.getStdScaling(0) , 0.25 )
            self.assertEqual( gen_obs.getStdScaling(1) , 2.00 )
            
