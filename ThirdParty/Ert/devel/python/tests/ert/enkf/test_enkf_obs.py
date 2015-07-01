from ert.test import ErtTestContext
from ert.test import ExtendedTestCase

from ert.ecl import EclGrid, EclSum
from ert.sched import History

from ert.util import BoolVector,IntVector
from ert.enkf import ActiveMode, EnsembleConfig
from ert.enkf import ObsVector , LocalObsdata, EnkfObs, TimeMap


class EnKFObsTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/obs_testing/config")
        self.obs_config = self.createTestPath("Statoil/config/obs_testing/observations")
        self.obs_config2 = self.createTestPath("Statoil/config/obs_testing/observations2")
        self.refcase = self.createTestPath("Statoil/config/obs_testing/EXAMPLE_01_BASE")
        self.grid = self.createTestPath("Statoil/config/obs_testing/EXAMPLE_01_BASE.EGRID")

        
    def testObs(self):
        with ErtTestContext("obs_test", self.config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()

            self.assertEqual(31, len(obs))
            for v in obs:
                self.assertTrue(isinstance(v, ObsVector))

            with self.assertRaises(IndexError):
                v = obs[-1]

            with self.assertRaises(IndexError):
                v = obs[40]

            with self.assertRaises(KeyError):
                v = obs["No-this-does-not-exist"]

            v1 = obs["WWCT:OP_3"]
            v2 = obs["GOPT:OP"]
            mask = BoolVector(True, ert.getEnsembleSize())
            current_fs = ert.getEnkfFsManager().getCurrentFileSystem()

            self.assertTrue(v1.hasData(mask, current_fs))
            self.assertFalse(v2.hasData(mask, current_fs))
            
            local_node = v1.createLocalObs( )
            self.assertEqual( len(local_node.getStepList()) , len(v1.getStepList()))
            for t1,t2 in zip( local_node.getStepList() , v1.getStepList()):
                self.assertEqual( t1 , t2 )
                
            

    def test_obs_block_scale_std(self):
        with ErtTestContext("obs_test_scale", self.config_file) as test_context:
            ert = test_context.getErt()
            fs = ert.getEnkfFsManager().getCurrentFileSystem()
            active_list = IntVector( )
            active_list.initRange(0 , ert.getEnsembleSize() , 1 )

            obs = ert.getObservations()
            obs_data = LocalObsdata( "OBSxx" )
            obs_vector = obs["WWCT:OP_1"]
            obs_data.addObsVector( obs_vector )
            scale_factor = obs.scaleCorrelatedStd( fs , obs_data , active_list )

            for obs_node in obs_vector:
                for index in range(len(obs_node)):
                    self.assertEqual( scale_factor , obs_node.getStdScaling( index ))
            
            


    def test_obs_block_all_active_local(self):
        with ErtTestContext("obs_test_all_active", self.config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()
            obs_data = obs.getAllActiveLocalObsdata( )

            self.assertEqual( len(obs_data) , len(obs) )
            for obs_vector in obs:
                self.assertTrue( obs_vector.getObservationKey() in obs_data )

                tstep_list1 = obs_vector.getStepList()
                local_node = obs_data[ obs_vector.getObservationKey() ]

                self.assertTrue( tstep_list1 , local_node.getStepList())
                active_list = local_node.getActiveList()
                self.assertEqual( active_list.getMode() , ActiveMode.ALL_ACTIVE )
                    
                

    def test_create(self):
        ensemble_config = EnsembleConfig()
        obs = EnkfObs(ensemble_config)
        self.assertEqual( len(obs) , 0 )
        self.assertFalse( obs.load(self.obs_config) )
        self.assertEqual( len(obs) , 0 )

        
        time_map = TimeMap()
        obs = EnkfObs(ensemble_config , external_time_map = time_map)
        self.assertEqual( len(obs) , 0 )

        grid = EclGrid(self.grid)
        refcase = EclSum(self.refcase)

        history = History.alloc_from_refcase( refcase , False )
        obs = EnkfObs( ensemble_config , grid = grid , history = history )
        with self.assertRaises(IOError):
            obs.load("/does/not/exist")

        self.assertTrue( obs.load(self.obs_config) )
        self.assertEqual( len(obs) , 32 )
        obs.clear()
        self.assertEqual( len(obs) , 0 )
        
        obs.load(self.obs_config)
        self.assertEqual( len(obs) , 32 )
        self.assertFalse( "RFT2" in obs )
        obs.load(self.obs_config2)
        self.assertEqual( len(obs) , 33 )
        self.assertTrue( "RFT2" in obs )


        
    def test_ert_obs_reload(self):
        with ErtTestContext("obs_test_reload", self.config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()
            self.assertEqual( len(obs) , 31 )
            ert.loadObservations("observations2")
            self.assertEqual( len(obs) , 1 )

            ert.loadObservations("observations" , clear = False)
            self.assertEqual( len(obs) , 32 )
            
