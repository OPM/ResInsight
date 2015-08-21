#!/usr/bin/env python
#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_labscale.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.


from ert.enkf import ObsVector
from ert.test import ExtendedTestCase , ErtTestContext



class LabScaleTest(ExtendedTestCase):


    def testObs(self):
        config_file = self.createTestPath("Statoil/config/labscale/config")
        with ErtTestContext("labscale", config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()

            self.assertEqual(4, len(obs))
            for v in obs:
                self.assertTrue(isinstance(v, ObsVector))

            v1 = obs["WWCT_1"]
            self.assertEqual( v1.activeStep() , 5 )
            node = v1.getNode( 5 )
            self.assertFloatEqual( node.getValue() , 0.00)

            v2 = obs["WWCT_2"]
            self.assertEqual( v2.activeStep() , 31 )
            node = v2.getNode( 31 )
            self.assertFloatEqual( node.getValue() , 0.575828)

            v3 = obs["WWCT_3"]
            self.assertEqual( v3.activeStep() , 73 )
            node = v3.getNode( 73 )
            self.assertFloatEqual( node.getValue() , 1.00)


            bpr = obs["BPR"]
            self.assertEqual( bpr.activeStep() , 31 )
            node = bpr.getNode( 31 )
            self.assertFloatEqual( node.getValue(0) , 10.284)

            

    def testObs_beijing(self):
        config_file = self.createTestPath("Statoil/config/lab-beijing/labunits/config")
        with ErtTestContext("labscale-beijing", config_file) as test_context:
            ert = test_context.getErt()
            obs = ert.getObservations()


            v0 = obs["WCT0"]
            self.assertEqual( v0.activeStep() , 18 )
            node = v0.getNode( 18 )
            self.assertEqual( node.getValue() , 0.12345 )


            v1 = obs["WCT1"]
            self.assertEqual( v1.activeStep() , 18 )
            node = v1.getNode( 18 )
            self.assertEqual( node.getValue() , 0.12345 )
