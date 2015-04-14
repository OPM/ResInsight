import math 

from ert.enkf.export import GenDataCollector
from ert.test import ErtTestContext, ExtendedTestCase

class GenDataCollectorTest(ExtendedTestCase):


    def test_gen_data_collector(self):
        config = self.createTestPath("Statoil/config/with_GEN_DATA_RFT/config")
        with ErtTestContext("python/enkf/export/gen_data_collector", config) as context:
            ert = context.getErt()

            with self.assertRaises(KeyError):
                data = GenDataCollector.loadGenData(ert, "default" , "RFT_XX" , 5)

            with self.assertRaises(ValueError):
                data = GenDataCollector.loadGenData(ert, "default" , "RFT_WI_1" , 90)
            
            data1 = GenDataCollector.loadGenData(ert, "default" , "RFT_WI_1" , 9)
            data2 = GenDataCollector.loadGenData(ert, "default" , "RFT_WI_2" , 5)

            self.assertFloatEqual( data1[0][0] , 346.088074)
            self.assertFloatEqual( data1[24][1] , 364.461090)
            self.assertFloatEqual( data2[0][1] ,  263.419434)

            self.assertTrue( math.isnan( data2[0][0] ) )
