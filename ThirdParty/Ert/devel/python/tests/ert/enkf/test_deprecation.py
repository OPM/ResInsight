#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_deprecation.py' is part of ERT - Ensemble based Reservoir Tool.
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
import warnings

from ert.test import ErtTestContext, ExtendedTestCase


class DeprecationTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("local/simple_config/minimum_config")
        self.obs_file    = self.createTestPath("local/simple_config/minimum_config")

        
    # Added in 1.10 development
    def test(self):
        with ErtTestContext("enkf_deprecation", self.config_file) as test_context:
            ert = test_context.getErt()

            ecl_config = ert.eclConfig()
            with warnings.catch_warnings():
                ecl_config.get_grid( )
                
    
