import os.path
from ert.ecl import EclGrid

from ert.test import ExtendedTestCase
from ert.test import ErtTestContext
from ert.enkf.local_ministep import LocalMinistep
from ert.enkf.active_list import ActiveList
from ert.enkf.local_obsdata import LocalObsdata
from ert.enkf.local_updatestep import LocalUpdateStep
from ert.enkf.local_obsdata_node import LocalObsdataNode
from ert.enkf import local_config
from ert.enkf.local_dataset import LocalDataset

class LocalConfigTest(ExtendedTestCase):
    
    def setUp(self):
               
        self.config = self.createTestPath("local/custom_kw/mini_config")
        
    def testLocalConfig(self):
                
        with ErtTestContext("python/enkf/data/local_config", self.config) as test_context:  

            main = test_context.getErt()
            self.assertTrue(main, "Load failed")
            
            local_config = main.getLocalConfig()
            analysis_module = main.analysisConfig().getModule("STD_ENKF")

            self.AllActive(local_config)

            local_config.clear()

            self.MiniStep(local_config, analysis_module)
            
            self.AttachMinistep(local_config)
            
            self.LocalDataset(local_config)
            
            self.LocalObsdata(local_config)
           
            local_config_file_summary = "local_config_summary.txt"
            local_config.writeSummaryFile( local_config_file_summary )
            self.assertTrue( os.path.isfile( local_config_file_summary ))

            self.clear(local_config)

            grid = local_config.getGrid()
            self.assertTrue( isinstance( grid , EclGrid ))
            
    def clear(self, local_config):
        local_config.clear()
        updateStep = local_config.getUpdatestep( )
        self.assertEqual( len(updateStep) , 0 )
        
        

    def AllActive(self , local_config):
        updateStep = local_config.getUpdatestep( )
        ministep = updateStep[0]
        self.assertEqual( 1 , len(ministep) )
        dataset = ministep["ALL_DATA"]
        
        self.assertTrue( "PERLIN_PARAM" in dataset )

        obsdata = ministep.getLocalObsData()
        self.assertEqual( len(obsdata) , 3 )
        
 
    def MiniStep( self, local_config, analysis_module ):                        
            
        # Ministep        
        ministep = local_config.createMinistep("MINISTEP", analysis_module)
        self.assertTrue(isinstance(ministep, LocalMinistep))

        self.assertFalse( "DATA" in ministep )
        with self.assertRaises(KeyError):
            ministep["DATA"]

                 
    def AttachMinistep( self, local_config):                        
            
        # Update step
        updatestep = local_config.getUpdatestep( )
        self.assertTrue(isinstance(updatestep, LocalUpdateStep))
        n1 = len(updatestep)

        # Ministep                                      
        ministep = local_config.createMinistep("MINISTEP")
        self.assertTrue(isinstance(ministep, LocalMinistep))   
        
        # Attach
        updatestep.attachMinistep(ministep)
        self.assertTrue(isinstance(updatestep[0], LocalMinistep))
        
        self.assertEqual( len(updatestep) , n1 + 1 )            


    def LocalDataset( self, local_config ):   
                             
        # Creating dataset
        data_scale = local_config.createDataset("DATA_SCALE")
        self.assertTrue(isinstance(data_scale, LocalDataset))
        
        # Try to add existing dataset
        with self.assertRaises(ValueError):
            local_config.createDataset("DATA_SCALE")
        
        with self.assertRaises(KeyError):
            data_scale.addNode("MISSING")

        data_scale.addNode("PERLIN_PARAM")
        active_list = data_scale.getActiveList("PERLIN_PARAM")
        self.assertTrue(isinstance(active_list, ActiveList))
        active_list.addActiveIndex(0)            

        self.assertTrue("PERLIN_PARAM" in data_scale)
        self.assertFalse("MISSING" in data_scale)

        ministep = local_config.createMinistep("MINISTEP")
        ministep.attachDataset( data_scale )
        self.assertTrue( "DATA_SCALE" in ministep )
        data_scale_get = ministep["DATA_SCALE"]
        self.assertTrue( "PERLIN_PARAM" in data_scale_get )

        # Error when adding existing data node
        with self.assertRaises(KeyError):
            data_scale.addNode("PERLIN_PARAM")
        

        
    def LocalObsdata( self, local_config ):              
          
        # Creating obsdata
        local_obs_data_1 = local_config.createObsdata("OBSSET_1") 
        self.assertTrue(isinstance(local_obs_data_1, LocalObsdata))
        
        # Try to add existing obsdata
        with self.assertRaises(ValueError):
            local_config.createObsdata("OBSSET_1")

        # Add node with range
        with self.assertRaises(KeyError):
            local_obs_data_1.addNodeAndRange("MISSING_KEY" , 0 , 1 )
            
        local_obs_data_1.addNodeAndRange("GEN_PERLIN_1", 0, 1)
        local_obs_data_1.addNodeAndRange("GEN_PERLIN_2", 0, 1)            
        
        self.assertEqual( len(local_obs_data_1) , 2 )
                
        # Delete node        
        del local_obs_data_1["GEN_PERLIN_1"]
        self.assertEqual( len(local_obs_data_1) , 1 )  

        # Get node
        node = local_obs_data_1["GEN_PERLIN_2"]
        self.assertTrue(isinstance(node, LocalObsdataNode))

        # Add node again with no range and check return type
        node_again = local_obs_data_1.addNode("GEN_PERLIN_1")
        self.assertTrue(isinstance(node_again, LocalObsdataNode))
        
        # Error when adding existing obs node
        with self.assertRaises(KeyError):
            local_obs_data_1.addNode("GEN_PERLIN_1")

            
    def AttachObsData( self , local_config):          
                  
        local_obs_data_2 = local_config.createObsdata("OBSSET_2") 
        self.assertTrue(isinstance(local_obs_data_2, LocalObsdata))
        
        # Obsdata                                           
        local_obs_data_2.addNodeAndRange("GEN_PERLIN_1", 0, 1)
        local_obs_data_2.addNodeAndRange("GEN_PERLIN_2", 0, 1)  
        
        # Ministep                                      
        ministep = local_config.createMinistep("MINISTEP")
        self.assertTrue(isinstance(ministep, LocalMinistep))   

        # Attach obsset                                
        ministep.attachObsset(local_obs_data_2)                     
                    
        # Retrieve attached obsset            
        local_obs_data_new = ministep.getLocalObsData()                   
        self.assertEqual( len(local_obs_data_new) , 2 )                    
                            
